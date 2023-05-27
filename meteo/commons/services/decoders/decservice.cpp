#include "decservice.h"
#include "tservicestat.h"

#include <meteo/commons/global/global.h>
#include <meteo/commons/rpc/rpc.h>
#include <meteo/commons/proto/state.pb.h>
#include <meteo/commons/tlgpool/telegramspool.h>


using namespace meteo;

DecodersService::DecodersService(const meteo::ServiceOpt& aopt):
  _updateTimer(0),
  _pool(0),
  _res(0),
  _option(aopt)
{
  _pool = new meteo::SafeTelegramPool<meteo::tlg::MessageNew>();
}

DecodersService::~DecodersService()
{
  if ( nullptr != _chSubscr ) {
    delete _chSubscr;
    _chSubscr = nullptr;
  }

  if (0 != _res) {
    delete _res;
    _res = 0;
  }

  if (_updateTimer) {
    delete _updateTimer;
    _updateTimer = 0;
  }

  if (_pool) {
    delete _pool;
    _pool = 0;
  }
}

bool DecodersService::init()
{
  _dt = QDateTime::currentDateTimeUtc();

  bool ok = true;
  _sprinfAddr = meteo::global::serviceAddress( meteo::settings::proto::kSprinf, &ok );
  if ( false == ok ) {
    error_log << QObject::tr("Адрес сервиса доступа к справочной информации не найден");
  }

  if ( 0 == _updateTimer ) {
    if ( 0 != (_updateTimer = new QTimer) ) {
      connect( _updateTimer, SIGNAL(timeout()), SLOT(controlChannels()) );
    }
  }
  _updateTimer->stop();
  _updateTimer->setInterval( 1000 );
  _updateTimer->start();

  QTimer::singleShot(100, this, SLOT(sendDecoded()));

  return ok;
}

void DecodersService::receivedMsg(meteo::tlg::MessageNew* tlg)
{
  if (0 != tlg && tlg->msgtype() == _sbscrType) {
    _pool->addTelegram(*tlg);
  } else if (0 != tlg) {
    debug_log << "invalid type" << tlg->msgtype() << _sbscrType;
  }
  //  debug_log << "recv=" << _pool->size();
}

//! отправка id раскодированной телеграммы
void DecodersService::sendDecoded(long long id)
{
  _decodedId.append(id);
}

void DecodersService::sendDecoded()
{
  QMutexLocker locker(&_mutex);

  if (_decodedId.isEmpty() || !checkChannel()) {
    QTimer::singleShot(100, this, SLOT(sendDecoded()));
    return;
  }

  QList<long long> decoded;
  decoded.swap(_decodedId);

  int time = 1000;

  meteo::msgcenter::DecodeRequest drec;
  for (int idx = 0; idx < decoded.size(); idx++) {
    drec.add_id(decoded.at(idx));
  }
  drec.set_state(true);

  _chSubscr->remoteCall(&meteo::msgcenter::MsgCenterService::SetDecode, drec,
                        this, &DecodersService::messageHandler);

  QTimer::singleShot(time, this, SLOT(sendDecoded()));
}


const QString& DecodersService::sprinfAddress()
{
  _sprinfAddr = meteo::global::serviceAddress( meteo::settings::proto::kSprinf );
  return _sprinfAddr;
}

//--------- статус

//! Обновление и отправка состояния
void DecodersServiceThread::updateStatus()
{
  meteo::app::OperationStatus status;
  status.set_state(meteo::app::OperationState_NORM);

  //  _statusMng->setAppId(&status);

  // meteo::app::OperationParam* param = status.add_param();
  // param->set_title( QObject::tr("Время запуска").toUtf8().constData() );
  // param->set_value(_dt.toString().toUtf8().constData());
  // param->set_state(meteo::app::OperationState_NORM);

  meteo::app::OperationParam* param = status.add_param();
  param->set_title( QObject::tr("Подключение к ЦКС").toUtf8().constData() );
  if (nullptr != _serv && _serv->checkChannel()) {
    param->set_value(QObject::tr("Есть").toStdString());
    param->set_state(meteo::app::OperationState_NORM);
  } else {
    param->set_value(QObject::tr("Нет").toStdString());
    param->set_state(meteo::app::OperationState_ERROR);
    status.set_state(meteo::app::OperationState_ERROR);
  }
  param->set_dt(QDateTime::currentDateTime().toString("dd.MM.yy hh:mm").toStdString());

  fillStatus(&status);
  emit sendStatus(status);

  //_statusMng->send(status);
}

