#include "thematicservice.h"

#include <mappi/settings/mappisettings.h>
#include <mappi/global/global.h>
#include <commons/proc_read/daemoncontrol.h>
#include <meteo/commons/rpc/rpc.h>


#include <QCoreApplication>

int main(int argc, char *argv[])
{
  QCoreApplication app(argc, argv);
  TAPPLICATION_NAME("meteo");
  
  // ::meteo::gGlobalObj(new ::mappi::MappiGlobal);
  // meteo::mappi::TMeteoSettings::instance()->load();

  meteo::gSettings(mappi::inter::Settings::instance());
  
  if ( false == meteo::gSettings()->load() ){
    error_log << meteo::msglog::kSettingsLoadFailed;
    return -1;
  }

  QString appName = app.applicationFilePath();
  if ( ProcControl::isAlreadyRun(appName) ) {
    error_log << QObject::tr("Процесс %1 уже запущен.").arg(appName);
    return EXIT_FAILURE;
  }
  
  try {      
    mappi::ThematicService service;
    
    meteo::rpc::Server* server = new meteo::rpc::Server(&service, meteo::global::serviceAddress(meteo::settings::proto::kThematic));
    if ( false == server->init() ) {
      error_log << QObject::tr("Не создан сервер связи с клиентами");
      return EXIT_FAILURE;
    }
    // QObject::connect( &server, &meteo::rpc::Server::ansverSendedError,
    //                   &service,&mappi::ThematicService::slotAnsverSendedError );
    // QObject::connect( &server, &meteo::rpc::Server::ansverSendedOK,
    //                   &service,&mappi::ThematicService::slotAnsverSendedOK );
    info_log << QObject::tr("%1 запущена").arg(meteo::global::serviceTitle(meteo::settings::proto::kThematic));
     
    app.exec();
    delete server;
    server = nullptr;
  }
  
  catch (const std::bad_alloc &) {
    critical_log << QObject::tr("Недостаточно памяти для функционирования приложения");
    return EXIT_FAILURE;
  }
  
  return 0;
}
