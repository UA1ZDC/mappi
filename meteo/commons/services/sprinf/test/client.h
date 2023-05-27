#ifndef CLIENT_H
#define CLIENT_H

#include <meteo/commons/proto/sprinf.pb.h>
#include <meteo/commons/rpc/rpc.h>

namespace sprinfclient {

  void testGetMilitaryDistrict(meteo::rpc::Channel* ctrl);


}

#endif
