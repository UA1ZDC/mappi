#include <stdlib.h>

#include <iostream>
#include <memory>

#include <qcoreapplication.h>
#include <qstringlist.h>
#include <qtextcodec.h>

#include <cross-commons/app/paths.h>

#include <commons/proc_read/daemoncontrol.h>

#include <meteo/commons/rpc/rpc.h>
#include <meteo/commons/settings/settings.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/services/fieldata/fieldanalyse.h>

int appMain(int argc, char* argv[])
{
  try {
    TAPPLICATION_NAME("meteo");
    meteo::global::setLogHandler();
    meteo::gSettings(meteo::global::Settings::instance());
    if ( false == meteo::global::Settings::instance()->load() ) {
      error_log << QObject::tr("Не удалось загрузить настройки.");
      return EXIT_FAILURE;
    }
    auto app = std::unique_ptr<QCoreApplication>(new QCoreApplication(argc, argv));
    ::meteo::AppStatusThread* status = new ::meteo::AppStatusThread;

    meteo::field::TFieldAnalyse* service = new meteo::field::TFieldAnalyse(status);
    meteo::rpc::Server server(service, meteo::global::serviceAddress(meteo::settings::proto::kField));
    if ( false == server.init() ) {
      error_log << QObject::tr("Не создан сервер связи с клиентами");
      return EXIT_FAILURE;
    }
    QObject::connect( &server,&meteo::rpc::Server::ansverSendedError,
                      service,&meteo::field::TFieldDataService::slotAnsverSendedError );
    QObject::connect( &server,&meteo::rpc::Server::ansverSendedOK,
                      service,&meteo::field::TFieldDataService::slotAnsverSendedOK );
    status->setUpdateLimit(1000);
    status->setSendLimit(5000);
    status->start();
    info_log << QObject::tr("%1 запущена").arg(meteo::global::serviceTitle(meteo::settings::proto::kField));
    app->exec();
  }
  catch (const std::bad_alloc &) {
    critical_log << QObject::tr("Недостаточно памяти для функционирования приложения");
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
