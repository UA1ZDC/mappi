#pragma once

#include "service.h"
#include <meteo/commons/rpc/rpc.h>


namespace mappi {

namespace playback {

using namespace meteo;
using namespace google::protobuf;


class ServerStub :
  public conf::ReceptionService
{
public :
  ServerStub(ServiceHandler* service);
  virtual ~ServerStub();

  virtual void ServiceMode(RpcController* ctrl, const ServiceRequest* req, conf::ReceptionResponse* resp, Closure* done);
  virtual void HealthCheck(RpcController* ctrl, const Dummy* req, conf::ReceptionResponse* resp, Closure* done);

private :
  ServiceHandler* service_;
};

}

}
