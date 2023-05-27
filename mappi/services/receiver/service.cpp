#include "service.h"

#define SERVICE_ERROR(text) \
  resp->set_comment(text.toStdString()); \
  error_log << text \


namespace mappi {

namespace receiver {

ServiceHandler::ServiceHandler(QObject* parent /*=*/) :
    QObject(parent),
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

void ServiceHandler::acceptSession(const conf::SessionBundle* req, conf::ReceiverResponse* resp)
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

void ServiceHandler::serviceMode(const ServiceRequest* req, conf::ReceiverResponse* resp)
{
  fsm_->proceed(req->flag() ? Primitive::SERVICE_MODE_ON : Primitive::SERVICE_MODE_OFF);

  setResponse(resp);
  if (ctx_->hasError()) {
    SERVICE_ERROR(ctx_->lastError());
    return ;
  }

  resp->set_result(true);
}

void ServiceHandler::healthCheck(const Dummy* req, conf::ReceiverResponse* resp)
{
  Q_UNUSED(req)

  setResponse(resp);
  if (ctx_->hasError()) {
    SERVICE_ERROR(ctx_->lastError());
    return ;
  }

  resp->set_result(true);
}

void ServiceHandler::setResponse(conf::ReceiverResponse* resp)
{
  resp->set_state(static_cast<ServiceState>(CURRENT_STATE(fsm_)));
  resp->set_apch(ctx_->conf.apch);

  conf::ReceiverParam* param = resp->mutable_param();
  param->set_mode(ctx_->conf.mode);
  Parameters::toProto(ctx_->conf.parameters(ctx_->session.data().satellite), param);
}

}

}
