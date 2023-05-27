#include "sessionmanager.h"

#include <qthread.h>
#include <qelapsedtimer.h>

#include <commons/textproto/tprototext.h>

#include <meteo/commons/global/common.h>

#include <mappi/global/global.h>
#include <mappi/device/antenna/antfollow.h>
#include <mappi/device/receiver/ctrl.h>
#include <mappi/device/receiver/recvstream.h>
#include <mappi/pretreatment/handler/handler.h>
#include <mappi/schedule/schedulehelper.hpp>

namespace mappi {
namespace receive {

static const QString kReception = MnCommon::etcPath("mappi") + "reception.conf";

const static int kTimeoutConnect  = 30000; //!< Таймаут соединения с сервисом

SessionManager::SessionManager()
  : ant_(new AntFollow())
  , rctrl_(nullptr)
  , receiver_(new RecvStream())
  , handler_(new po::Handler())
  , chReceiver_(nullptr)
    //  , ctrlReceiver_(nullptr)
    //  , replySchedule_(new conf::ScheduleReply())
  , timer_(new QTimer(this))
  , state_(kWaiting)
{
 
}
SessionManager::~SessionManager()
{
  ant_->finish();
  receiver_->finish();
  if (nullptr != rctrl_) {
    rctrl_->stop();
  }

  delete handler_;
  handler_ = nullptr;
}

bool SessionManager::init()
{
  trc;
  bool ok = true;
  QFile file(kReception);
  if ( !file.open(QIODevice::ReadOnly) ) {
    error_log << QObject::tr("Ошибка при загрузке настроек '%1'").arg(kReception);
    ok = false;
  }

  if(ok) {
    QString text = QString::fromUtf8(file.readAll());
    file.close();

    if(!TProtoText::fillProto(text, &conf_)) {
      error_log << QObject::tr("Ошибка в структуре файла настроек '%1'").arg(kReception);
      ok = false;
    }
  }

  meteo::GeoPoint site(conf_.site().point().lat_radian(),
		       conf_.site().point().lon_radian(),
		       conf_.site().point().height_meters());
    
  ant_->slotInit(site, conf_.antenna());

  receiver_->init(QString::fromStdString(conf_.site().name()), site, QString::fromStdString(conf_.satdata()));
  receiver_->setFileSource(QString::fromStdString(conf_.receiverfile()));


 //Поток управления антенной
  QThread* antThread = new QThread();
  QObject::connect(ant_,      &AntFollow::positionChanged, this,      &SessionManager::slotDeviceSubscr);
  QObject::connect(ant_,      &AntFollow::finished,        ant_,      &AntFollow::deleteLater);
  QObject::connect(ant_,      &AntFollow::destroyed,       antThread, &QThread::quit);
  QObject::connect(ant_,      &AntFollow::checked,         this,      &SessionManager::slotDeviceUncheck);
  QObject::connect(antThread, &QThread::finished,          antThread, &QThread::deleteLater);
  ant_->moveToThread(antThread);
  antThread->start();

  //Поток управления приёмом
  QThread* receiverThread = new QThread();
  QObject::connect(receiver_,      &RecvStream::receivedData, this,           &SessionManager::slotDataSubscr);
  QObject::connect(receiver_,      &RecvStream::finished,     receiver_,      &RecvStream::deleteLater);
  QObject::connect(receiver_,      &RecvStream::destroyed,    receiverThread, &QThread::quit);
  QObject::connect(receiverThread, &QThread::finished,        receiverThread, &QThread::deleteLater);
  receiver_->moveToThread(receiverThread);
  receiverThread->start();

  QObject::connect(timer_, &QTimer::timeout, this, &SessionManager::slotTimeout);

  // ant_->check();
  ant_->slotCheck();



  return ok;
}

void SessionManager::slotDeviceUncheck(bool check)
{
  if(check) {
    slotInitChannel();
  }
  else {
    error_log << QObject::tr("Антенна не доступна");
    timer_->stop();
    stopReceiver();
    thread()->quit();
  }
}

bool SessionManager::checkSchedule() {
  trc;
  schedule::Schedule sched = schedule::ScheduleHelper::getSchedule();
  return checkSession(sched);
}

bool SessionManager::checkSession(const schedule::Schedule& sched)
{ 
  trc;
  if (true == sched.isEmpty()) {
    state_ = kNoSession;
    return false;
  }

  // Если текущий сеанс выполняется ничего не делаем
  if(near_.currentStage() == schedule::Session::ALREADY_TAKEN && near_.adopted()) {
    return true;
  }

  near_ = getNextNear(sched);
  
  debug_log << "current" << currentDt() << timeOffset_;

  if (near_.isDefaults()) {
    error_log << QObject::tr("Нет сеансов");
    state_ = kNoSession;
    return false;
  }

  timer_->stop();

  sendNearSession();
  
  //Запрос tle у сервиса расписаний
  stle_ = schedule::ScheduleHelper::getSTle(near_.getNameSat(), near_.getAos());
  if(stle_.satName.isEmpty()) { //Если сервис не прислал tle пытаемся прочитать из файла
    SatelliteBase::getSTLE(near_.getNameSat(),  meteo::global::findWeatherFile(QString::fromStdString(conf_.tledir()), currentDt()), &stle_);
  }
  ant_->setSTLE(stle_);
  ant_->setStartPosition(near_.getAos(), near_.getLos(), true); //TODO проверка установки антенны. но тестировать без антенны, тогда не получится


//  virtual void TleSatellite(google::protobuf::RpcController* ctrl, const conf::TleParamRequest* req,
//    conf::TleParamResponse* resp, google::protobuf::Closure* done);


  var(near_.toLine());
  int timeout = currentDt().secsTo(near_.getAos()) * 1000;
  var(timeout / 60000.0);

  if (timeout <= 0) {
    state_ = kRecv;
    debug_log << "start timer" << currentDt().secsTo(near_.getLos())*1000;
    timer_->start(currentDt().secsTo(near_.getLos())*1000);
    process();
  } else {
    state_ = kWaiting;
    debug_log << "wait  timer" << timeout;
    timer_->start(timeout);
  }

  return true;
}

schedule::Session SessionManager::getNextNear(const schedule::Schedule& sched)
{
  schedule::Session near = sched.nearSession(true);
  if(currentDt().secsTo(near.getLos()) < 60) {
    near = sched.nearSession(false);
  }
  
  return near;
}


//! проверка времени начала/окончания приёма
void SessionManager::slotTimeout()
{
  switch (state_)
  {
  case kNoSession:
    checkSchedule();
    break;
  case kWaiting:
    state_ = kRecv;
    process();
    break;
  case kRecv:
    state_ = kWaiting;
    stopProcess();
    break;
  default:
    error_log << QObject::tr("Неизвестное состояние менеджера приёма");
    break;
  }
}


//!* Функции работы с сервисом расписаний */
void SessionManager::deleteChannel()
{
  if(nullptr != chReceiver_) {
    QObject::disconnect(chReceiver_, 0, this, 0);
    chReceiver_->deleteLater();
    chReceiver_ = nullptr;
  }

  // if(nullptr != ctrlReceiver_) {
  //   ctrlReceiver_->deleteLater();
  //   ctrlReceiver_ = nullptr;
  // }
}
void SessionManager::slotInitChannel()
{
  deleteChannel();
  slotConnectToService();
}
void SessionManager::slotConnectToService()
{
  if(nullptr == chReceiver_)
  {
    chReceiver_ = meteo::global::serviceChannel(meteo::settings::proto::kSchedule);
    if(nullptr == chReceiver_) {
      QTimer::singleShot(kTimeoutConnect, this, &SessionManager::slotConnectToService);
      return;
    }
    QObject::connect(chReceiver_, &meteo::rpc::Channel::disconnected, this, &SessionManager::slotDisconnectedFromService);
  }

  if(false == chReceiver_->isConnected())
  {
    deleteChannel();
    QTimer::singleShot(kTimeoutConnect, this, &SessionManager::slotConnectToService);
    return;
  }
  
  checkSchedule();
  subscribe();
}

void SessionManager::slotDisconnectedFromService()
{
  deleteChannel();
  QTimer::singleShot(kTimeoutConnect, this, &SessionManager::slotConnectToService);
}

bool SessionManager::subscribe()
{
  if(nullptr == chReceiver_) return false;
  if(false == chReceiver_->isConnected()) return false;

  //  ctrlReceiver_ = new rpc::TController(chReceiver_);

  mappi::conf::ScheduleSubscribeRequest request;
  request.set_subscr(true);

  bool ok = chReceiver_->subscribe(&mappi::conf::ScheduleService::Subscribe, request,
                                   this, &SessionManager::callbackSchedule);
  if(false == ok) {
    error_log << QObject::tr("Не удалось подписаться на получение информации о состоянии приемника");
    QTimer::singleShot(kTimeoutConnect, this, &SessionManager::slotInitChannel);
  }
  return ok;
}

void SessionManager::callbackSchedule(conf::ScheduleReply* reply)
{
  schedule::Schedule sched;
  for(const auto& it : reply->session()) {
    sched.appendSession(schedule::Session(it));
  }
  checkSession(sched);
}


void SessionManager::process()
{
  timer_->stop();

  QDir dir;
  dir.remove(QString::fromStdString(conf_.receiverfile()));

  Satellite satellite;
  satellite.readTLE(stle_);

  //Запуск управления антеной
  ant_->start(60 * 1000);


  //запуск ресивера
  startReceiver();

  if (nullptr != handler_) {
    handler_->setupRT(near_.getNameSat(), conf_.recvmode(), satellite.getTLEParams());
    tmpdata_.clear();
    tmprows_ = 0;
  }

  //чтение/сохранение данных с ресивера
  receiver_->start(near_.getNameSat(), conf_.recvmode(), near_.getDirection(), satellite.getTLEParams());

  
  int timeout = currentDt().secsTo(near_.getLos()) * 1000;
  if (timeout < 0) {
    timeout = 0;
  }
  debug_log << "wait  timer" << timeout;
  timer_->start(timeout);
}

void SessionManager::stopProcess()
{
  timer_->stop();

  QString savedFile;
  uint64_t dbId = 0;
  receiver_->stop(conf_.site(), &savedFile, &dbId);

  //остановка ресивера
  stopReceiver();
  
  //остановка антенны
  ant_->stop();
 
  //обработка данных
  slotFileSubscr(savedFile, dbId);

  if(false == checkSchedule()) {
    return;
  }
  //TODO если ждать следующего сеанса - отключить питание
}

