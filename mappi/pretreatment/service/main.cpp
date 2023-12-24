#include "pretrservice.h"

#include <mappi/global/global.h>

#include <mappi/global/global.h>
#include <mappi/settings/mappisettings.h>
#include <commons/proc_read/daemoncontrol.h>

#include <qcoreapplication.h>

#include <getopt.h>

void printHelp(const QString& progName)
{
  info_log << QObject::tr("\nИспользование: %1 [option]\n").arg(progName)
	   << QObject::tr("Опции:\n")
	   << QObject::tr("-f <type>\t Тип заголовка файла [default = 3 (Handler::kSeparateHeader)]\n")
	   << QObject::tr("-n <type>\t Тип уведомлений [default = 1 (CreateNotify::ServiceNotify)] \n")
	   << QObject::tr("-h\t\t Справка\n");
}


int main (int argc, char** argv)
{
  try {

    TAPPLICATION_NAME("meteo");
    
    QCoreApplication app(argc, argv);
    
    // ::meteo::gGlobalObj(new ::mappi::MappiGlobal);
    // if( !::meteo::mappi::TMeteoSettings::instance()->load() ){
    //   return EXIT_FAILURE;
    // }
    
    meteo::gSettings(mappi::inter::Settings::instance());
    
    if ( false == meteo::gSettings()->load() ) {
      error_log << meteo::msglog::kSettingsLoadFailed;
      return EXIT_FAILURE;
    }

    QString appName = app.applicationFilePath();
    if ( ProcControl::isAlreadyRun(appName) ) {
      error_log << QObject::tr("Процесс %1 уже запущен.").arg(appName);
      return EXIT_FAILURE;
    }

    int htype = mappi::po::Handler::kSeparateHeader;
    int ntype = mappi::CreateNotify::ServiceNotify;
    int opt;

    static struct option long_options[] = {
      {"help", 0, 0, 'h'},
      {0,0,0,0}
    }; 
    
    int option_index = 0;
    
    while ((opt = getopt_long(argc, argv, "hf:n:", long_options, &option_index)) != -1) {
      
      switch (opt) {
      case 'f':
	sscanf(optarg, "%d", &htype);
	break;
      case 'n':
	sscanf(optarg, "%d", &ntype);
	break;
      case 'h':
	printHelp(argv[0]);
	exit(0);
	break;
      default: /* '?' */
	exit(-1);
      }
    }
      
    if (!mappi::CreateNotify::isValidType(ntype)) {
      error_log << QObject::tr("Задан некорректный тип уведомлений");
      return EXIT_FAILURE;
    }
    
    mappi::po::PretrService service;
    service.init(mappi::po::Handler::HeaderType(htype), mappi::CreateNotify::Type(ntype));
    
    meteo::rpc::Server* server = new meteo::rpc::Server(&service, meteo::global::serviceAddress(meteo::settings::proto::kPretreatment));
    if (false == server->init()) {
      error_log << QObject::tr("Не создан сервер связи с клиентами");
      return EXIT_FAILURE;
    }
    // QObject::connect( &server, &meteo::rpc::Server::ansverSendedError,
    //                   &service,&mappi::ThematicService::slotAnsverSendedError );
    // QObject::connect( &server, &meteo::rpc::Server::ansverSendedOK,
    //                   &service,&mappi::ThematicService::slotAnsverSendedOK );
    info_log << QObject::tr("%1 запущена").arg(meteo::global::serviceTitle(meteo::settings::proto::kPretreatment));
    
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
