#include <stdlib.h>

#include <qstringlist.h>
#include <qtextcodec.h>
#include <QThread>

#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>

#include <commons/proc_read/daemoncontrol.h>

#include <meteo/commons/rpc/rpc.h>

#include <mappi/settings/mappisettings.h>
#include <mappi/global/global.h>

#include "sessiondataservice.h"


int main( int argc, char* argv[] )
{
  TAPPLICATION_NAME("meteo");

  try {
    //::meteo::gGlobalObj(new ::mappi::MappiGlobal);

    QCoreApplication* app = new QCoreApplication( argc, argv, false );

    //TLog::setMsgHandler( meteo::global::stdOutWithTime );

    int res = ProcControl::singleInstanceSock("mappi.data.service.singleinstance");
    if ( 0 != res ) {
      QObject::tr("Не удалось указать, что приложение может быть запущено  только в единственном экземпляре");
      return EXIT_FAILURE;
    }

    // if ( false == meteo::mappi::TMeteoSettings::instance()->load() ) {
    //   error_log << QObject::tr("Не удалось загрузить настройки.");
    //   return EXIT_FAILURE;      
    // }

    meteo::gSettings(mappi::inter::Settings::instance());
    
    if ( false == meteo::gSettings()->load() ){
      error_log << meteo::msglog::kSettingsLoadFailed;
      return EXIT_FAILURE;
    }

    mappi::SessionDataService* service = new mappi::SessionDataService;

    int msgPort = meteo::global::service( meteo::settings::proto::kDataControl ).port();
    meteo::rpc::Server* netserver = new meteo::rpc::Server(service, QString("0.0.0.0:%1").arg(msgPort));
    if ( nullptr == netserver ) {
      error_log << QObject::tr("Не удалось создать сервер msgcenter");
      return EXIT_FAILURE;
    }
    netserver->init();
    info_log << QObject::tr("Сервис опубликован по адресу %1").arg(netserver->address());
    
    // debug_log << "ADDR =" << rpc::Address( "127.0.0.1", meteo::global::serviceAddress( meteo::settings::proto::kDataControl ).port() ).asString();
    // rpc::Server* netserver = rpc::Server::create(
    //   service,
    //   rpc::Address( "127.0.0.1", meteo::global::serviceAddress( meteo::settings::proto::kDataControl ).port() ) );
    // if ( 0 == netserver ) {
    //   error_log << QObject::tr("Не создан сервер связи с клиентами ");
    //   return EXIT_FAILURE;
    // }
    // QObject::connect( netserver, SIGNAL( clientConnected( rpc::Channel* ) ),
    //     service->handler(), SLOT( slotClientConnected( rpc::Channel* ) ) );

//    meteo::map::WeatherMapThread* thread = new meteo::map::WeatherMapThread;
    QThread* thread = new QThread;
    thread->moveToThread(thread);
    thread->start();

    info_log << QObject::tr("Сервис данных запущен успешно");

    app->exec();
    thread->quit();
    delete thread; thread = 0;
    delete service; service = 0;
  }
  catch(const std::bad_alloc& ) {
    critical_log << QObject::tr("Недостаточно памяти для работы программы");
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}