  //TODO для виртуального, копи-пасте предыдущего
void SessionManager::stopProcessWithDt()
{
  timer_->stop();

  QString savedFile;
  uint64_t dbId = 0;
  if (!near_.isDefaults()) {
    receiver_->stop(conf_.site(), near_.getAos(), near_.getLos(), &savedFile, &dbId);
  } else {
    receiver_->stop(conf_.site(), &savedFile, &dbId);
  }

  //остановка ресивера
  stopReceiver();
  
  //остановка антенны
  ant_->stop();
 
  //обработка данных
  slotFileSubscr(savedFile, dbId);

  if(false == checkSchedule()) {
    return;
  }
  //TODO если ждать следующего сеанса - отключить питание
}

bool SessionManager::startReceiver()
{
  debug_log << "startReceiver!";
  for(const auto& sat : conf_.sat())
  {
    if(sat.has_name() && QString::fromStdString(sat.name()) == near_.getNameSat())
    {
      for(const auto& recv : sat.recv())
      {
        if(recv.has_mode() && conf_.has_recvmode() && recv.mode() == conf_.recvmode() && conf_.has_recvapp())
        {
          stopReceiver();

          debug_log << "start: " << sat.name() << " freq: " << recv.freq() << " gain: " << recv.gain() << " rate: " << recv.rate();
          debug_log << "app = " << MnCommon::binPath("mappi") + QString::fromStdString(conf_.recvapp());

          rctrl_ = new mappi::receive::Ctrl(MnCommon::binPath("mappi") + QString::fromStdString(conf_.recvapp()));

          Coords::GeoCoord site(conf_.site().point().lat_radian(),
				conf_.site().point().lon_radian(),
				conf_.site().point().height_meters());


          if(conf_.adjust_freq()) {
            rctrl_->setAdjustFreq(site);
          }
          rctrl_->init(stle_, recv.freq(), recv.gain(), recv.rate());

          QThread* rctrlThread = new QThread();
          QObject::connect(rctrl_,      &Ctrl::finished,    rctrl_,      &Ctrl::deleteLater);
          QObject::connect(rctrl_,      &Ctrl::destroyed,   [&]() { rctrl_ = nullptr; });
          QObject::connect(rctrl_,      &Ctrl::destroyed,   rctrlThread, &QThread::quit);
          QObject::connect(rctrlThread, &QThread::started,  rctrl_,      &Ctrl::start);
          QObject::connect(rctrlThread, &QThread::finished, rctrlThread, &QThread::deleteLater);
          rctrl_->moveToThread(rctrlThread);
          rctrlThread->start();

          debug_log << "success startReceiver!";
          return true;
        }
      }
    }
  }
  debug_log << "failed startReceiver!";
  return false;
}
bool SessionManager::stopReceiver()
{
  if (nullptr != rctrl_) {
    rctrl_->stop();
  }
  return true;
}


//! Настройка для подписки на полученный поток
void SessionManager::GetDataMulti(google::protobuf::RpcController* controller, const conf::DataRequest* request, 
				  conf::DataReply* response, google::protobuf::Closure* done)
{
  DataMultiCard card;
  card.request  = request;
  card.response = response;
  card.done = done;
  card.controller = controller;

  dataMultiCards_[subs_.key(done)] = card;

  debug_log << QObject::tr("Оформлена подписка на получение спутниковых данных, всего подписчиков %1").arg(dataMultiCards_.size());
}

void SessionManager::GetDeviceStateMulti(google::protobuf::RpcController *controller,
					 const conf::DataRequest *request,
					 conf::DeviceStateReply *response,
					 google::protobuf::Closure *done)
{
  DeviceMultiCard card;
  card.request  = request;
  card.response = response;
  card.done = done;
  card.controller = controller;

  deviceMultiCards_[subs_.key(done)] = card;

  debug_log << QObject::tr("Оформлена подписка на получение информации о состоянии устройств, всего подписчиков %1").arg(deviceMultiCards_.size());

  sendNearSession();
}


void SessionManager::slotClientSubscribed(meteo::rpc::Controller* ctrl)
{
  QMutexLocker lock(&mutex_);
  debug_log << tr("Подключился клиент '%1'.").arg(ctrl->channel()->address());
  subs_[ctrl->channel()] = ctrl->closure();
}
void SessionManager::slotClientUnsubscribed(meteo::rpc::Controller* ctrl)
{
  QMutexLocker lock(&mutex_);
  meteo::rpc::Channel* ch = ctrl->channel();
  
  if ( subs_.contains(ch) ) {
    debug_log << tr("Клиент '%1' отключился.").arg(ch->address());
    subs_.remove(ch);
    dataMultiCards_.remove(ch);
    deviceMultiCards_.remove(ch);
  }
}


//! Отправка данных для подписавшихся на получение имени файла
void SessionManager::slotFileSubscr(const QString& fileName, uint64_t sessionId)
{
  if ( dataMultiCards_.isEmpty() || fileName.isEmpty()) { return; }

  if ( sendingDataMulti_ ) { return; }
  sendingDataMulti_ = true;

  QElapsedTimer t;
  t.start();

  conf::DataReply response;
  response.set_file(fileName.toStdString());
  response.set_session_id(sessionId);
  for(auto& card : dataMultiCards_.values())
  {
    if(card.request->subscr() == conf::kFileSubsr)
    {
      card.response->CopyFrom(response);
      card.done->Run();
    }
  }
  
  sendingDataMulti_ = false;

  info_log_if(t.elapsed() > 500) << QObject::tr("Время рассылки спутниковых данных подписчикам %1 мсек.").arg(t.elapsed());
}

//! Отправка данных для подписавшихся на получение потока
void SessionManager::slotDataSubscr(const QByteArray& data, int status)
{
  if (0 == handler_) return;

  if ( dataMultiCards_.isEmpty() ) { return; }

  if ( sendingDataMulti_ ) { return; }
  sendingDataMulti_ = true;

  QElapsedTimer t;
  t.start();

  // QDateTime cur = currentDt();
  // cur.setTime(QTime(0,0,0));
  // std::string curDt = cur.date().toString(Qt::ISODate).toStdString();

  bool subscrExist = false;
  for(auto& card : dataMultiCards_.values())
  {
    if (card.request->subscr() == conf::kDataSubscr)
    {
      subscrExist = true;   
      break;
    }
  }

  if (subscrExist)
  {
    QByteArray oneres;
    handler_->processRT(currentDt(), data, &oneres);
    //saveImg(oneres);
    conf::DataReply response;
    response.set_status(mappi::conf::StatusSession(status));
    response.set_data(oneres.data(), oneres.size());
    for(auto& card : dataMultiCards_.values())
    {
      if(card.request->subscr() == conf::kDataSubscr)
      {
	debug_log << "send" <<  oneres.size();
        card.response->CopyFrom(response);
        card.done->Run();
      }
    }
  }

  sendingDataMulti_ = false;

  info_log_if(t.elapsed() > 500) << QObject::tr("Время рассылки спутниковых данных подписчикам %1 мсек.").arg(t.elapsed());
}

void SessionManager::slotDeviceSubscr(float azimut, float antAzimut, float elevation, float antElevation)
{
  if ( deviceMultiCards_.isEmpty() ) { return; }

  if ( sendingDeviceMulti_ ) { return; }
  sendingDeviceMulti_ = true;

  QElapsedTimer t;
  t.start();

  conf::DeviceStateReply response;
  //TODO  с этими полями гуя думает, что надо сеанс обновить
  //response.set_satellite(near_.getNameSat().toStdString());
  //response.set_revol(near_.getRevol());
  //response.set_time_offset(timeOffset_);
  response.mutable_antenna()->set_azimut(azimut);
  response.mutable_antenna()->set_ant_azimut(antAzimut);
  response.mutable_antenna()->set_elevation(elevation);
  response.mutable_antenna()->set_ant_elev(antElevation);

  for(auto& card : deviceMultiCards_.values())
  {
    if (card.request->subscr() == conf::kAntennaSubscr)
    {
      card.response->CopyFrom(response);
      card.done->Run();
    }
  }
  
  sendingDeviceMulti_ = false;

  info_log_if(t.elapsed() > 500) << QObject::tr("Время рассылки информации об устройствах подписчикам %1 мсек.").arg(t.elapsed());
}

void SessionManager::sendNearSession()
{
  if ( deviceMultiCards_.isEmpty() ) { return; }

  if ( sendingDeviceMulti_ ) { return; }
  sendingDeviceMulti_ = true;

  QElapsedTimer t;
  t.start();

  conf::DeviceStateReply response;
  response.set_satellite(near_.getNameSat().toStdString());
  response.set_revol(near_.getRevol());
  response.set_time_offset(timeOffset_);

  for(auto& card : deviceMultiCards_.values())
  {
    if (card.request->subscr() == conf::kAntennaSubscr)
    {
      card.response->CopyFrom(response);
      card.done->Run();
    }
  }
  
  sendingDeviceMulti_ = false;

  info_log_if(t.elapsed() > 500) << QObject::tr("Время рассылки информации об устройствах подписчикам %1 мсек.").arg(t.elapsed());
}


//для отладки
void SessionManager::saveImg(const QByteArray& data)
{
//  trc;

  QVector<QRgb> palette(256);
  for (uint i=0; i< 256; i++) {
    palette[i] = QRgb((i<<16)+ (i<<8) + i);
  }

  int thinning;
  int chcnt;
  int number;
  int rows, cols;
  QByteArray chData;
  QDataStream ds(data);
  ds >> thinning;
  ds >> chcnt;
  //debug_log << thinning << chcnt;
  for (int idx = 0; idx < chcnt; idx++) {
    ds >> number;
    ds >> chData;
    ds >> rows;
    ds >> cols;

    // var(number);
    // var(rows);
    // var(cols);
    
    if (number == 1) {
      tmpdata_.append(chData);
      tmprows_ += rows;
      // _data[number].append(chData);
      // _rows += rows;
            
      // var(_rows);
      // var(chData.size());
      // var(_data[number].size());
      QImage imqt((const uchar*)(tmpdata_.data()), cols, tmprows_, cols, QImage::Format_Indexed8);
      imqt.setColorCount(256);
      
      imqt.setColorTable(palette);
      imqt.save("/tmp/img_recv_" + QString::number(number) + ".bmp", "BMP");
    }
  }
}

void SessionManager::recreateSchedule()
{
  trc;
  if (nullptr != chReceiver_) {
    mappi::conf::IdleRequest req;
    auto* reply = chReceiver_->remoteCall(&mappi::conf::ScheduleService::Recreate, req, 60 * 1000, true);
    delete reply;
  }
}

} // receive
} // mappi
