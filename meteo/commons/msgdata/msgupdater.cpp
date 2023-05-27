#include "msgupdater.h"

#include <qelapsedtimer.h>
#include <qtimer.h>

#include <cross-commons/debug/tlog.h>

#include <sql/dbi/dbiquery.h>

#include <meteo/commons/global/global.h>

namespace meteo {

static const QString& kDbTlg = "telegramsdb";


MsgUpdater::MsgUpdater(QObject* parent)
  : QObject(parent)
{
  metric_.resize(60);
}

void MsgUpdater::setStatusSender(AppStatusThread* status)
{
  status_ = status;
  status_->setTitle(kUpdater, tr("[SetDecode] среднее время обновления 1 документа"));
  status_->setParam(kUpdater, QString("< нет данных >"));
}

void MsgUpdater::slotInit()
{
  if ( nullptr != timer_ ) {
    timer_->stop();
    delete timer_;
  }

  timer_ = new QTimer(this);
  timer_->setInterval(1000);
  timer_->start();
  connect( timer_, &QTimer::timeout, this, &MsgUpdater::slotRun );

  run_ = false;
}

void MsgUpdater::slotRun()
{
  if ( nullptr == queue_ ) {
    return;
  }

  if ( true == run_ ) {
    return;
  }

  std::unique_ptr<Dbi> db(meteo::global::dbTelegram());
  if ( nullptr == db.get() ) { return; }

  run_ = true;

  QElapsedTimer timer;
  int reconnectCnt = 100;

  while ( queue_->size() > 0 )
  {
    timer.restart();

    msgcenter::DecodeRequest request = queue_->takeFirst();

    if ( request.id_size() == 0 ) {
      continue;
    }
    auto query = db->queryptrByName("update_tlg_set_decoded");
    if(nullptr == query) {
      run_ = false;//TODO нужно это???      
      return;
     }

    query->arg("id",request.id());//TODO
    query->arg("decoded",request.state());


    if ( false == db->connected() && false == db->connect()){
      error_log << meteo::msglog::kDbConnectFailed;
      queue_->append(request);
      sleep(1);
      --reconnectCnt;
      if (reconnectCnt == 0){
        error_log << QObject::tr("Исчерпан лимит попыток подключения к БД");
        run_ = false;//TODO нужно это???
        return;
      }
      continue;
    }

    if ( false == query->exec() ) {
      error_log << meteo::msglog::kDbRequestFailed;
      continue;
    }

    const DbiEntry& result = query->result();
    int ok = qRound(result.valueDouble("ok"));
    int n = qRound(result.valueDouble("n"));
    int nModified = qRound(result.valueDouble("nModified"));
    if ( 1 != ok ){
      debug_log << "ERROR!";
      //TODO не знаю что сдесь: error_log << meteo::msglog::kDbRequestFailedArg.arg(result.jsonString());
      continue;
    }
    if ( n != request.id_size() || nModified != request.id_size() ){
      warning_log << QObject::tr("Выбрано для обновления %1 телеграмм, обновлено %2 телеграмм, ожидалось %3")
                   .arg(n)
                   .arg(nModified)
                   .arg(request.id_size());
    }
    metric_.insert(TimeCountMetric(timer.elapsed(),request.id_size()));
  }

  // вычисление метрик
  int time = 0;
  int count = 0;
  for ( int i = 0, isz = metric_.size(); i < isz; ++i ) {
    time += metric_[i].time;
    count += metric_[i].count;
  }

  if ( 0 != count ) {
    double avg = double(time)/count;

    status_->setParam(kUpdater, QString("%1 мсек.").arg(avg, 4, 'f', 2));
  }

  run_ = false;
}

} // meteo
