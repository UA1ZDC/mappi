#include "service.h"
#include <QTimerEvent>

#define SERVICE_ERROR(text) \
  resp->set_comment(text.toStdString()); \
  error_log << text \


namespace mappi {

namespace antenna {

static int NOTIFY_TIMER = 1000; // 1 сек.

ServiceHandler::ServiceHandler(QObject* parent /*=*/) :
    QObject(parent),
  timerId_(-1),
  ctx_(new Context),
  fsm_(new FSM(ctx_, this))
{
}

ServiceHandler::~ServiceHandler()
{
  delete ctx_;
  ctx_ = nullptr;
}

bool ServiceHandler::init()
{
  return ctx_->init();
}

bool ServiceHandler::start()
{
  fsm_->proceed(Primitive::ACTIVE_SESSION);
  return true;
}

void ServiceHandler::acceptSession(const conf::SessionBundle* req, conf::AntennaResponse* resp)
{
  ctx_->tleItem_prep = req->tle();
  schedule::Session::fromProto(req->session(), &ctx_->sessionOriginal_prep);
  schedule::Session::fromProto(req->session(), &ctx_->session_prep);
  if (ctx_->conf.demoMode) {
    ctx_->session_prep.data().aos = schedule::SessionData::dateFromString(req->demo_aos().c_str());
    ctx_->session_prep.data().los = ctx_->session_prep.data().aos.addSecs(ctx_->sessionOriginal_prep.duration());
  }

  if (ctx_->isReplay()) {
    warning_log << QObject::tr("Повторное уведомление, сеанс(%1)")
      .arg(ctx_->sessionOriginal_prep.data().info());

    ctx_->sessionOriginal_prep.data().setDefault();
  } else
    fsm_->proceed(Primitive::ACCEPT_SESSION);

  setResponse(resp);
  if (ctx_->hasError()) {
    SERVICE_ERROR(ctx_->lastError());
    return ;
  }

  resp->set_result(true);
}

void ServiceHandler::serviceMode(const ServiceRequest* req, conf::AntennaResponse* resp)
{
  fsm_->proceed(req->flag() ? Primitive::SERVICE_MODE_ON : Primitive::SERVICE_MODE_OFF);

  setResponse(resp);
  if (ctx_->hasError()) {
    SERVICE_ERROR(ctx_->lastError());
    return ;
  }

  resp->set_result(true);
}

void ServiceHandler::healthCheck(const Dummy* req, conf::AntennaResponse* resp)
{
  Q_UNUSED(req)

  setResponse(resp);
  if (ctx_->hasError()) {
    SERVICE_ERROR(ctx_->lastError());
    return ;
  }

  resp->set_result(true);
}

void ServiceHandler::feedhornPower(const conf::Feedhorn* req, conf::AntennaResponse* resp)
{
  if (req->has_power_on()) {
    debug_log << QString("feedhorn power(1): %1").arg(req->power_on());
    ctx_->antenna->feedhorn(req->power_on());
  }

  setResponse(resp);
  if (ctx_->hasError()) {
    SERVICE_ERROR(ctx_->lastError());
    return ;
  }

  resp->set_result(true);
}

void ServiceHandler::feedhornOutput(const conf::Feedhorn* req, conf::AntennaResponse* resp)
{
  if (req->has_output_1()) {
    debug_log << QString("output(1): %1").arg(req->output_1());
    ctx_->antenna->feedhornOutput(Feedhorn::OUTPUT_1, req->output_1());
  }

  if (req->has_output_2()) {
    debug_log << QString("output(2): %1").arg(req->output_2());
    ctx_->antenna->feedhornOutput(Feedhorn::OUTPUT_2, req->output_2());
  }

  if (req->has_output_3()) {
    debug_log << QString("output(3): %1").arg(req->output_3());
    ctx_->antenna->feedhornOutput(Feedhorn::OUTPUT_3, req->output_3());
  }

  if (req->has_output_4()) {
    debug_log << QString("output(4): %1").arg(req->output_4());
    ctx_->antenna->feedhornOutput(Feedhorn::OUTPUT_4, req->output_4());
  }

  setResponse(resp);
  if (ctx_->hasError()) {
    SERVICE_ERROR(ctx_->lastError());
    return ;
  }

  resp->set_result(true);
}

void ServiceHandler::setPosition(const conf::Position* req, conf::AntennaResponse* resp)
{
  ctx_->direct->setPosition(req->azimut(), req->elevat(), req->azimut_speed(), req->elevat_speed(), true);
  fsm_->proceed(Primitive::ANTENNA_SET_POSITION);

  setResponse(resp);
  if (ctx_->hasError()) {
    SERVICE_ERROR(ctx_->lastError());
    return ;
  }

  resp->set_result(true);
}

void ServiceHandler::stop(const Dummy* req, conf::AntennaResponse* resp)
{
  Q_UNUSED(req)

  fsm_->proceed(Primitive::ANTENNA_STOP);

  setResponse(resp);
  if (ctx_->hasError()) {
    SERVICE_ERROR(ctx_->lastError());
    return ;
  }

  resp->set_result(true);
}

void ServiceHandler::monkeyTest(const conf::SpinParam* req, conf::AntennaResponse* resp)
{
  ctx_->mTest->setDSA(req->dsa());
  ctx_->mTest->setDSE(req->dse());
  fsm_->proceed(Primitive::ANTENNA_MONKEY_TEST);

  setResponse(resp);
  if (ctx_->hasError()) {
    SERVICE_ERROR(ctx_->lastError());
    return ;
  }

  resp->set_result(true);
}

void ServiceHandler::travelMode(const Dummy* req, conf::AntennaResponse* resp)
{
  Q_UNUSED(req)

  fsm_->proceed(Primitive::ANTENNA_TRAVEL_MODE);

  setResponse(resp);
  if (ctx_->hasError()) {
    SERVICE_ERROR(ctx_->lastError());
    return ;
  }

  resp->set_result(true);
}

void ServiceHandler::subscribe(meteo::rpc::Controller* ctrl, conf::AntennaResponse* resp)
{
  if (subscriber_.contains(ctrl))
    return ;

  subscriber_[ctrl] = resp;
  info_log << QString("подписка оформлена, host(%1)").arg(ctrl->channel()->address());

  if (timerId_ == -1)
    timerId_ = startTimer(NOTIFY_TIMER);
}

void ServiceHandler::unsubscribe(meteo::rpc::Controller* ctrl)
{
  subscriber_.remove(ctrl);
  info_log << QString("отписка, host(%1)").arg(ctrl->channel()->address());

  if (subscriber_.isEmpty()) {
    killTimer(timerId_);
    timerId_ = -1;

    debug_log << QString("подписчиков нет");
  }
}

void ServiceHandler::timerEvent(QTimerEvent* event)
{
  if (event->timerId() != timerId_)
    return ;

  QMapIterator<meteo::rpc::Controller*, conf::AntennaResponse*> it(subscriber_);
  while (it.hasNext()) {
    it.next();

    meteo::rpc::Controller* ctrl = it.key();

    setResponse(it.value());
    it.value()->set_result(true);

    ctrl->closure()->Run();
  }
}

void ServiceHandler::setResponse(conf::AntennaResponse* resp)
{
  Drive azimut;
  Drive elevat;
  Feedhorn feedhorn;
  ctx_->antenna->snapshot(&azimut, &elevat, &feedhorn);

  conf::Drive* drive = resp->mutable_azimut();
  drive->set_dst(azimut.dst);
  drive->set_self(azimut.self);
  drive->set_state(azimut.state);

  drive = resp->mutable_elevat();
  drive->set_dst(elevat.dst);
  drive->set_self(elevat.self);
  drive->set_state(elevat.state);

  conf::Feedhorn* fh = resp->mutable_feedhorn();
  fh->set_power_on(feedhorn.isTurnOn());
  fh->set_output_1(feedhorn.isActiveOutput(Feedhorn::OUTPUT_1));
  fh->set_output_2(feedhorn.isActiveOutput(Feedhorn::OUTPUT_2));
  fh->set_output_3(feedhorn.isActiveOutput(Feedhorn::OUTPUT_3));
  fh->set_output_4(feedhorn.isActiveOutput(Feedhorn::OUTPUT_4));

  resp->set_state(static_cast<ServiceState>(CURRENT_STATE(fsm_)));
}

}

}
