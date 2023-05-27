#include <qcoreapplication.h>
#include <qstringlist.h>

#include <commons/proc_read/daemoncontrol.h>

#include <meteo/commons/rpc/rpc.h>

#include <meteo/commons/services/sprinf/sprinfservice.h>
#include <meteo/commons/services/sprinf/clienthandler.h>

#include <meteo/commons/global/global.h>
#include <meteo/commons/settings/settings.h>

#include <locale.h>


int appMain(int argc, char *argv[])
{
  int result = EXIT_SUCCESS;
  meteo::global::setLogHandler();
  TAPPLICATION_NAME("meteo");
  try {

    QCoreApplication app(argc, argv);
    setlocale(LC_NUMERIC, "C");

    meteo::gSettings(meteo::global::Settings::instance());

    if ( false == meteo::global::Settings::instance()->load() ) {
      error_log << QObject::tr("Не удалось загрузить настройки.");
      return EXIT_FAILURE;
    }
    //    TLog::setMsgHandler(meteo::global::stdOutWithTime);
    QString appName = app.applicationFilePath();
    if(ProcControl::isAlreadyRun(appName)) {
      error_log << QObject::tr("Процесс %1 уже запущен.").arg(appName);
      return EXIT_FAILURE;
    }
    ::meteo::AppStatusThread* status = new ::meteo::AppStatusThread;

    meteo::sprinf::TSprinfService service(status);
    meteo::rpc::Server server(&service, meteo::global::serviceAddress(meteo::settings::proto::kSprinf));
    if ( false == server.init() ) {
      error_log << QObject::tr("Не создан сервер связи с клиентами");
      return EXIT_FAILURE;
    }

    QObject::connect(&server, SIGNAL(clientConnected(meteo::rpc::Channel*)), service.handler(), SLOT(slotClientConnected(meteo::rpc::Channel*)));

    QObject::connect(&server, &meteo::rpc::Server::ansverSendedError,
                      &service,&meteo::sprinf::TSprinfService::slotAnsverSendedError );
    QObject::connect(&server, &meteo::rpc::Server::ansverSendedOK,
                      &service,&meteo::sprinf::TSprinfService::slotAnsverSendedOK );
    status->setUpdateLimit(1000);
    status->setSendLimit(5000);
    status->start();

    info_log << QObject::tr("%1 запущена").arg(meteo::global::serviceTitle(meteo::settings::proto::kSprinf));

    app.exec();
  }
  catch(const std::bad_alloc&) {
    critical_log << QObject::tr("Недостаточно памяти для работы приложения");
    result = EXIT_FAILURE;
  }
  return result;
}
