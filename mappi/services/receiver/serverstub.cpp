#include "serverstub.h"


namespace mappi {

namespace receiver {

ServerStub::ServerStub(ServiceHandler* service) :
  service_(service)
{
}

ServerStub::~ServerStub()
{
  service_ = nullptr;
}

void ServerStub::AcceptSession(RpcController* ctrl, const conf::SessionBundle* req, conf::ReceiverResponse* resp, Closure* done)
{
  Q_UNUSED(ctrl)

  resp->set_result(false);
  service_->acceptSession(req, resp);
  done->Run();
}

void ServerStub::ServiceMode(RpcController* ctrl, const ServiceRequest* req, conf::ReceiverResponse* resp, Closure* done)
{
  Q_UNUSED(ctrl)

  resp->set_result(false);
  service_->serviceMode(req, resp);
  done->Run();
}

void ServerStub::HealthCheck(RpcController* ctrl, const Dummy* req, conf::ReceiverResponse* resp, Closure* done)
{
  Q_UNUSED(ctrl)

  resp->set_result(false);
  service_->healthCheck(req, resp);
  done->Run();
}

}

}
