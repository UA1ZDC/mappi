#include "service.h"

#define SERVICE_ERROR(text) \
  resp->set_comment(text.toStdString()); \
  error_log << text \


namespace mappi {

namespace playback {

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
  if (ctx_->confLoad() == false)
    return false;

  info_log << (ctx_->demoMode ? "demo" : "auto") << "work mode";
  return true;
}

bool ServiceHandler::start()
{
  fsm_->proceed(Primitive::ACCEPT_SESSION);
  return true;
}

void ServiceHandler::serviceMode(const ServiceRequest* req, conf::ReceptionResponse* resp)
{
  fsm_->proceed(req->flag() ? Primitive::SERVICE_MODE_ON : Primitive::SERVICE_MODE_OFF);

  setResponse(resp);
  if (ctx_->hasError()) {
    SERVICE_ERROR(ctx_->lastError());
    return ;
  }

  resp->set_result(true);
}

void ServiceHandler::healthCheck(const Dummy* req, conf::ReceptionResponse* resp)
{
  Q_UNUSED(req)

  setResponse(resp);
  if (ctx_->hasError()) {
    SERVICE_ERROR(ctx_->lastError());
    return ;
  }

  resp->set_result(true);
}

void ServiceHandler::setResponse(conf::ReceptionResponse* resp)
{
  resp->set_state(static_cast<ServiceState>(CURRENT_STATE(fsm_)));
  Session::toProto(ctx_->session, resp->mutable_session());
  *resp->mutable_tle() = ctx_->tleItem;
  if (ctx_->demoMode)
    resp->set_demo_aos(SessionData::dateToString(ctx_->actualAos()).toStdString());
}

}

}
