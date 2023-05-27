#include "service.h"
#include <sat-commons/satellite/satellitebase.h>

#define SERVICE_ERROR(text) \
  resp->set_comment(text.toStdString()); \
  error_log << text \


namespace mappi {

namespace schedule {

static const int PERCENT_COMPLETION = 20;
static const QString TLE_FILE_HANDLER_FAIL = QObject::tr("Ошибка обработки tle-файла");

ServiceHandler::ServiceHandler(QObject* parent /*=*/) :
    QObject(parent),
  ctx_(new Context),
  tleMonitor_(new TleMonitor(this))
{
  QObject::connect(tleMonitor_, SIGNAL(update()), this, SLOT(tleUpdate()));
}

ServiceHandler::~ServiceHandler()
{
  delete ctx_;
  ctx_ = nullptr;
}

bool ServiceHandler::init()
{
  // WARNING конфигурация читаеться только один раз, при старте сервиса
  if (ctx_->confLoad()) {
    tleMonitor_->start(ctx_->conf.tlePath);
    return true;
  }

  return false;
}

bool ServiceHandler::start()
{
  ctx_->makeSchedule();
  if (ctx_->hasError()) {
    error_log << ctx_->lastError();
    return false;
  }

  return true;
}

void ServiceHandler::makeSchedule(const Dummy* req, conf::ScheduleResponse* resp)
{
  Q_UNUSED(req)

  ctx_->makeSchedule();
  if (ctx_->hasError()) {
    SERVICE_ERROR(ctx_->lastError());
    return ;
  }

  scheduleToMessage(resp);

  resp->set_result(true);
}

void ServiceHandler::currentSchedule(const Dummy* req, conf::ScheduleResponse* resp)
{
  Q_UNUSED(req)

  if (PERCENT_COMPLETION <= ctx_->schedule.percentCompleted()) {
    info_log << QObject::tr("Превышена отметка принятых сеансов: %1, расписание будет обновлено")
      .arg(PERCENT_COMPLETION);
    ctx_->makeSchedule();

    if (ctx_->hasError()) {
      SERVICE_ERROR(ctx_->lastError());
      return ;
    }
  }

  scheduleToMessage(resp);

  resp->set_result(true);
}

void ServiceHandler::editSession(const conf::Session* req, conf::SessionResponse* resp)
{
  Session& session = ctx_->schedule.getSession(req->satellite().c_str(), req->revol());
  if (session.data().isDefault()) {
    SERVICE_ERROR(QObject::tr("Сеанс не найден: %1, %2")
      .arg(req->satellite().c_str())
      .arg(req->revol())
    );
    return ;
  }

  // WARNING правится только состояние конфликта
  session.data().conflState = req->confl_state();
  ctx_->saveSchedule();

  Session::toProto(session, resp->mutable_session());

  debug_log << QString("edit session(%1)").arg(session.data().info());
  resp->set_result(true);
}

void ServiceHandler::nearSession(const Dummy* req, conf::SessionResponse* resp)
{
  Q_UNUSED(req)

  Session& session = ctx_->schedule.near();
  // похоже на случай, когда расписание закончилось
  if (session.data().isDefault()) {
    ctx_->makeSchedule();
    if (ctx_->hasError()) {
      SERVICE_ERROR(ctx_->lastError());
      return ;
    }
  }

  Session::toProto(session, resp->mutable_session());
  if (tleToMessage(session.data().satellite, resp->mutable_tle()) == false) {
    SERVICE_ERROR(TLE_FILE_HANDLER_FAIL);
    return ;
  }

  debug_log << QString("near session(%1)").arg(session.data().info());
  resp->set_result(true);
}

void ServiceHandler::nextSession(const conf::Session* req, conf::SessionResponse* resp)
{
  Session tmp(*req);
  QDateTime dt = QDateTime::currentDateTimeUtc();
  if (!tmp.data().isDefault())
    dt = tmp.data().los.addSecs(1);

  Session& session = ctx_->schedule.near(dt);
  // похоже на случай, когда расписание закончилось
  if (session.data().isDefault()) {
    ctx_->makeSchedule();
    if (ctx_->hasError()) {
      SERVICE_ERROR(ctx_->lastError());
      return ;
    }
  }

  Session::toProto(session, resp->mutable_session());
  if (tleToMessage(session.data().satellite, resp->mutable_tle()) == false) {
    SERVICE_ERROR(TLE_FILE_HANDLER_FAIL);
    return ;
  }

  debug_log << QString("next session(%1)").arg(session.data().info());
  resp->set_result(true);
}

void ServiceHandler::tleSatellite(const conf::SessionInfo* req, conf::SessionResponse* resp)
{
  if (tleToMessage(req->satellite().c_str(), resp->mutable_tle()) == false) {
    SERVICE_ERROR(TLE_FILE_HANDLER_FAIL);
    return ;
  }

  resp->set_result(true);
}

void ServiceHandler::tleSatelliteList(const Dummy* req, conf::SatelliteResponse* resp)
{
  Q_UNUSED(req)

  QStringList list;
  if (SatelliteBase::readSatelliteList(ctx_->conf.tlePath, &list) == false) {
    SERVICE_ERROR(TLE_FILE_HANDLER_FAIL);
    return ;
  }

  for (auto item : list)
    resp->add_list()->append(item.toStdString());

  resp->set_result(true);
}

bool ServiceHandler::tleToMessage(const QString& satellite, conf::TleItem* tle)
{
  MnSat::STLEParams tleParam;
  if (SatelliteBase::getSTLE(satellite, ctx_->conf.tlePath, &tleParam) == true) {
    tle->set_title(satellite.toStdString());
    tle->set_line_1(tleParam.firstString.toStdString());
    tle->set_line_2(tleParam.secondString.toStdString());

    return true;
  }

  return false;
}

void ServiceHandler::scheduleToMessage(conf::ScheduleResponse* resp)
{
  // WARNING пустое расписание не считается ошибкой
  int size = ctx_->schedule.size();
  for (int i = 0; i < size; ++i)
    Session::toProto(ctx_->schedule.getSession(i), resp->add_list());
}

void ServiceHandler::tleUpdate()
{
  info_log << QObject::tr("tle-файл обновлен");

  ctx_->makeSchedule();
  if (ctx_->hasError())
    error_log << ctx_->lastError();
}

}

}