void DecodersServiceThread::fillStatus(meteo::app::OperationStatus* status) const
{
  if (!status) return;
  meteo::app::OperationParam* param = status->add_param();
  param->set_title( tr("Тут некие осмысленные слова").toUtf8().constData() );
  param->set_value( tr("Тут некие осмысленные значения").toUtf8().constData());
  param->set_state(meteo::app::OperationState_ERROR);
}

//! Установка параметров для слежения за состоянием сервиса
/*!
  \param manager Название менеджера, которому отправляется состояние
  \return true - в случае успешной настройки
*/
bool DecodersServiceThread::setStatusControl(TServiceStatus* statMng)
{
  if (0 == statMng) {
    return false;
  }

  qRegisterMetaType<meteo::app::OperationStatus>("meteo::app::OperationStatus");

  _statusMng = statMng;
  _statusMng->moveToThread(&_pthr);

  connect(&_pthr, SIGNAL(finished()), _statusMng, SLOT(deleteLater()));
  connect(&_pthr, SIGNAL(started()), _statusMng, SLOT(init()));
  connect(this, SIGNAL(sendStatus(const meteo::app::OperationStatus&)), _statusMng, SLOT(sendStatus(const meteo::app::OperationStatus&)));

  return true;
}

//--------- контроль/подключение канала подписки/отправки результата

bool DecodersService::createChannel()
{
  if ( nullptr != _chSubscr ) {
    delete _chSubscr;
    _chSubscr = nullptr;
  }

  _chSubscr = meteo::global::serviceChannel(meteo::settings::proto::kMsgCenter);

  if ( nullptr == _chSubscr ) {
    error_log << QObject::tr("Сервис рассылки телеграмм не обнаружен");
    return false;
  }

  return true;
}

bool DecodersService::checkChannel() const
{
  return ( nullptr != _chSubscr && _chSubscr->isConnected() );
}

