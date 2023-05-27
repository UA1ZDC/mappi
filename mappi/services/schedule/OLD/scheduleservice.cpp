#include "scheduleservice.h"

#include <qelapsedtimer.h>
#include <qfilesystemwatcher.h>

#include <sat-commons/satellite/satellitebase.h>

#include <meteo/commons/global/global.h>

#include <mappi/global/global.h>
#include <mappi/schedule/scheduler.h>

using namespace meteo;

namespace mappi {
namespace schedule {

static const QString kTle = MnCommon::varPath("mappi");
static const QString kSchedule = MnCommon::varPath("mappi") + "schedule.txt";

ScheduleService::ScheduleService()
  : watcher_(new QFileSystemWatcher(this))
  , checkTimer(new QTimer(this))
{
  QString tle = meteo::global::findWeatherFile(kTle, QDateTime::currentDateTimeUtc());
  if(false == tle.isEmpty()) {
    files_.insert(tle);
    watcher_->addPath(tle);
  }
  watcher_->addPath(kTle);
  QObject::connect(watcher_, &QFileSystemWatcher::directoryChanged, this, &ScheduleService::slotChangedTleFile);
  QObject::connect(watcher_, &QFileSystemWatcher::fileChanged,      this, [&](const QString& filename) {
    //Функция обновляет расписание если файл по которому построено расписание удалили,
    //изменили(при копировании файла событие срабатывает 2 раза)
    if(false == QFile::exists(filename)) {
      files_.remove(filename);
      slotRefreshSchedule();
    }
    else if(files_.contains(filename)) {
      slotRefreshSchedule();
    }
    else {
      files_.insert(filename);
    }
  });
  

  QObject::connect(checkTimer, &QTimer::timeout, this, &ScheduleService::slotRefreshSchedule);
  checkTimer->setInterval(60 * 60 * 1000);
  checkTimer->start();

  slotRefreshSchedule(); // Строим расписание учитывая пользовательское решение конфликтов
}

ScheduleService::~ScheduleService() {
  // delete server_; server_ = nullptr;
}

/*! \brief ScheduleService::slotChangedTleFile - изменилась директория с tle */
void ScheduleService::slotChangedTleFile()
{
  QString tleFile = meteo::global::findWeatherFile(kTle, QDateTime::currentDateTimeUtc());
  if(watcher_->files().isEmpty()) {
    watcher_->addPath(tleFile);
  }
  else if(false == watcher_->files().contains(tleFile)) {
    watcher_->removePaths(watcher_->files());
    watcher_->addPath(tleFile);
  }
}

/*! \brief ScheduleService::createSchedule - создать расписание */
int ScheduleService::createSchedule()
{
  Schedule schedule;
  Scheduler scheduler;

  if (!scheduler.create(schedule)) {
    error_log << QObject::tr("Не удалось построить расписание");
    return 1;
  }

  scheduler.resolveConfl( schedule );

  if (!scheduler.save(schedule, kSchedule)) {
    error_log << QObject::tr("Не удалось сохранить расписание в файл");
    return 2;
  }
  debug_log << QObject::tr("Построено новое расписание");
  sendSchedule(schedule);
  return 0;
}

/*! \brief ScheduleService::slotRefreshSchedule - обновить расписание */
int ScheduleService::slotRefreshSchedule()
{
  Schedule schedule;
  Scheduler scheduler;

  scheduler.restore(schedule, kSchedule);

  bool ok = scheduler.refresh(schedule);
  if(!ok) {
    error_log << QObject::tr("Не удалось построить расписание");
    return 1;
  }

  scheduler.resolveConfl(schedule, true);

  if (!scheduler.save(schedule, kSchedule)) {
    error_log << QObject::tr("Не удалось сохранить расписание в файл");
    return 2;
  }

  sendSchedule(schedule);
  return 0;
}


/*! \brief ScheduleService::Subscribe - Подписка на рассылку расписания */
void ScheduleService::Subscribe(google::protobuf::RpcController *controller
    , const conf::ScheduleSubscribeRequest *request
    , conf::ScheduleReply *response
    , google::protobuf::Closure *done)
{
  QMutexLocker lock(&mutex_);

  trc;
  meteo::rpc::Controller* ctrl = static_cast<meteo::rpc::Controller*>(controller );
  QObject::connect( ctrl->channel(), &meteo::rpc::Channel::clientUnsubscribed,
                    this, &ScheduleService::clientUnsubscribed, Qt::DirectConnection );

  subs_[ctrl->channel()] = done;
  
  ScheduleMultiCard card;
  card.request    = request;
  card.response   = response;
  card.done       = done;
  card.controller = controller;
  
  scheduleMultiCards_[ctrl->channel()] = card;
  debug_log << QObject::tr("Оформлена подписка на получение информации расписания, всего подписчиков %1")
               .arg(scheduleMultiCards_.count());
  
}

/*! \brief ScheduleService::GetSchedule - Запросить расписание */
void ScheduleService::GetSchedule(google::protobuf::RpcController *ctrl
    , const mappi::conf::IdleRequest *request, mappi::conf::ScheduleReply *response
    , google::protobuf::Closure *done)
{
  // meteo::rpc::TController* tctrl = static_cast<meteo::rpc::TController*>(ctrl);
  // if(nullptr == tctrl->channel() || nullptr == request || nullptr == response) {
  //   return;
  // }
  trc;

  schedule::Schedule sched;
  bool ok = schedule::Scheduler::restore(sched, kSchedule);
  if(ok) {
    auto count = sched.countSession();
    for(decltype(count) index = 0; index < count; ++index) {
      ::mappi::conf::Session* session = response->add_session();
      schedule::Session one = sched.getSession(index);
      session->set_satellite(one.getNameSat().toStdString());
      session->set_aos(one.getAos().toString(Qt::ISODate).toStdString());
      session->set_los(one.getLos().toString(Qt::ISODate).toStdString());
      session->set_maxel(one.getMaxEl());
      session->set_revol(one.getRevol());
      session->set_direction(conf::SatDirection(one.getDirection()));
      session->set_state(one.getState());
    }
  }
  else {
    response->set_comment(QObject::tr("Не удалось найти расписание %1").arg(kSchedule).toStdString());
  }
  response->set_result(ok);
  done->Run();
}

/*! \brief ScheduleService::EditSession - Изменить конфликт сеанса */
void ScheduleService::EditSession(google::protobuf::RpcController *ctrl
    , const mappi::conf::SessionRequest *request, mappi::conf::ScheduleReply *response
    , google::protobuf::Closure *done)
{
  // rpc::TController* tctrl = static_cast<rpc::TController*>(ctrl);
  // if(nullptr == tctrl->channel() || nullptr == request || nullptr == response) {
  //   return;
  // }

  schedule::Schedule sched;
  schedule::Scheduler scheduler;
  bool ok = schedule::Scheduler::restore(sched, kSchedule);
  if(ok) {
    auto session = sched.getSession(QString::fromStdString(request->satellite()), request->revol());
    ok = !session.isDefaults();
    if(ok) {
      session.setState(request->state());
      ok = sched.setSessionState(session);
      if(ok) {
        ok = schedule::Scheduler::save(sched, kSchedule);
        if(ok) {
          sendSchedule(sched);
        }
        else {
          response->set_comment(QObject::tr("Не удалось сохранить расписание %1").arg(kSchedule).toStdString());
        }
      }
      else {
        response->set_comment(QObject::tr("Сеанс %1 %2 не найден").arg(session.getNameSat().arg(session.getRevol())).toStdString());
      }
    }
    else {
      response->set_comment(QObject::tr("Сеанс %1 %2 не найден").arg(session.getNameSat().arg(session.getRevol())).toStdString());
    }

    auto count = sched.countSession();
    for(decltype(count) index = 0; index < count; ++index) {
      ::mappi::conf::Session* session = response->add_session();
      schedule::Session one = sched.getSession(index);
      session->set_satellite(one.getNameSat().toStdString());
      session->set_aos(one.getAos().toString(Qt::ISODate).toStdString());
      session->set_los(one.getLos().toString(Qt::ISODate).toStdString());
      session->set_maxel(one.getMaxEl());
      session->set_revol(one.getRevol());
      session->set_direction(conf::SatDirection(one.getDirection()));
      session->set_state(one.getState());
    }
  }
  else {
    response->set_comment(QObject::tr("Не удалось найти расписание %1").arg(kSchedule).toStdString());
  }

  response->set_result(ok);
  done->Run();
}

/*! \brief ScheduleService::Recreate - Перестроить расписание */
void ScheduleService::Recreate(google::protobuf::RpcController *ctrl
    , const mappi::conf::IdleRequest *request, mappi::conf::ScheduleReply *response
    , google::protobuf::Closure *done)
{
  // rpc::TController* tctrl = static_cast<rpc::TController*>(ctrl);
  // if(nullptr == tctrl->channel() || nullptr == request || nullptr == response) {
  //   return;
  // }
  switch (createSchedule())
  {
  case 1:
    response->set_comment(QObject::tr("Не удалось построить расписание").toStdString());
    response->set_result(false);
    break;
  case 2:
    response->set_comment(QObject::tr("Не удалось сохранить расписание в файл").toStdString());
    response->set_result(false);
    break;
  default:
    break;
  }

  schedule::Schedule sched;
  bool ok = schedule::Scheduler::restore(sched, kSchedule);
  if(ok) {
    auto count = sched.countSession();
    for(decltype(count) index = 0; index < count; ++index) {
      ::mappi::conf::Session* session = response->add_session();
      schedule::Session one = sched.getSession(index);
      session->set_satellite(one.getNameSat().toStdString());
      session->set_aos(one.getAos().toString(Qt::ISODate).toStdString());
      session->set_los(one.getLos().toString(Qt::ISODate).toStdString());
      session->set_maxel(one.getMaxEl());
      session->set_revol(one.getRevol());
      session->set_direction(conf::SatDirection(one.getDirection()));
      session->set_state(one.getState());
    }
  }
  else {
    response->set_comment(QObject::tr("Не удалось найти расписание %1").arg(kSchedule).toStdString());
  }

  response->set_result(ok);
  done->Run();
}

/*! \brief ScheduleService::Recreate - Перестроить расписание */
void ScheduleService::Refresh(google::protobuf::RpcController *ctrl
    , const mappi::conf::IdleRequest *request, mappi::conf::ScheduleReply *response
    , google::protobuf::Closure *done)
{
  // meteo::rpc::TController* tctrl = static_cast<rpc::TController*>(ctrl);
  // if(nullptr == tctrl->channel() || nullptr == request || nullptr == response) {
  //   return;
  // }

  switch (slotRefreshSchedule()) {
  case 1:
    response->set_comment(QObject::tr("Не удалось построить расписание").toStdString());
    response->set_result(false);
    break;
  case 2:
    response->set_comment(QObject::tr("Не удалось сохранить расписание в файл").toStdString());
    response->set_result(false);
    break;
  default:
    break;
  }
  schedule::Schedule sched;
  bool ok = schedule::Scheduler::restore(sched, kSchedule);
  if(ok) {
    auto count = sched.countSession();
    for(decltype(count) index = 0; index < count; ++index) {
      ::mappi::conf::Session* session = response->add_session();
      schedule::Session one = sched.getSession(index);
      session->set_satellite(one.getNameSat().toStdString());
      session->set_aos(one.getAos().toString(Qt::ISODate).toStdString());
      session->set_los(one.getLos().toString(Qt::ISODate).toStdString());
      session->set_maxel(one.getMaxEl());
      session->set_revol(one.getRevol());
      session->set_direction(conf::SatDirection(one.getDirection()));
      session->set_state(one.getState());
    }
  }
  else {
    response->set_comment(QObject::tr("Не удалось найти расписание %1").arg(kSchedule).toStdString());
  }

  response->set_result(ok);
  done->Run();
}

void ScheduleService::GetSTle(google::protobuf::RpcController *ctrl
    , const mappi::conf::TleRequest *request, mappi::conf::STleReply* response
    , google::protobuf::Closure *done)
{
  // rpc::TController* tctrl = static_cast<rpc::TController*>(ctrl);
  // if(nullptr == tctrl->channel() || nullptr == request || nullptr == response) {
  //   return;
  // }
  bool ok = request->has_name() && request->has_aos();
  if(ok){
    auto reception = ::mappi::inter::Settings::instance()->reception();
    QString tleFile = global::findWeatherFile(reception.has_tledir() ? QString::fromStdString(reception.tledir())
                                                                     : kTle
                                            , QDateTime::fromString(QString::fromStdString(request->aos()), Qt::ISODate));
    MnSat::STLEParams tle;
    ok &= SatelliteBase::getSTLE(QString::fromStdString(request->name()), tleFile, &tle);
    if(ok) {
      response->set_name(tle.satName.toStdString());
      response->set_firststring(tle.firstString.toStdString());
      response->set_secondstring(tle.secondString.toStdString());
    }
  }
  response->set_result(ok);
  done->Run();
}


void ScheduleService::GetAllSatellites(google::protobuf::RpcController *ctrl
    , const mappi::conf::IdleRequest *request, mappi::conf::AllSatellitesReply* response
    , google::protobuf::Closure *done)
{
  // rpc::TController* tctrl = static_cast<rpc::TController*>(ctrl);
  // if(nullptr == tctrl->channel() || nullptr == request || nullptr == response) {
  //   return;
  // }

  auto reception = mappi::inter::Settings::instance()->reception();
  QString tleFile = global::findWeatherFile(reception.has_tledir() ? QString::fromStdString(reception.tledir())
                                                                   : kTle
                                          , QDateTime::currentDateTimeUtc());
  QStringList satellites;
  SatelliteBase::readSatelliteList(tleFile, &satellites);
  for(const auto& sat : satellites) {
    response->add_name()->append(sat.toStdString());
  }
  done->Run();
}

void ScheduleService::sendSchedule(const schedule::Schedule& sched)
{
  trc;
  QElapsedTimer t;
  t.start();
  auto count = sched.countSession();

  conf::ScheduleReply response;
  response.set_result(true);

  for(decltype(count) index = 0; index < count; ++index)
  {
    ::mappi::conf::Session* session = response.add_session();
    schedule::Session one = sched.getSession(index);
    session->set_satellite(one.getNameSat().toStdString());
    session->set_aos(one.getAos().toString(Qt::ISODate).toStdString());
    session->set_los(one.getLos().toString(Qt::ISODate).toStdString());
    session->set_maxel(one.getMaxEl());
    session->set_revol(one.getRevol());
    session->set_direction(conf::SatDirection(one.getDirection()));
    session->set_state(one.getState());
  }

  for(auto& card : scheduleMultiCards_.values())
  {
    if(card.request->has_subscr())
    {
      if(card.request->subscr())
      {
        card.response->CopyFrom(response);
        card.done->Run();
      }
    }
  }
  info_log_if(t.elapsed() > 500) << QObject::tr("Время рассылки информации об устройствах подписчикам %1 мсек.").arg(t.elapsed());
}


// void ScheduleService::clientSubscribed(meteo::rpc::Controller* ctrl)
// {
//   debug_log << tr("Подключился клиент '%1'.").arg(ch->address().asString());
//   subs_[ch] = done;
// }

void ScheduleService::clientUnsubscribed(meteo::rpc::Controller* ctrl)
{
  if (nullptr == ctrl) return;

  QMutexLocker lock(&mutex_);
  meteo::rpc::Channel* ch = ctrl->channel();

  if (nullptr == ch) return;
  
  if(subs_.contains(ch)) {
    debug_log << tr("Клиент '%1' отключился.").arg(ch->address());
    subs_.remove(ch);
    scheduleMultiCards_.remove(ch);
  }
}

} // schedule
} // mappi
