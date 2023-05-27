#pragma once

#include "service.h"
#include <meteo/commons/rpc/rpc.h>


namespace mappi {

namespace schedule {

using namespace meteo;
using namespace google::protobuf;


class ServerStub :
  public conf::ScheduleService
{
public :
  ServerStub(ServiceHandler* service);
  virtual ~ServerStub();

  virtual void MakeSchedule(RpcController* ctrl, const Dummy* req, conf::ScheduleResponse* resp, Closure* done);
  virtual void CurrentSchedule(RpcController* ctrl, const Dummy* req, conf::ScheduleResponse* resp, Closure* done);

  virtual void EditSession(RpcController* ctrl, const conf::Session* req, conf::SessionResponse* resp, Closure* done);
  virtual void NearSession(RpcController* ctrl, const Dummy* req, conf::SessionResponse* resp, Closure* done);
  virtual void NextSession(RpcController* ctrl, const conf::Session* req, conf::SessionResponse* resp, Closure* done);

  virtual void TleSatellite(RpcController* ctrl, const conf::SessionInfo* req, conf::SessionResponse* resp, Closure* done);
  virtual void TleSatelliteList(RpcController* ctrl, const Dummy* req, conf::SatelliteResponse* resp, Closure* done);

private :
  ServiceHandler* service_;
};

}

}
