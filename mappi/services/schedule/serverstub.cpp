#include "serverstub.h"


namespace mappi {

namespace schedule {

ServerStub::ServerStub(ServiceHandler* service) :
  service_(service)
{
}

ServerStub::~ServerStub()
{
  service_ = nullptr;
}

void ServerStub::MakeSchedule(RpcController* ctrl, const Dummy* req, conf::ScheduleResponse* resp, Closure* done)
{
  Q_UNUSED(ctrl)

  resp->set_result(false);
  service_->makeSchedule(req, resp);
  done->Run();
}

void ServerStub::CurrentSchedule(RpcController* ctrl, const Dummy* req, conf::ScheduleResponse* resp, Closure* done)
{
  Q_UNUSED(ctrl)

  resp->set_result(false);
  service_->currentSchedule(req, resp);
  done->Run();
}

void ServerStub::EditSession(RpcController* ctrl, const conf::Session* req, conf::SessionResponse* resp, Closure* done)
{
  Q_UNUSED(ctrl)

  resp->set_result(false);
  service_->editSession(req, resp);
  done->Run();
}

void ServerStub::NearSession(RpcController* ctrl, const Dummy* req, conf::SessionResponse* resp, Closure* done)
{
  Q_UNUSED(ctrl)

  resp->set_result(false);
  service_->nearSession(req, resp);
  done->Run();
}

void ServerStub::NextSession(RpcController* ctrl, const conf::Session* req, conf::SessionResponse* resp, Closure* done)
{
  Q_UNUSED(ctrl)

  resp->set_result(false);
  service_->nextSession(req, resp);
  done->Run();
}

void ServerStub::TleSatellite(RpcController* ctrl, const conf::SessionInfo* req, conf::SessionResponse* resp, Closure* done)
{
  Q_UNUSED(ctrl)

  resp->set_result(false);
  service_->tleSatellite(req, resp);
  done->Run();
}

void ServerStub::TleSatelliteList(RpcController* ctrl, const Dummy* req, conf::SatelliteResponse* resp, Closure* done)
{
  Q_UNUSED(ctrl)

  resp->set_result(false);
  service_->tleSatelliteList(req, resp);
  done->Run();
}

}

}