bool DecodersService::createSubscribe()
{
  if (! createChannel()) {
    return false;
  }

  bool result = false;
  meteo::msgcenter::DistributeMsgRequest req;
  req.set_drop_duplicate(true);
  req.set_received_only(true);
  msgcenter::Rule* rule = nullptr;
  QString uid;

  switch (_option.stype) {
  case meteo::settings::proto::kAlphanum:
    uid = "alphanum";
    req.set_uid(uid.toStdString());
    rule = req.add_rule();
    rule->set_type(msgcenter::RuleType::kAllow);
    rule->mutable_tlgfilter()->set_msgtype(uid.toStdString());
    rule->set_to("uid");
    rule->set_priority(0);
    rule = req.add_rule();
    rule->set_type(msgcenter::RuleType::kDeny);
    rule->set_from("all");
    rule->set_to("uid");
    rule->set_priority(1);
    _sbscrType = uid.toStdString();
    break;
  case meteo::settings::proto::kGphAlphanum:
    uid = "gphalphanum";
    req.set_uid(uid.toStdString());
    rule = req.add_rule();
    rule->set_type(msgcenter::RuleType::kAllow);
    rule->mutable_tlgfilter()->set_t1("z");
    rule->mutable_tlgfilter()->set_t2("i");
    rule->set_to("uid");
    rule->set_priority(0);
    rule = req.add_rule();
    rule->set_type(msgcenter::RuleType::kDeny);
    rule->set_from("all");
    rule->set_to("uid");
    rule->set_priority(1);
    _sbscrType = "alphanum";
    break;
  case meteo::settings::proto::kGrib:
    uid = "grib";
    req.set_uid(uid.toStdString());
    rule = req.add_rule();
    rule->set_type(msgcenter::RuleType::kAllow);
    rule->mutable_tlgfilter()->set_msgtype(uid.toStdString());
    rule->set_to("uid");
    rule->set_priority(0);
    rule = req.add_rule();
    rule->set_type(msgcenter::RuleType::kDeny);
    rule->set_from("all");
    rule->set_to("uid");
    rule->set_priority(1);
    _sbscrType = uid.toStdString();
    break;
  case meteo::settings::proto::kBufr:
    uid = "bufr";
    req.set_uid(uid.toStdString());
    rule = req.add_rule();
    rule->set_type(msgcenter::RuleType::kAllow);
    rule->mutable_tlgfilter()->set_msgtype(uid.toStdString());
    rule->set_to("uid");
    rule->set_priority(0);
    rule = req.add_rule();
    rule->set_type(msgcenter::RuleType::kDeny);
    rule->set_from("all");
    rule->set_to("uid");
    rule->set_priority(1);
    _sbscrType = uid.toStdString();
    break;
  case meteo::settings::proto::kCliwareAlphanum:
    uid = "clialphanum";
    req.set_uid(uid.toStdString());
    rule = req.add_rule();
    rule->set_type(msgcenter::RuleType::kAllow);
    rule->mutable_tlgfilter()->set_msgtype(uid.toStdString());
    rule->set_to("uid");
    rule->set_priority(0);
    rule = req.add_rule();
    rule->set_type(msgcenter::RuleType::kDeny);
    rule->set_from("all");
    rule->set_to("uid");
    rule->set_priority(1);
    _sbscrType = uid.toStdString();
    break;
  default: {
    error_log << tr("Не задан тип сервиса (подписки)");
  }
  }

  result = _chSubscr->subscribe(&meteo::msgcenter::MsgCenterService::DistributeMsg,
                                req,
                                this,
                                &DecodersService::receivedMsg);

  //var(result);
  info_log << QObject::tr("Подписка к %1 (тип подписки %2)").
    arg(global::serviceTitle(meteo::settings::proto::kMsgCenter)).
    arg(_sbscrType.c_str());

  return result;
}



//! проверка каналов и их восстановление при необходимости [private slot]
void DecodersService::controlChannels()
{
  if (!checkChannel()) {
    createSubscribe();
  }

  //  debug_log << "_pool =" << _pool->size() << " _decoded =" << _decodedId.size();
}

DecodersServiceThread::DecodersServiceThread(const meteo::ServiceOpt& opt):
  _serv(0),
  _pool(0),
  _statusMng(0)
{
  _serv = new DecodersService(opt);
  _pool = _serv->pool();

  _serv->moveToThread(&_pthr);

  connect(&_pthr, SIGNAL(finished()), _serv, SLOT(deleteLater()));
  connect(&_pthr, SIGNAL(started()), _serv, SLOT(init()));
  connect(this, SIGNAL(sendDecoded(long long)), _serv, SLOT(sendDecoded(long long)));

}

DecodersServiceThread::~DecodersServiceThread()
{
  _pthr.quit();
  _pthr.wait();
}

void DecodersServiceThread::start()
{
  info_log << QObject::tr("Запуск обработки телеграмм");
  _pthr.start();
  processMsg();
}

void DecodersServiceThread::processMsg()
{
  if (_pool == 0) return;

  while (!_pool->isEmpty()) {
    meteo::tlg::MessageNew tlg = _pool->takeFirst();
    if ( tlg.metainfo().id() == 0 ) {
     // debug_log << "idPtkpp = 0" << tlg.header().Utf8DebugString() << tlg.metainfo().Utf8DebugString();
    }
    ulong id = processMsg(tlg);
    //if ( id == 0 ) {
    //  var(tlg.Utf8DebugString());
    //}
    //    debug_log << "proc=" << _pool->size();
    //_pool->removeFirst();
    if (id > 0) {
      emit sendDecoded(id);
    }
    if (0 != _statusMng && _statusMng->isNeedUpdate()) {
      updateStatus();
    }
  }

  if (0 != _statusMng && _statusMng->isNeedUpdate()) {
    updateStatus();
  }

  QTimer::singleShot(1000, this, SLOT(processMsg()));
}
