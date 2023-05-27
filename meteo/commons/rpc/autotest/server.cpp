#include "server.h"
#include <qobject.h>
#include <unistd.h>
#include <qcoreapplication.h>
#include <qtimer.h>

using namespace meteo::test;


int msleep(int msecs){
  return usleep(1000*msecs);
}

void TestService::testreq( google::protobuf::RpcController* c,
                            const proto::TestMessage* req,
                            proto::TestMessage* res,
                            google::protobuf::Closure* d )
{
  Q_UNUSED(c);
  Q_UNUSED(req);
  Q_UNUSED(d);

  res->set_count(req->count());
  unsigned int random = ((unsigned int) rand() ) % 500;
  msleep(500 + random);
  d->Run();
}
