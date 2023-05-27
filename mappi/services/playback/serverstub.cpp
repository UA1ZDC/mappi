#include "serverstub.h"


namespace mappi {

namespace playback {

ServerStub::ServerStub(ServiceHandler* service) :
  service_(service)
{
}

ServerStub::~ServerStub()
{
  service_ = nullptr;
}

void ServerStub::ServiceMode(RpcController* ctrl, const ServiceRequest* req, conf::ReceptionResponse* resp, Closure* done)
{
  Q_UNUSED(ctrl)

  resp->set_result(false);
  service_->serviceMode(req, resp);
  done->Run();
}

void ServerStub::HealthCheck(RpcController* ctrl, const Dummy* req, conf::ReceptionResponse* resp, Closure* done)
{
  Q_UNUSED(ctrl)

  resp->set_result(false);
  service_->healthCheck(req, resp);
  done->Run();
}

}

}
