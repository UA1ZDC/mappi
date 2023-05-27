#include "router.h"

#include <qelapsedtimer.h>

#include <sql/dbi/dbiquery.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/global/appstatusthread.h>
#include <meteo/commons/rpc/controller.h>
#include <meteo/commons/msgparser/common.h>
#include <meteo/commons/msgdata/types.h>

namespace meteo {

static const QString kBeforeSubs("before");
static const QString kAfterSubs("after");

Router::Router( AppStatusThread* status, const QString& wmoid, const QString& hmsid )
  : mutex_( QMutex::Recursive ),
  status_(status)
{
  selfids_ << wmoid << hmsid;
  if ( -1 == lastid_ ) {
    error_log << QObject::tr("Не удалось получить последний идентификатор телеграммы");
  }
  status_->setTitle(kRecvDbStatus, tr("Подключение к БД"));
  status_->setTitle(kWriteToDb, tr("Записано в БД"));
  status_->setTitle(kRecvDuplicate, tr("Принято дубликатов"));

  status_->setParam(kWriteToDb, 0);
  status_->setTitle(kRecvDuplicate, 0);
  status_->setTitle(kDbSkippedSaveCount, QObject::tr("Количество не сохраненных в БД телеграмм"));
}

Router::~Router()
{
}
void Router::slotNewMessage( tlg::MessageNew tlg )
{
  routeMessage(&tlg);
}

void Router::slotThreadStarted()
{
  lastid_ = getLastTlgId();
  gridfs_ = new GridFs();
  ConnectProp param = Global::instance()->mongodbConfTelegrams();

  if ( false == gridfs_->connect( param ) ) {
    error_log << gridfs_->lastError();
  }
  gridfs_->use( param.name(), "fs" );
}

void Router::routeMessage( tlg::MessageNew* tlg )
{
  tlg->mutable_metainfo()->set_id(++lastid_);
  QStringList receivers = messageReceivers(tlg);
  setMetric(*tlg);
  QElapsedTimer tm;
  tm.start();
  bool saveres = saveToDb(tlg);
  //debug_log << QObject::tr("Время записи одной телеграммы = %1 мс").arg( tm.elapsed() );
  routeToSubs( receivers, tlg, &( subs_[kBeforeSubs] ) );
  if ( true == saveres ) {
    routeToSubs( receivers, tlg, &( subs_[kAfterSubs] ) );
  }
}

void Router::routeToSubs( const QStringList& recuids, tlg::MessageNew* tlg, QMap< rpc::Controller*, QList<ReqResp> >* subs )
{
  QMutexLocker lock(&mutex_);
  for ( auto it = subs->begin(), end = subs->end(); it != end; ++it ) {
    auto slist = it.value();
    auto ctrl = it.key();
    if ( false == recuids.contains( subuids_[ctrl] )  ) {
      continue;
    }
    QString uid = subuids_[ctrl];
    countSend_[uid] += 1;
    status_->setParam(paramIdSend_[uid], countSend_[uid]);
    for ( auto reqresp : slist ) {
      reqresp.second->CopyFrom(*tlg);
      ctrl->closure()->Run();
    }
  }
}

bool Router::checDb()
{
  if ( nullptr == db_ ) {
    db_ = meteo::global::dbTelegram();
    if ( nullptr == db_ ) {
      status_->setParam(kRecvDbStatus, tr("Нет"), app::OperationState_ERROR);
      status_->setParam(kDbSkippedSaveCount, ++skipCount_, app::OperationState_ERROR);
      error_log << QObject::tr("Не удалось подключиться к БД телеграмм");
      return false;
    }
  }
  if ( false == db_->connected() ) {
    delete db_;
    db_ = meteo::global::dbTelegram();
  }
  if ( nullptr == db_ ) {
    status_->setParam(kRecvDbStatus, tr("Нет"), app::OperationState_ERROR);
    status_->setParam(kDbSkippedSaveCount, ++skipCount_, app::OperationState_ERROR);
    error_log << QObject::tr("Не удалось подключиться к БД телеграмм");
    return false;
  }
  return true;
}

bool Router::saveToDb( tlg::MessageNew* msg )
{
  if ( false == checDb() ) {
    return false;
  }
  QString qinsert("insert_tlg");
  auto query = db_->queryptrByName( qinsert );
  if ( nullptr == query ) {
    status_->setParam(kDbSkippedSaveCount, ++skipCount_, app::OperationState_ERROR);
    error_log << QObject::tr("Не удалось создать запрос '%1'").arg(qinsert);
    return false;
  }
  query->arg( "id", lastid_ );
  query->arg( "t1", msg->header().t1().c_str() );
  query->arg( "t2", msg->header().t2().c_str() );
  query->arg( "a1", msg->header().a1().c_str() );
  query->arg( "a2", msg->header().a2().c_str() );
  query->arg( "ii", msg->header().ii() );

  QString cccc = QString::fromUtf8( msg->header().cccc().c_str() );
  QString dddd;
  if ( msg->addrmode() == tlg::kAddress && msg->format() == tlg::kWMO ) {
    dddd = cccc;

    cccc = tlg::creator(*msg);
    if ( cccc.isEmpty() ) {
      error_log << tr("Отсутствует информаци о составителе сообщения.");
    }
  }
  query->arg( "cccc", msg->header().cccc().c_str() );

  if ( msg->addrmode() == tlg::kAddress && msg->format() == tlg::kGMS ) {
    dddd = QString::fromUtf8( msg->header().gms().dddd().c_str()) ;
  }
  query->arg( "dddd", msg->header().gms().dddd().c_str() );

  query->arg( "bbb", QString::fromStdString(msg->header().bbb()).trimmed() );
  query->arg( "yygggg", msg->header().yygggg().c_str() );

  QDateTime cur = QDateTime::currentDateTimeUtc();
  query->arg( "dt", cur );

  QDateTime convDt = QDateTime::fromString(QString::fromStdString(msg->metainfo().converted_dt()), Qt::ISODate);
  convDt.setTimeSpec(Qt::UTC);
  if ( !convDt.isValid() ) {
    convDt = QDateTime(QDate(1,1,1),QTime(0,0,0));
  }
  query->arg( "msg_dt", convDt );

  MsgInfo msginfo(*msg);
  query->arg( "md5", QString( msginfo.md5().toBase64() ) );

  bool external = ( false == selfids_.contains(cccc) );
  if ( msg->metainfo().has_external() ) {
    external = msg->metainfo().external();
  }
  query->arg( "external", external );

  query->arg( "prior", tlg::priority(*msg) );

  query->arg( "bin", msg->isbinary() );
  query->arg( "format",tlg::Format_Name(msg->format()).c_str() );
  query->arg( "type", msg->msgtype().c_str() );

  QString convDtStr = msginfo.convertedDt().toString("yyyyMMdd");
  QString fileName = generateFileName( *msg, msginfo );
  GridFile file;
  fileName = generateFileName( *msg, msginfo );
//  debug_log << "ZAPIS =" << fileName;

  const QByteArray raw = tlg::raw2tlg(*msg);

  if ( false == gridfs_->put( fileName, raw, &file, false ) && GridFs::kErrorDuplicate != gridfs_->error() ) {
    status_->setParam(kDbSkippedSaveCount, ++skipCount_, app::OperationState_ERROR);
    error_log << gridfs_->lastError();
    return false;
  }


  query->arg( "size", int32_t(file.size()) );
  query->argOid( "file_id", file.id() );

  // globalfile_id
  // добавляем идентификатор файла в целом (если он по кускам приходит)
  //
  int globalfileid=0;
  if ( true == msg->metainfo().has_globalfile_id() ) {
    globalfileid = msg->metainfo().globalfile_id();
  }
  query->arg("globalfile_id", globalfileid );
  // определение типа
  bool addr_tlg = false;
  bool confirm_tlg = false;
  if ( ( QObject::tr("Б").toStdString() == msg->header().t1() || QObject::tr("B").toStdString() == msg->header().t1() )
      && ( QObject::tr("A").toStdString() == msg->header().t2() || QObject::tr("А").toStdString() == msg->header().t2() ) ) {
    addr_tlg = false;
    confirm_tlg = true;
    if(false == msg->metainfo().has_from()){
      msg->mutable_metainfo()->set_from("auto");
    }
  }
  else if (((QObject::tr("A").toStdString() == msg->header().t1() )
           && (QObject::tr("A").toStdString() == msg->header().t2()))
           || ((QObject::tr("А").toStdString() == msg->header().t1() )
               && (QObject::tr("А").toStdString() == msg->header().t2()))) { // русский и английский
    addr_tlg = true;
    confirm_tlg = false;
  }
  query->arg("addr_tlg",addr_tlg);
  query->arg("confirm_tlg",confirm_tlg);
  const char* src = msg->metainfo().has_from() ? msg->metainfo().from().c_str() : "<unk>";

  query->arg("src",src);
  query->arg("dst",msginfo.distributed());

  if ( true == msg->has_cor_number() && 0 < msg->cor_number() ) {
    query->arg("cor_number", msg->cor_number() );
  }
  if ( false == query->exec() ) {
    gridfs_->remove(fileName);
    error_log << QObject::tr("Ошибка выполнения запроса = %1")
      .arg( query->query() );
    status_->setParam(kDbSkippedSaveCount, ++skipCount_, app::OperationState_ERROR);
    return false;
  }
  bool noduplicate = false;
  const DbiEntry& res = query->result();
  if ( true == res.hasField("upserted") || ( true == res.hasField("dup") && 0 == res.valueInt32("dup") ) ) {
    status_->setParam(kWriteToDb, ++writeToDb_);
    noduplicate = true;
   // return true;
  }
  else {
    status_->setParam( kRecvDuplicate, ++dup_ );
    noduplicate = false;;
  }
  //return false;

  if ( (QObject::tr("Б").toStdString() == msg->header().t1()|| QObject::tr("B").toStdString() == msg->header().t1()  )
       && ( QObject::tr("A").toStdString() == msg->header().t2() || QObject::tr("А").toStdString() == msg->header().t2() ) ) {
    QString md5 = QString::fromStdString( msg->msg() );
    if ( md5.contains("CONFIRM MD5 ") ) {
      md5 = md5.remove("CONFIRM MD5 ").simplified();

      auto queryUpdateConfirm = db_->queryptrByName("update_msg_confirm"); //FIXME медленный запрос на партициях по дате! 
      if ( nullptr != queryUpdateConfirm ) {
        QByteArray arr( md5.toStdString().data(), md5.toStdString().size() );
        arr = arr.fromHex(arr);
        queryUpdateConfirm->argMd5("md5", arr );
        queryUpdateConfirm->arg("confirm",true);
        if ( false == queryUpdateConfirm->exec() ) {
          error_log << QObject::tr("Не удалось установить подтверждение для телеграммы md5:") << md5;
        }
      }
    }
  }
  status_->setParam(kRecvDbStatus, tr("Есть"));
  return noduplicate;
}

int64_t Router::getLastTlgId()
{
  if ( false == checDb() ) {
    return -1;
  }
  QString qgetlast("get_last_tlg_id");
  auto query = db_->queryptrByName(qgetlast);
  if ( nullptr == query ) {
    error_log << QObject::tr("Не удалось создать запрос '%1'").arg(qgetlast);
    return -1;
  }

  QString err;
  if ( false == query->execInit(&err) ){
    error_log << QObject::tr("Не удалось выполнить запрос = %1. Ошибка = %2")
      .arg( query->query() )
      .arg(err);
    return 1;
  }

  int64_t id = -1;
  if ( true == query->next()) {
    const DbiEntry& doc = query->entry();
    id = doc.valueInt64("_id");
  }
  else {
    debug_log << tr("Пустой ответ на запрос");
  }
  return id;
}

QString Router::generateFileName( const tlg::MessageNew& msg, const MsgInfo& info ) const
{
  QString bbb = QString::fromUtf8(msg.header().bbb().c_str()).trimmed();

  QString fileName = info.convertedDt().toString("yyyyMMdd") + QString("_%1%2%3%4%5_%6_%7_%8_%9.tlg")
      .arg(QString::fromUtf8(msg.header().t1().c_str()),
           QString::fromUtf8(msg.header().t2().c_str()),
           QString::fromUtf8(msg.header().a1().c_str()),
           QString::fromUtf8(msg.header().a2().c_str()),
           QString::number(msg.header().ii()).rightJustified(2,'0'),
           QString::fromUtf8(msg.header().cccc().c_str()),
           QString::fromUtf8(msg.header().yygggg().c_str()),
           (bbb.isEmpty() ? "" : ("_" + bbb)),
	   QString::fromUtf8( info.md5().toHex() )
           );
  return fileName;
}

void Router::subscribeClient( rpc::Controller* ctrl, const ReqResp& request )
{
  QString uid = QString::fromStdString( request.first->uid() );
  if ( uid.isEmpty() ) {
    uid = "subscriber";
    warning_log << tr("Не указан идентификатор клиента. Присвоен идентификатор: '%1'.").arg(uid);
  }

  QMutexLocker lock(&mutex_);

  debug_log << "uid =" << uid;
  QList<rpc::Controller*> ctrls = subuids_.keys(uid);
  if ( 0 != ctrls.size() ) {
    int i = 0;
    QString orig = uid;
    while ( true ) {
      if ( 0 == ctrls.size() ) {
        warning_log << tr("Клиент '%1' уже зарегистрирован. Новому клиенту присвоен идентификатор: '%2'.")
                       .arg(orig)
                       .arg(uid);
        orig = uid;
        break;
      }
      uid = QString("%1.%2").arg(orig).arg(++i,2,10,QChar('0'));
      ctrls = subuids_.keys(uid);
    }
  }
  if ( !paramIdSend_.contains(uid) ) {
    paramIdSend_[uid] = kCenterSend + paramIdSend_.size();
    QString title = tr("Передано в %1").arg(uid);
    status_->setTitle(paramIdSend_[uid], title);
  }
  status_->setParam(paramIdSend_[uid], 0);

  Rules rules( request.first );
//  rules = table_.merge(rules);
  setUID( &rules, uid );
  filters_.insert( uid, rules );
  if ( true == request.first->received_only() ) {
    subs_[kAfterSubs][ctrl].append(request);
  }
  else {
    subs_[kBeforeSubs][ctrl].append(request);
  }
  subuids_.insert( ctrl, uid );
}

void Router::setUID( Rules* rules, const QString& uid )
{
  for ( int i = 0, sz = rules->size(); i < sz; ++i ) {
    auto rule = rules->at(i);
    if ( rule.to() == "uid" ) {
      rule.setTo(uid);
    }
    if ( rule.from() == "uid" ) {
      rule.setFrom(uid);
    }
    rules->replace(i,rule);
  }
}

QStringList Router::messageReceivers( tlg::MessageNew* msg )
{
  QStringList recvlist;
  QStringList uids = subuids_.values();
  QString fromUid = QString::fromStdString( msg->metainfo().from() );
  TlgFilter filter(*msg);
  Rule rule;
  rule.setMsgFilter(filter);
  rule.setFrom(fromUid);
  for ( const QString& uid : uids ) {
    if ( !fromUid.isEmpty() && uid == fromUid ) {
      continue;
    }
    rule.setTo(uid);
    if ( filters_.contains(uid) ) {
      if ( false == filters_[uid].allow(rule) ) {
        continue;
      }
    }
    if ( table_.allow(rule) ) {
      msg->mutable_metainfo()->add_distibuted(uid.toStdString());
      recvlist.append(uid);
    }
  }
  return recvlist;
}

void Router::unsubscribeClient( rpc::Controller* ctrl )
{
  QMutexLocker lock(&mutex_);
  for ( auto it = subs_.begin(); it != subs_.end(); ++it ) {
    auto& s = it.value();
    if ( true == s.contains(ctrl) ) {
      s.remove(ctrl);
    }
  }
  subuids_.remove(ctrl);
}

void Router::setMetric( const tlg::MessageNew& msg )
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

}
