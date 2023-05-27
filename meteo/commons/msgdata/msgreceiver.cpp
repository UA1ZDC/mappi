#include "msgreceiver.h"

#include <qdir.h>
#include <qfile.h>
#include <qtimer.h>
#include <qfileinfo.h>
#include <qdatetime.h>
#include <qelapsedtimer.h>

#include <cross-commons/debug/tlog.h>

#include <commons/textproto/pbtools.h>

#include <sql/nosql/document.h>
#include <sql/dbi/dbiquery.h>
#include <sql/dbi/gridfs.h>
#include <sql/dbi/gridfile.h>

#include <meteo/commons/global/global.h>
#include <meteo/commons/global/log.h>
#include <meteo/commons/msgparser/common.h>
#include <meteo/commons/msgparser/tlgfuncs.h>

#include <meteo/commons/msgdata/msgqueue.h>

#include "types.h"

namespace meteo {

static const QString kDbName = "telegramsdb";
static const QString kGridDbName = "telegramsdb";
static const QString kGridPrefix = "fs";

MsgReceiver::MsgReceiver(QObject *parent) :
  QObject(parent)
{
}

MsgReceiver::~MsgReceiver()
{
}

void MsgReceiver::setStatusSender(AppStatusThread* status)
{
  status_ = status;
  status_->setTitle(kRecvDbStatus, tr("Подключение к БД"));
//  status_->setTitle(kRecvFsStatus, tr("Запись данных в ФС"));
  status_->setTitle(kWriteToDb, tr("Записано в БД"));
  status_->setTitle(kRecvDuplicate, tr("Принято дубликатов"));

  status_->setParam(kWriteToDb, 0);
  status_->setTitle(kRecvDuplicate, 0);

  status_->setTitle(kDbSkippedSaveCount, QObject::tr("Количество не сохраненных в БД телеграмм"));
}

void MsgReceiver::slotInit()
{
  if ( nullptr == timer_ ) {
    timer_ = new QTimer(this);
    timer_->setInterval(500);
    connect( timer_, SIGNAL(timeout()), SLOT(slotRun()) );
  }
  run_ = false;
  timer_->start();

  std::unique_ptr<Dbi> db(meteo::global::dbTelegram());
  if ( nullptr == db.get() ) { return; }
  auto query = db->queryptrByName("get_last_tlg_id");
  if(nullptr == query) {return;}

  QTime errTimer; errTimer.start();;
  if ( !db->connect() ) {
    error_log << tr("Не удалось подключиться к БД");
    dbErrorTimeCount_ += qMax(errTimer.elapsed(), 200);
    QTimer::singleShot( 10000, this, SLOT(slotInit()) );
    return;
  }

  errTimer.restart();
  QString err;
  if ( !query->execInit(&err) ){
   // error_log << err;
    dbErrorTimeCount_ += qMax(errTimer.elapsed(), 200);
    QTimer::singleShot( 10000, this, SLOT(slotInit()) );
    return;
  }

  id_ = -1;
  if ( true == query->next()) {
    const DbiEntry& doc = query->entry();
    id_ = doc.valueInt64("_id");
  } else {
     debug_log<<tr("Пустой ответ на запрос");
  }

  status_->setParam(kRecvDbStatus, tr("Есть"));
  dbErrorTimeCount_ = 0;

}


bool MsgReceiver::getGridFsDb(GridFs *gridfs, Dbi *db){
  ConnectProp param = Global::instance()->mongodbConfTelegrams();



  //NoSql db(param.host(), param.port().toInt());


  if ( !db->connect() ) {
    status_->setParam(kRecvDbStatus, tr("Нет"), app::OperationState_ERROR);
    return false;
  }
  else {
    status_->setParam(kRecvDbStatus, tr("Есть"));
  }

  if ( !gridfs->connect( param ) ) {

    error_log << gridfs->lastError();
    status_->setParam(kRecvDbStatus, tr("Нет"), app::OperationState_ERROR);
    return false;
  }
  gridfs->use(kGridDbName,kGridPrefix);
  return true;
}


MsgReceiver::ret_value MsgReceiver::checkDupl(Dbi *db,  MsgQueueItem *item)
// 1. Проверка дубликатов
{
  const tlg::MessageNew& msg = item->msg();
  MsgInfo msginfo(msg);

 // std::unique_ptr<Dbi> db(meteo::global::dbTelegram());
  if ( nullptr == db ) { return error_res; }
  auto checkQ = db->queryptrByName("check_tlg_dup");
  if ( nullptr == checkQ ) {
    return error_res;
  }

  checkQ->arg("md5",QString(msginfo.md5().toBase64()));
  const char* src = msg.metainfo().has_from() ? msg.metainfo().from().c_str() : "<unk>";
  checkQ->arg("src",src);
  checkQ->arg("dst",msginfo.distributed());
  QTime errTimer; errTimer.start();
  if ( !checkQ->exec()) {
    status_->setParam(kRecvDbStatus, tr("Нет"), app::OperationState_ERROR);
    dbErrorTimeCount_ += qMax(errTimer.elapsed(), 200);

    return error_res;
  }

  const DbiEntry& doc = checkQ->result();
  if ( doc.hasField("value._id") && -1 != doc.valueInt64("value._id") ) {
    tlg::MessageNew m = item->msg();
    m.mutable_metainfo()->set_duplicate(true);
    item->setMsg(m);

    if ( false == item->nextStage() ) {
      int id = item->id();
      emit msgProcessed(id);
      emit msgProcessed(id, -1);
      emit msgProcessed(id, true, QString());
      emit msgProcessed(id, QString(), -1, item->msg().metainfo().duplicate());
    }

    status_->setParam(kRecvDuplicate,++dup_);
    return true_res;
  }
  return false_res;
}

void MsgReceiver::setMetric(const tlg::MessageNew& msg)
{
// 0. Метрика
count_[msg.metainfo().from()] += 1;
int cnt = count_[msg.metainfo().from()];
if ( 1 == cnt ) {
  QString title;
  if ( msg.metainfo().from().size() == 0 ) {
    paramId_["_create_"] = kRecvCounter + paramId_.size();
    title = tr("Сформировано телеграмм");
  }
  else {
    paramId_[msg.metainfo().from()] = kRecvCounter + paramId_.size();
    title = tr("Принято из канала %1").arg(QString::fromStdString(msg.metainfo().from()));
  }
  status_->setTitle(paramId_[msg.metainfo().from()], title);
}
status_->setParam(paramId_[msg.metainfo().from()], cnt);
}


void MsgReceiver::slotRun()
{
  QTime errTimer; errTimer.start();;
  if ( nullptr == queue_ || nullptr == status_ ) { return; }

  if ( 0 == queue_->size(kReceive) ) {
    return;
  }

  if ( run_ ) { return; }
  run_ = true;

  if ( dbErrorTimeCount_ >= maxDbErrorTime ){

    while ( this->queue_->size(kReceive) > 0 ) {
      auto item = this->queue_->head(kReceive);
      if ( false == item.nextStage() ) {
        emit msgProcessed(item.id());
        emit msgProcessed(item.id(), -1);
        emit msgProcessed(item.id(), false, QObject::tr("Ошибка при сохранении в БД. База данных недоступна."));
        emit msgProcessed(item.id(), QObject::tr("Ошибка при сохранении в БД. База данных недоступна"), -1, false);
        continue;
      }
      ++skipCount;
    }

    status_->setParam(kDbSkippedSaveCount, skipCount, app::OperationState_ERROR);
    dbErrorTimeCount_ = 0;
    run_ = false;
    return;
  }

  QElapsedTimer timer;
  timer.start();
  GridFs gridfs;

  std::unique_ptr<Dbi> db(meteo::global::dbTelegram());
  if ( nullptr == db.get() ) { return; }


  errTimer.restart();
  if ( false == getGridFsDb(&gridfs,db.get()) ) {
    dbErrorTimeCount_ += qMax(errTimer.elapsed(), 200);
    run_ = false;
    return;
  }


  if(false == run_)  { return; }

  int n = 0;
  while ( queue_->size(kReceive) > 0 )
  {
    MsgQueueItem item = queue_->head(kReceive);
    const tlg::MessageNew& msg = item.msg();
    MsgInfo msginfo(msg);
    // 0. Метрика
    setMetric(msg);

    // 1. Проверка дубликатов
    errTimer.restart();
    switch (checkDupl(db.get(),&item)) {
      case true_res:
        continue;
      case error_res:
        run_=false;
        dbErrorTimeCount_ += qMax(errTimer.elapsed(), 200);
        return;
      default:
        break;
     }

    // 2. Добавляем новый документ в коллекцию
    auto query = db->queryptrByName("insert_tlg");
    if(nullptr == query) {return;}

    ++id_;
    query->arg("id",id_);
    query->arg("t1",msg.header().t1().c_str());
    query->arg("t2",msg.header().t2().c_str());
    query->arg("a1",msg.header().a1().c_str());
    query->arg("a2",msg.header().a2().c_str());
    query->arg("ii",msg.header().ii());

    QString cccc = QString::fromUtf8(msg.header().cccc().c_str());
    QString dddd;
    if ( msg.addrmode() == tlg::kAddress && msg.format() == tlg::kWMO ) {
      dddd = cccc;

      cccc = tlg::creator(msg);
      if ( cccc.isEmpty() ) {
        error_log << tr("Отсутствует информаци о составители сообщения.");
      }
    }
    query->arg("cccc",msg.header().cccc().c_str());

    if ( msg.addrmode() == tlg::kAddress && msg.format() == tlg::kGMS ) {
      dddd = QString::fromUtf8(msg.header().gms().dddd().c_str());
    }
    query->arg("dddd",msg.header().gms().dddd().c_str());

    query->arg("bbb",QString::fromStdString(msg.header().bbb()).trimmed());
    query->arg("yygggg",msg.header().yygggg().c_str());

    QDateTime cur = QDateTime::currentDateTimeUtc();
    query->arg("dt",cur);

    QDateTime convDt = QDateTime::fromString(QString::fromStdString(msg.metainfo().converted_dt()), Qt::ISODate);
    convDt.setTimeSpec(Qt::UTC);
    if ( !convDt.isValid() ) {
      convDt = QDateTime(QDate(1,1,1),QTime(0,0,0));
    }
    query->arg("msg_dt",convDt);

    query->arg("md5",QString(msginfo.md5().toBase64()));

    bool external = !selfIds_.contains(cccc);
    if ( msg.metainfo().has_external() ) {
      external = msg.metainfo().external();
    }
    query->arg("external",external);

    query->arg("prior",tlg::priority(msg));

    query->arg("bin",msg.isbinary());
    query->arg("format",tlg::Format_Name(msg.format()).c_str());
    query->arg("type",msg.msgtype().c_str());

    query->arg("map_name",msg.metainfo().map_name().c_str());
    query->arg("job_name",msg.metainfo().job_name().c_str());

    QString convDtStr = msginfo.convertedDt().toString("yyyyMMdd");
    QString fileName = generateFileName(item.msg(), convDtStr);
    GridFile file;
    int idx = 0;
    for ( ;; ) {
      QString suff = ( idx > 0 ) ? "_" + QString::number(idx) : "";
      ++idx;
      fileName = generateFileName(item.msg(), convDtStr, suff);

      file = gridfs.findOne(QString("{ \"filename\": \"%1\" }").arg(fileName));
      errTimer.restart();
      if ( !file.isValid() ) {
        dbErrorTimeCount_ += qMax(errTimer.elapsed(), 200);
        error_log << gridfs.lastError();
        break;
      }
      if ( !file.hasFile() ) {
        break;
      }

    }

    if ( !file.isValid() ) {
      break;
    }

    const QByteArray raw = tlg::raw2tlg(msg);

    if ( !gridfs.put(fileName, raw, &file) ) {
      error_log << gridfs.lastError();
      emit msgProcessedError(item.id(), gridfs.lastError());
      if ( false == item.nextStage() ) {
        emit msgProcessed(item.id());
        emit msgProcessed(item.id(), -1);
        emit msgProcessed(item.id(), false, gridfs.lastError());
        emit msgProcessed(item.id(), gridfs.lastError(), -1, false);
      }
      continue;
    }

    query->arg("size",int32_t(file.size()));
    query->argOid("file_id",file.id());

    // globalfile_id
    // добавляем идентификатор файла в целом (если он по кускам приходит)
    //
    int globalfileid=0;
    if ( msg.metainfo().has_globalfile_id() ){
      globalfileid = msg.metainfo().globalfile_id();
    }
    query->arg("globalfile_id", globalfileid );
    // определение типа
    bool addr_tlg = false;
    bool confirm_tlg = false;
    if ((QObject::tr("Б").toStdString() == msg.header().t1() )
        && (QObject::tr("А").toStdString() == msg.header().t2()) ) {
      addr_tlg = false;
      confirm_tlg = true;
    }
    else if (((QObject::tr("A").toStdString() == msg.header().t1() )
             && (QObject::tr("A").toStdString() == msg.header().t2()))
             || ((QObject::tr("А").toStdString() == msg.header().t1() )
                 && (QObject::tr("А").toStdString() == msg.header().t2()))) { // русский и английский
      addr_tlg = true;
      confirm_tlg = false;
    }
    query->arg("addr_tlg",addr_tlg);
    query->arg("confirm_tlg",confirm_tlg);
    const char* src = msg.metainfo().has_from() ? msg.metainfo().from().c_str() : "<unk>";

    query->arg("src",src);
    query->arg("dst",msginfo.distributed());

    if ( true == msg.has_cor_number() && 0 < msg.cor_number() ) {
      query->arg("cor_number", msg.cor_number() );
    }

    errTimer.restart();
    if ( false == query->exec() ) {
      status_->setParam(kRecvDbStatus, tr("Нет"), app::OperationState_ERROR);
      error_log << meteo::msglog::kDbRequestFailed;
      dbErrorTimeCount_ += qMax(errTimer.elapsed(), 200);
      break;
    }

    const DbiEntry& result = query->result();
    // при нарушении уникальности _id повторяем инициализацию
    if ( result.hasField("writeErrors") ) {
      Array errors = result.valueArray("writeErrors");
      while ( errors.next() ) {
        Document e;
        errors.valueDocument(&e);
        if ( 11000 == e.valueInt32("code") ) {
          gridfs.remove(fileName);
          QTimer::singleShot( 0, this, SLOT(slotInit()) );
          warning_log << tr("Ошибка при взаимодействии с БД. %1.").arg(e.valueString("errmsg"));
        }
        else {
          error_log << tr("Ошибка при взаимодействии с БД. %1.").arg(e.valueString("errmsg"));
        }
      }
      run_ = false;
      return;
    }
    if ( (QObject::tr("Б").toStdString() == msg.header().t1() )
         && (QObject::tr("А").toStdString() == msg.header().t2() ) ) {
      QString md5 = QString::fromStdString( msg.msg() );
      if ( md5.contains("CONFIRM MD5 ") ) {
        md5 = md5.remove("CONFIRM MD5 ").simplified();

        auto queryUpdateConfirm = db->queryptrByName("update_msg_confirm");
        if(nullptr == queryUpdateConfirm) {return;}
          QByteArray arr(md5.toStdString().data(),md5.toStdString().size());
          arr = arr.fromHex(arr);
          queryUpdateConfirm->argMd5("md5",arr);
          queryUpdateConfirm->arg("confirm",QString::number(id_));
          errTimer.restart();
          if ( !queryUpdateConfirm->exec() ) {
            dbErrorTimeCount_ += qMax(errTimer.elapsed(), 200);
            error_log << QObject::tr("Не удалось установить подтверждение для телеграммы md5:") << md5;
            break;
          }
      }
    }

    status_->setParam(kWriteToDb, ++writeToDb_);

    tlg::MessageNew mes = item.msg();
    mes.mutable_metainfo()->set_id(id_);
    item.setMsg(mes);

    if ( false == item.nextStage() ) {
      emit msgProcessed(item.id());
      emit msgProcessed(item.id(), id_);
      emit msgProcessed(item.id(), true, QString());
      emit msgProcessed(item.id(), QString(), id_, item.msg().metainfo().duplicate());
    }

    ++n;
    debug_log_if( (n%1000) == 0 ) << tr("[ receive ] %1 за %2 мсек.").arg(1000, 4, 10, QChar(' ')).arg(timer.restart());
  }


  run_ = false;
}

bool MsgReceiver::saveToFile(const tlg::MessageNew& msg, const QString& path, const QString& fileName)
{
  QDir dir(path);
  if ( !dir.exists() ) {
    if ( !dir.mkpath(path) ) {
      error_log << tr("Не удалось создать директорию %1.").arg(path);
      return false;
    }
  }

  QFile file(path + "/" + fileName);
  if( !file.open(QIODevice::WriteOnly) ) {
    error_log << tr("Не удалось открыть файл %1.").arg(file.fileName())
              << file.errorString();
    return false;
  }

  QByteArray ba = tlg::raw2tlg(msg);

  quint64 r = file.write(ba);
  if ( r != static_cast<quint64>(ba.size()) ) {
    error_log << tr("Ошибка при записи сообщения в ФС");
  }

  return true;
}

QString MsgReceiver::generatePath(const tlg::MessageNew& msg) const
{
  QDateTime d = QDateTime::currentDateTime();
  if( msg.metainfo().has_converted_dt() ) {
    QDateTime metaDt = QDateTime::fromString(QString::fromUtf8(msg.metainfo().converted_dt().c_str()), Qt::ISODate);
    QDateTime convertedDt = convertedDateTime(metaDt, QString::fromUtf8(msg.header().yygggg().c_str()));
    if ( convertedDt.isValid() ) {
      d = convertedDt;
    }
  }

  return tlgPath_ + "/" + d.toString("yyyy-MM-dd") + "/" + QString::fromUtf8(msg.header().cccc().c_str());
}

QString MsgReceiver::generateFileName(const tlg::MessageNew& msg, const QString& pref, const QString& suff) const
{
  QString bbb = QString::fromUtf8(msg.header().bbb().c_str()).trimmed();

  QString fileName = pref + QString("_%1%2%3%4%5_%6_%7%8%9.tlg")
      .arg(QString::fromUtf8(msg.header().t1().c_str()),
           QString::fromUtf8(msg.header().t2().c_str()),
           QString::fromUtf8(msg.header().a1().c_str()),
           QString::fromUtf8(msg.header().a2().c_str()),
           QString::number(msg.header().ii()).rightJustified(2,'0'),
           QString::fromUtf8(msg.header().cccc().c_str()),
           QString::fromUtf8(msg.header().yygggg().c_str()),
           (bbb.isEmpty() ? "" : ("_" + bbb)),
           (suff.isEmpty() ? "" : suff)
           );
  return fileName;
}

} // meteo
