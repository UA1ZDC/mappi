#ifndef METEO_UKAZ_APPMANAGER_APPMANAGERDAEMON_H
#define METEO_UKAZ_APPMANAGER_APPMANAGERDAEMON_H

#include <commons/qtservice/qtservice.h>

namespace rpc {
class Server;
} // rpc

namespace meteo {
namespace app {
class Control;
class Service;
} // app
} // meteo

class AppManagerDaemon : public QtService<QCoreApplication>
{
public:
  AppManagerDaemon(int argc, char** argv);

protected:
  virtual void start();
  virtual void stop();

private:
  meteo::app::Control* control_;
  meteo::app::Service* service_;
  rpc::Server* server_;
};

#endif // METEO_UKAZ_APPMANAGER_APPMANAGERDAEMON_H
