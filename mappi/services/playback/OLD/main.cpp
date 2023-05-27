#include "sessionmanager.h"
#include "virtualsession.h"

#include <commons/proc_read/daemoncontrol.h>

#include <mappi/global/global.h>
#include <mappi/settings/mappisettings.h>

#include <qcoreapplication.h>

#include <unistd.h>

void printHelp(const QString& progName)
{
  info_log << QObject::tr("\nИспользование: %1 [option]\n").arg(progName)
	   << QObject::tr("Опции:\n")
    	   << QObject::tr("-v\t\t Запуск в режиме виртуального времени\n")
	   << QObject::tr("-h\t\t Справка\n");
}


int main( int argc, char* argv[] ) 
{
  TAPPLICATION_NAME("meteo");

  QCoreApplication app(argc, argv);

  int opt;
  bool virt = false; 
  
  while ((opt = getopt(argc, argv, "hv")) != -1) {
    switch (opt) {
    case 'v':
      virt = true;
      break;
    case 'h':
      printHelp(argv[0]);
      exit(0);
      break;
    default: /* '?' */
      exit(-1);
    }
  }

  // ::meteo::gGlobalObj(new ::mappi::MappiGlobal);
  // if( !::meteo::mappi::TMeteoSettings::instance()->load() ){
  //   return EXIT_FAILURE;
  // }

  meteo::gSettings(mappi::inter::Settings::instance());
  
  if ( false == meteo::gSettings()->load() ){
    error_log << meteo::msglog::kSettingsLoadFailed;
    return EXIT_FAILURE;
  }

  
  
  if ( 0 != ProcControl::daemonizeSock("mappi.receive.manager", false)) {
    error_log << QObject::tr("Такой процесс уже запущен");
    return EXIT_FAILURE;
  }
  info_log << QObject::tr("Демон %1 запущен в режиме %2 времени").arg(argv[0]).arg(virt ? "виртуального" : "реального");
   

  mappi::receive::SessionManager* sManager = nullptr;

  if (virt) {
    sManager = new mappi::receive::VirtualSession();
  } else {
    sManager = new mappi::receive::SessionManager();
  }
  
  if(nullptr == sManager) {
    error_log << QObject::tr("Ошибка выделения памяти");
    return EXIT_FAILURE;
  }
  if(false == sManager->init()) {
    error_log << QObject::tr("Не удалось инициализировать сервис приёма");
    return EXIT_FAILURE;
  }
  
  int msgPort = meteo::global::service( meteo::settings::proto::kRecvStream ).port();
  meteo::rpc::Server* srv = new meteo::rpc::Server(sManager, QString("0.0.0.0:%1").arg(msgPort));
  if ( nullptr == srv ) {
    error_log << QObject::tr("Не удалось создать сервер msgcenter");
    return EXIT_FAILURE;
  }
  srv->init();
  info_log << QObject::tr("Сервис опубликован по адресу %1").arg(srv->address());
  
  QObject::connect(srv, &meteo::rpc::Server::clientSubscribed,   sManager, &mappi::receive::SessionManager::slotClientSubscribed);
  QObject::connect(srv, &meteo::rpc::Server::clientUnsubscribed, sManager, &mappi::receive::SessionManager::slotClientUnsubscribed);
  
  app.exec();
  
  delete srv;
  delete sManager;

  return 0;
}


/*
 //! \brief Запуск предварительной обработки
 void startPredObr( SpSctrl::SessionCtrl* sc ) {
   int pid = ::vfork();
   switch ( pid ) {
     case 0 : {
 //      SpFileCtrl fctrl;
       SpSctrl::DeviceService* dService = sc->getDeviceService();
       fctrl->setArg( FsCtrl::SAT_NAME, dService->getCodeSatName() );
       fctrl->setArg( FsCtrl::FILE_TYPE, SpThematic::RAW_EXT );
       fctrl->setArg( FsCtrl::STREAM_TYPE, dService->getStrStreamType() );

       SpSctrl::SessionService* sService = sc->getSessionService();
       SpSchedule::Session* session = sService->getSession();
       QString filePath = fctrl->getFileName( FsCtrl::HRPTDIGIT_PATH, session->getAos() );
       debug( "[ ::startPredObr(1) ] - %s;", (const char*)filePath );

       ::execl( SPPRETRD_NAME, SPPRETRD_NAME, (const char*)filePath, "-t", NULL );
       ::_exit( 0 );
     };
       break;
     case -1 :
       debug( "Ошибка запуска предварительной обработки" );
       break;
     default :
       //      ::waitpid( pid, 0, WUNTRACED );
       break;
   };
 }
*/
