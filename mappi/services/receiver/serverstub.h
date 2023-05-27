#pragma once

#include "service.h"
#include <meteo/commons/rpc/rpc.h>


namespace mappi {

namespace receiver {

using namespace meteo;
using namespace google::protobuf;


class ServerStub :
  public conf::ReceiverService
{
public :
  ServerStub(ServiceHandler* service);
  virtual ~ServerStub();

  virtual void AcceptSession(RpcController* ctrl, const conf::SessionBundle* req, conf::ReceiverResponse* resp, Closure* done);
  virtual void ServiceMode(RpcController* ctrl, const ServiceRequest* req, conf::ReceiverResponse* resp, Closure* done);
  virtual void HealthCheck(RpcController* ctrl, const Dummy* req, conf::ReceiverResponse* resp, Closure* done);

private :
  ServiceHandler* service_;
};

}

}
