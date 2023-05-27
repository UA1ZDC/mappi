#include <meteo/commons/rpc/autotest/rpctimeout/testserver.pb.h>

namespace meteo {
namespace test {

class TestService: public proto::TestService {

  virtual void testreq( google::protobuf::RpcController* c,
                         const proto::TestMessage* req,
                         proto::TestMessage* res,
                         google::protobuf::Closure* d ) override;

};

}
}
