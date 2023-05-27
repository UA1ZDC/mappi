#include <stdlib.h>

#include <qcoreapplication.h>
#include <qhostaddress.h>

#include "test.h"

#include <cross-commons/debug/tlog.h>
#include <meteo/commons/rpc/channel.h>

namespace meteo {
namespace rpc {

Stub::Stub()
{
}

Stub::~Stub()
{
}

bool Stub::connectToAddress( const QString& addr )
{
  Channel ch;
  
  bool res = ch.connect(addr);
  debug_log << "CONNECTRESULT =" << res;
  return res;
}

}
}

int main( int argc, char* argv[] )
{
  QCoreApplication app( argc, argv );
  meteo::rpc::Stub stub;
  if ( false == stub.connectToAddress( "127.0.0.1:34567" ) ) {
    return EXIT_FAILURE;
  }

  app.exec();
  return EXIT_SUCCESS;
}
