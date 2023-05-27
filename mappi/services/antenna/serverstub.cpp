#include "serverstub.h"


namespace mappi {

namespace antenna {

ServerStub::ServerStub(ServiceHandler* service) :
  service_(service)
{
}

ServerStub::~ServerStub()
{
  service_ = nullptr;
}

void ServerStub::AcceptSession(RpcController* ctrl, const conf::SessionBundle* req, conf::AntennaResponse* resp, Closure* done)
{
  Q_UNUSED(ctrl)

  resp->set_result(false);
  service_->acceptSession(req, resp);
  done->Run();
}

void ServerStub::ServiceMode(RpcController* ctrl, const ServiceRequest* req, conf::AntennaResponse* resp, Closure* done)
{
  Q_UNUSED(ctrl)

  resp->set_result(false);
  service_->serviceMode(req, resp);
  done->Run();
}

void ServerStub::HealthCheck(RpcController* ctrl, const Dummy* req, conf::AntennaResponse* resp, Closure* done)
{
  Q_UNUSED(ctrl)

  resp->set_result(false);
  service_->healthCheck(req, resp);
  done->Run();
}

void ServerStub::FeedhornPower(RpcController* ctrl, const conf::Feedhorn* req, conf::AntennaResponse* resp, Closure* done)
{
  Q_UNUSED(ctrl)

  resp->set_result(false);
  service_->feedhornPower(req, resp);
  done->Run();
}

void ServerStub::FeedhornOutput(RpcController* ctrl, const conf::Feedhorn* req, conf::AntennaResponse* resp, Closure* done)
{
  Q_UNUSED(ctrl)

  resp->set_result(false);
  service_->feedhornOutput(req, resp);
  done->Run();
}

void ServerStub::SetPosition(RpcController* ctrl, const conf::Position* req, conf::AntennaResponse* resp, Closure* done)
{
  Q_UNUSED(ctrl)

  resp->set_result(false);
  service_->setPosition(req, resp);
  done->Run();
}

void ServerStub::Stop(RpcController* ctrl, const Dummy* req, conf::AntennaResponse* resp, Closure* done)
{
  Q_UNUSED(ctrl)

  resp->set_result(false);
  service_->stop(req, resp);
  done->Run();
}

void ServerStub::MonkeyTest(RpcController* ctrl, const conf::SpinParam* req, conf::AntennaResponse* resp, Closure* done)
{
  Q_UNUSED(ctrl)

  resp->set_result(false);
  service_->monkeyTest(req, resp);
  done->Run();
}

void ServerStub::TravelMode(RpcController* ctrl, const Dummy* req, conf::AntennaResponse* resp, Closure* done)
{
  Q_UNUSED(ctrl)

  resp->set_result(false);
  service_->travelMode(req, resp);
  done->Run();
}

void ServerStub::Subscribe(RpcController* ctrl, const Dummy* req, conf::AntennaResponse* resp, Closure* done)
{
  Q_UNUSED(done);
  Q_UNUSED(req);
  meteo::rpc::Controller* controller = static_cast<meteo::rpc::Controller*>(ctrl);
  service_->subscribe(controller, resp);
}

}

}
