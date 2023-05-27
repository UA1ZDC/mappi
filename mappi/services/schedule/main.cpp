#include "serverstub.h"
#include <commons/proc_read/daemoncontrol.h>
#include <mappi/global/global.h>
#include <mappi/settings/mappisettings.h>
#include <qcoreapplication.h>
#include <stdexcept>

using namespace mappi;
using namespace meteo;


int main(int argc, char *argv[])
{
  TAPPLICATION_NAME("meteo");
  QCoreApplication app(argc, argv);
  gSettings(global::Settings::instance());
  // global::setLogHandler();

  if (global::Settings::instance()->load() == false) {
    error_log << meteo::msglog::kSettingsLoadFailed;
    return EXIT_FAILURE;
  }

  QString appName = app.applicationFilePath();
  if (ProcControl::isAlreadyRun(appName)) {
    error_log << QObject::tr("Сервис %1 уже запущен").arg(appName);
    return EXIT_FAILURE;
  }

  schedule::ServiceHandler service;
  if (service.init() == false)
    return EXIT_FAILURE;

  try {
    schedule::ServerStub* stub = new schedule::ServerStub(&service);
    int port = global::service(settings::proto::kSchedule).port();
    rpc::Server* server = new rpc::Server(stub, QString("0.0.0.0:%1").arg(port));
    if (server->init() == false)
      return EXIT_FAILURE;

    info_log << QObject::tr("%1 запущен").arg(global::serviceTitle(settings::proto::kSchedule));
    debug_log << QObject::tr("Сервис опубликован по адресу: %1").arg(server->address());
  } catch (const std::exception& exp) {

    error_log << QObject::tr("При запуске сервиса произошло исключение: %1").arg(exp.what());
    return EXIT_FAILURE;
  }

  if (service.start() == false)
    return EXIT_FAILURE;

  return app.exec();
}
