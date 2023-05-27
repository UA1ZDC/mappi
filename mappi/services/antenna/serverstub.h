#pragma once

#include "service.h"
#include <meteo/commons/rpc/rpc.h>


namespace mappi {

namespace antenna {

using namespace meteo;
using namespace google::protobuf;


class ServerStub :
  public conf::AntennaService
{
public :
  ServerStub(ServiceHandler* service);
  virtual ~ServerStub();

  virtual void AcceptSession(RpcController* ctrl, const conf::SessionBundle* req, conf::AntennaResponse* resp, Closure* done);
  virtual void ServiceMode(RpcController* ctrl, const ServiceRequest* req, conf::AntennaResponse* resp, Closure* done);
  virtual void HealthCheck(RpcController* ctrl, const Dummy* req, conf::AntennaResponse* resp, Closure* done);

  virtual void FeedhornPower(RpcController* ctrl, const conf::Feedhorn* req, conf::AntennaResponse* resp, Closure* done);
  virtual void FeedhornOutput(RpcController* ctrl, const conf::Feedhorn* req, conf::AntennaResponse* resp, Closure* done);

  virtual void SetPosition(RpcController* ctrl, const conf::Position* req, conf::AntennaResponse* resp, Closure* done);
  virtual void Stop(RpcController* ctrl, const Dummy* req, conf::AntennaResponse* resp, Closure* done);
  virtual void MonkeyTest(RpcController* ctrl, const conf::SpinParam* req, conf::AntennaResponse* resp, Closure* done);
  virtual void TravelMode(RpcController* ctrl, const Dummy* req, conf::AntennaResponse* resp, Closure* done);

  virtual void Subscribe(RpcController* ctrl, const Dummy* req, conf::AntennaResponse* resp, Closure* done);

private :
  ServiceHandler* service_;
};

}

}
