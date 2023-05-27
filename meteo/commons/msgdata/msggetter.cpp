#include "msggetter.h"

#include <qelapsedtimer.h>
#include <qfile.h>
#include <qjsonarray.h>
#include <qjsondocument.h>
#include <qjsonobject.h>
#include <qtimer.h>

#include <cross-commons/debug/tlog.h>

#include <sql/nosql/nosqlquery.h>
#include <sql/dbi/gridfs.h>
#include <sql/dbi/gridfile.h>

#include <meteo/commons/global/global.h>
#include <meteo/commons/proto/msgcenter.pb.h>
#include <meteo/commons/msgparser/common.h>
#include <meteo/commons/msgparser/tlgfuncs.h>
#include <meteo/commons/msgparser/tlgparser.h>
#include <meteo/commons/proto/msgparser.pb.h>

namespace meteo {

static const QString kGridPrefix = "fs";

//
// funcs
//

void MsgGetter::fillMessage(const DbiEntry& doc, tlg::MessageNew* mes)
{
  mes->mutable_metainfo()->set_id(doc.valueInt64("_id"));
  if ( doc.hasField("t1") ) { mes->mutable_header()->set_t1(doc.valueString("t1").toStdString()); }
  if ( doc.hasField("t2") ) { mes->mutable_header()->set_t2(doc.valueString("t2").toStdString()); }
  if ( doc.hasField("a1") ) { mes->mutable_header()->set_a1(doc.valueString("a1").toStdString()); }
  if ( doc.hasField("a2") ) { mes->mutable_header()->set_a2(doc.valueString("a2").toStdString()); }
  if ( doc.hasField("ii") ) { mes->mutable_header()->set_ii(doc.valueInt32("ii")); }
  if ( doc.hasField("cccc") ) { mes->mutable_header()->set_cccc(doc.valueString("cccc").toStdString()); }
  if ( doc.hasField("dddd") ) {
    QString dddd = doc.valueString("dddd");
    if ( !dddd.isEmpty() ) {
      mes->mutable_header()->mutable_gms()->set_dddd(dddd.toStdString());
    }
  }
  if ( doc.hasField("yygggg") ) { mes->mutable_header()->set_yygggg(doc.valueString("yygggg").toStdString()); }
  if ( doc.hasField("bbb") ) { mes->mutable_header()->set_bbb(doc.valueString("bbb").toStdString()); }
  if ( doc.hasField("external") ) { mes->mutable_metainfo()->set_external(doc.valueBool("external")); }
  if ( doc.hasField("file_id") ) { mes->mutable_metainfo()->set_file_id(doc.valueOid("file_id").toStdString()); }
  if ( doc.hasField("type") ) { mes->set_msgtype(doc.valueString("type").toStdString()); }
  if ( doc.hasField("msg_dt") ) {
    QDateTime dt = doc.valueDt("msg_dt");
    mes->mutable_metainfo()->set_converted_dt(dt.toString(Qt::ISODate).toStdString());
  }
}

QString makeQuery(const msgcenter::GetTelegramRequest& r)
{

  std::unique_ptr<Dbi> db( global::dbTelegram() );
  if ( nullptr == db ) {
    error_log << QObject::tr("Не удалось подключиться к БД!");
    return "";
  }
  auto query = db->queryptrByName("get_tlg");
  if ( nullptr == query ) {
    error_log << QObject::tr("Не найден запрос 'get_tlg'");
    return "";
  }


  QList<qint64> ids;
  for ( int i = 0, isz = r.ptkpp_id_size(); i < isz; ++i ) {
    ids << r.ptkpp_id(i);
  }
  if ( !ids.isEmpty() ) {
    query->arg("id",ids);
  }

  QStringList ts1;
  for ( int i = 0, isz = r.t1_size(); i < isz; ++i ) {
    ts1 << QString::fromStdString(r.t1(i));
  }
  if ( false == ts1.isEmpty() ) {
    query->arg("t1",ts1);
  }

  QStringList ts2;
  for ( int i = 0, isz = r.t2_size(); i < isz; ++i ) {
    ts2 << QString::fromStdString(r.t2(i));
  }
  if ( false == ts2.isEmpty() ) {
    query->arg("t2",ts2);
  }

  QStringList as1;
  for ( int i = 0, isz = r.a1_size(); i < isz; ++i ) {
    as1 << QString::fromStdString(r.a1(i));
  }
  if ( false == as1.isEmpty() ) {
    query->arg("a1",as1);
  }

  QStringList as2;
  for ( int i = 0, isz = r.a2_size(); i < isz; ++i ) {
    as2 << QString::fromStdString(r.a2(i));
  }
  if ( false == as2.isEmpty() ) {
    query->arg("a2",as2);
  }

  QStringList ccccs;
  for ( int i = 0, isz = r.cccc_size(); i < isz; ++i ) {
    ccccs << QString::fromStdString(r.cccc(i));
  }
  if ( false == ccccs.isEmpty() ) {
    query->arg("cccc",ccccs);
  }

  QStringList bbbs;
  for ( int i = 0, isz = r.bbb_size(); i < isz; ++i ) {
    bbbs << QString::fromStdString(r.bbb(i));
  }
  if ( false == bbbs.isEmpty() ) {
    query->arg("bbbb",bbbs);
  }

  QList<qint32> iis;
  for ( int i = 0, isz = r.ii_size(); i < isz; ++i ) {
    iis << r.ii(i);
  }
  if ( false == iis.isEmpty() ) {
    query->arg("ii",iis);
  }

  QStringList yyggggs;
  for ( int i = 0, isz = r.yygggg_size(); i < isz; ++i ) {
    yyggggs << QString::fromStdString(r.yygggg(i));
  }
  if ( false == yyggggs.isEmpty() ) {
    query->arg("yygggg",yyggggs);
  }

  QStringList msgtypes;
  for ( int i = 0, isz = r.msgtype_size(); i < isz; ++i ) {
    msgtypes << QString::fromStdString(r.msgtype(i));
  }
  if ( false == msgtypes.isEmpty() ) {
    query->arg("type",msgtypes);
  }

  if ( r.has_decoded() ) {
    query->arg("decoded",r.decoded());
  }

  if ( r.has_msg_beg_dt() && r.has_msg_end_dt() ) {
    QDateTime beg = QDateTime::fromString(QString::fromStdString(r.msg_beg_dt()), Qt::ISODate);
    QDateTime end = QDateTime::fromString(QString::fromStdString(r.msg_end_dt()), Qt::ISODate);
    query->arg("start_msgdt",beg);
    query->arg("end_msgdt",end);
  }

  if ( r.has_beg_dt() && r.has_end_dt() ) {
    QDateTime beg = QDateTime::fromString(QString::fromStdString(r.beg_dt()), Qt::ISODate);
    QDateTime end = QDateTime::fromString(QString::fromStdString(r.end_dt()), Qt::ISODate);
    query->arg("start_dt",beg);
    query->arg("end_dt",end);
  }

  if ( r.has_sort_by_id() ) {
    query->arg("sid",qBound(-1, r.sort_by_id(), 1));
  }

  if ( r.has_limit() ) {
    query->arg("limit",r.limit());
  }

  if ( r.has_skip() ) {
    query->arg("skip",r.skip());
  }

  return query->query();
}

MsgGetter::MsgGetter(QObject* parent)
  : QObject(parent)
{
  metric_.resize(60);
}

void MsgGetter::setStatusSender(AppStatusThread* status)
{
  status_ = status;
  status_->setTitle(kGetter, tr("[GetTelegram] среднее время получения 1 телеграммы"));
  status_->setParam(kGetter, QString("< нет данных >"));
}

void MsgGetter::slotInit()
{
  if ( nullptr != timer_ ) {
    timer_->stop();
    delete timer_;
  }

  params_ = global::mongodbConfTelegram();

  timer_ = new QTimer(this);
  timer_->setInterval(200);
  timer_->start();
  connect( timer_, &QTimer::timeout, this, &MsgGetter::slotRun );

  run_ = false;
}

void MsgGetter::slotRun()
{
  if ( nullptr == queue_ ) { return; }

  if ( 0 == queue_->size() ) {
    return;
  }

  if ( run_ ) { return; }
  run_ = true;

  QElapsedTimer timer;

  QByteArray ba;
  TlgParser parser(ba);

  std::unique_ptr<Dbi> db(meteo::global::dbTelegram());
  if ( nullptr == db.get() ) {
      error_log<<"Не удалось создать клиент БД";
      run_ = false;
      return;
    }
  if ( false == db->connected() && false == db->connect() ) {
     error_log << QObject::tr("Нет подключения к БД");
     run_ = false;
     return;
   }

  GridFs gridfs;

  if ( !gridfs.connect( params_ ) ) {
    error_log << gridfs.lastError();
    run_ = false;
    return;
  }
  gridfs.use( params_.name() ,kGridPrefix);

  while ( nullptr != db && queue_->size() > 0 )
  {
    timer.restart();

    GetTelegramCard card = queue_->takeFirst();

    const msgcenter::GetTelegramRequest* r = card.request;


//    if ( r->onlyamount() ) {
//
//      QStringList dts;
//      if ( r->has_msg_beg_dt() ) {
//        QDateTime beg = QDateTime::fromString(QString::fromStdString(r->msg_beg_dt()), Qt::ISODate);
//        dts << query::comparison::gte(util::date(beg));
//      }
//      if ( r->has_msg_end_dt() ) {
//        QDateTime end = QDateTime::fromString(QString::fromStdString(r->msg_end_dt()), Qt::ISODate);
//        dts << query::comparison::lt(util::date(end));
//      }
//
//      QStringList stages;
//      stages << pipeline::match(util::field("msg_dt", dts));
//      stages << pipeline::group("msg_dt", util::field("count", aggregation::accumulators::sum(1)));
//
//      auto query = Aggregate("msg", stages);
//
//      NosqlCursor* cur = db->aggregateRaw("telegramsdb", query);
//      if ( nullptr == cur ) {
//        card.response->set_error(tr("Ошибка при взаимодействии с БД.\n%1").arg(db->getLastError()).toStdString());
//      }
//
//      while ( nullptr != cur && cur->next() ) {
//        QJsonDocument doc = QJsonDocument::fromJson(cur->getDocument());
//        const QJsonObject o = doc.object();
//        QDateTime dt = QDateTime::fromMSecsSinceEpoch(o.value("_id").toObject().value("$date").toDouble(), Qt::UTC);
//        dt.setTimeSpec(Qt::UTC);
//
//        card.response->add_dt_list(dt.toString(Qt::ISODate).toStdString());
//        card.response->add_count_list(o.value("count").toInt());
//      }
//
//      delete cur;
//    }
//    else {
      QString q = makeQuery(*r);
      auto query = db->queryptr(q);

      QString error;
      if (nullptr == query || !query->execInit(&error) ) {
        //error = QObject::tr("Не удалось выполнить запрос = %1.").arg(q);
        card.response->set_error(error.toStdString());
        syncQueue_->append(card.done);
        emit syncQueueChanged();
        //error_log << error;
        break;
      }

      while ( true == query->next() ) {
        const DbiEntry& doc = query->entry();

        tlg::MessageNew* mes = card.response->add_msg();

        if ( !card.request->onlyheader() ) {
          QString qq = QString("{ \"_id\": { \"$oid\": \"%1\"} }")
                              .arg(doc.valueOid("file_id"));
          GridFile file = gridfs.findOne(qq);
          if ( !file.isValid() || !file.hasFile() ) {
            error_log << gridfs.lastError();
          }

          bool ok = false;
          QByteArray raw = file.readAll(&ok);
          if ( ok ) {
            parser.setData(raw);
            if ( !parser.parseNextMessage(mes) ) {
              error_log << QObject::tr("Не удалось загрузить телеграмму с идентификатором %1.").arg(doc.valueInt64("_id"));
            }
          }
        }

        fillMessage(doc, mes);
      }
//    }

    metric_.insert(TimeCountMetric(timer.elapsed(),card.response->msg_size()));

    syncQueue_->append(card.done);
    emit syncQueueChanged();
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
    status_->setParam(kGetter, QString("%1 мсек.").arg(avg, 4, 'f', 2));
  }

  run_ = false;
}


//
// MsgGetterMulti
//


MsgGetterMulti::MsgGetterMulti(QObject* parent)
  : QObject(parent)
{

}

MsgGetterMulti::~MsgGetterMulti()
{
  if ( -1 != timerId_ ) {
    killTimer(timerId_);
  }
}

void MsgGetterMulti::slotInit()
{
  params_ = global::mongodbConfTelegram();

  run_ = false;

  if ( -1 == timerId_ ) {
    timerId_ = startTimer(250);
  }
}

void MsgGetterMulti::slotRun()
{
  if ( 0 == jobs_.size() ) {
    return;
  }

  if ( run_ ) { return; }
  run_ = true;

  QByteArray ba;
  TlgParser parser(ba);

  wasdisconnected_ = false;

  QMutableHashIterator<int,TelegramJob> it(jobs_);
  while ( false == wasdisconnected_ && it.hasNext() ) {
    it.next();

    TelegramJob& item = it.value();

    if ( kRunQuery != item.state ) { continue; }

    if ( syncQueue_->contains(item.card.done) ) {
//      debug_log << tr("СИНХРОНИЗАЦИЯ задание №%1").arg(item.id);
      continue;
    }

    if ( item.remove ) {
      debug_log << tr("ЗАВЕРШЕНО задание №%1").arg(item.id);
      delete item.db;
      it.remove();
      continue;
    }

//    debug_log << tr("ВЫПОЛНЕНИЕ ЗАПРОСА задание №%1").arg(item.id);

    item.db = meteo::global::dbTelegram();
    if ( nullptr == item.db ) {
      error_log << tr("Нет подключения к БД");
      break;
    }
    if ( false == item.db->connected() && false == item.db->connect() ) {
       error_log << QObject::tr("Нет подключения к БД");
       break;
     }
    item.query = makeQuery(*item.card.request);

    if ( !item.db->execQuery(item.query) ) {
      QString error = QObject::tr("Не удалось выполнить запрос = %1.").arg( params_.name(), item.query );
      item.card.response->set_error(error.toStdString());
      syncQueue_->append(item.card.done);
      emit syncQueueChanged();
      error_log << error;
      item.remove = true;;
      continue;
    }

    /*TODO!if ( false == item.db->toCursor() ) {
      QString error = QObject::tr("Нет курсора в результате выполнения запроса %1").arg(item.query);
      item.card.response->set_error(error.toStdString());
      syncQueue_->append(item.card.done);
      emit syncQueueChanged();
      error_log << error;
      item.remove = true;;
      continue;
    }*/

    item.state = kSendResponse;
  }

  it.toFront();

//  GridFs gridfs;
//  if ( !gridfs.connect( params_ ) ) {
//
//    error_log << gridfs.lastError();
//    run_ = false;
//    return;
//  }
//  gridfs.use( params_.name(), kGridPrefix );
 /*TODO! while ( false == wasdisconnected_ && it.hasNext() ) {
    it.next();

    TelegramJob& item = it.value();

    if ( kSendResponse != item.state ) { continue; }

    if ( syncQueue_->contains(item.card.done) ) {
//      debug_log << tr("СИНХРОНИЗАЦИЯ задание №%1").arg(item.id);
      continue;
    }

    if ( item.remove ) {
      debug_log << tr("ЗАВЕРШЕНО задание №%1").arg(item.id);
      delete item.db;
      it.remove();
      continue;
    }

    debug_log << tr("ПОЛУЧЕНИЕ ДОКУМЕНТОВ задание №%1").arg(item.id);

    item.card.response->Clear();

    const int kSizeLimit = 512 * 1024;
    const int kCountLimit = 100;

    int n = 0;
    int sz = 0;
    bool hasNext = false;
    while ( sz < kSizeLimit && n < kCountLimit ) {
      hasNext = item.db->next();
      if ( !hasNext ) {
        break;
      }

      Document doc = item.db->document();

      tlg::MessageNew* mes = item.card.response->add_msg();

      if ( !item.card.request->onlyheader() ) {
        QString qq = QString("{ \"_id\": \"{ \"$oid\": \"%1\"}\" }")
        .arg(doc.valueOid("file_id"));

        GridFile file = gridfs.findOne(qq);
        if ( !file.isValid() || !file.hasFile() ) {
          error_log << gridfs.lastError();
          item.card.response->set_result(false);
          item.card.response->set_comment(gridfs.lastError().toStdString());
          item.card.response->set_finished(true);
          item.remove = true;
          break;
        }


        bool ok = false;
        QByteArray raw = file.readAll(&ok);
        if ( ok ) {
          parser.setData(raw);
          if ( !parser.parseNextMessage(mes) ) {
            error_log << QObject::tr("Не удалось загрузить телеграмму с идентификатором %1.").arg(doc.valueInt64("_id"));
          }
          sz += raw.size();
        }
      }

      MsgGetter::fillMessage(doc,mes);
      ++n;
    }

    if ( !hasNext ) {
      item.remove = true;
      item.card.response->set_finished(true);
    }

    if ( !item.card.response->has_result() ) {
      item.card.response->set_result(true);
    }

    syncQueue_->append(item.card.done);
    emit syncQueueChanged();
  }*/

  run_ = false;
}

void MsgGetterMulti::slotClientSubscribed(rpc::Controller* call)
{
  TelegramJob item;
  item.id = cardId_++;
  item.done = call->closure();
  item.callback = call;
  jobs_[item.id] = item;
}

void MsgGetterMulti::slotClientUnsubscribed(rpc::Controller* call)
{
  QMutableHashIterator<int,TelegramJob> it(jobs_);
  while ( it.hasNext() ) {
    it.next();
    if ( it.value().callback == call ) {
      debug_log << tr("УДАЛЕНО задание №%1").arg(it.value().id);
      delete it.value().db;
      it.remove();
      wasdisconnected_ = true;
    }
  }
}

void MsgGetterMulti::slotNewGetTelegramMulti(const GetTelegramCard& card)
{
  QMutableHashIterator<int,TelegramJob> it(jobs_);
  while ( it.hasNext() ) {
    it.next();
    TelegramJob& item = it.value();
    if ( item.done == card.done ) {
      item.card = card;
      item.state = kRunQuery;
      debug_log << tr("ДОБАВЛЕНО задание №%1").arg(item.id);
      break;
    }
  }
}

void MsgGetterMulti::timerEvent(QTimerEvent* event)
{
  Q_UNUSED( event );

  slotRun();
}

}
