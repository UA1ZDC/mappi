#include <stdlib.h>

#include <qstringlist.h>
#include <qtextcodec.h>
#include "qthread.h"
#include "qobject.h"

#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>

#include <commons/proc_read/daemoncontrol.h>

#include <meteo/commons/rpc/rpc.h>

#include <mappi/settings/mappisettings.h>
#include <mappi/global/global.h>

#include "fileservice.h"

int main( int argc, char* argv[] )
{
  TAPPLICATION_NAME("meteo");

  try {
    //    ::meteo::gGlobalObj(new ::mappi::MappiGlobal);

    QCoreApplication* app = new QCoreApplication( argc, argv, false );

    TLog::setMsgHandler(TLog::clearStdOut);

    int res = ProcControl::singleInstanceSock("mappi.file.service.singleinstance");
    if ( 0 != res ) {
      QObject::tr("Не удалось указать, что приложение может быть запущено  только в единственном экземпляре");
      return EXIT_FAILURE;
    }

    meteo::gSettings(mappi::inter::Settings::instance());
  
    if ( false == meteo::gSettings()->load() ){
      error_log << meteo::msglog::kSettingsLoadFailed;
      return -1;
    }
    
    // if ( false == meteo::mappi::TMeteoSettings::instance()->load() ) {
    //   error_log << QObject::tr("Не удалось загрузить настройки.");
    //   return EXIT_FAILURE;
    // }

    mappi::FileService* service = new mappi::FileService;
    
    int msgPort = meteo::global::service( meteo::settings::proto::kFile ).port();
    meteo::rpc::Server* netserver = new meteo::rpc::Server(service, QString("0.0.0.0:%1").arg(msgPort));
    if ( nullptr == netserver ) {
      error_log << QObject::tr("Не удалось создать сервер scheduler");
      return EXIT_FAILURE;
    }
    netserver->init();
    info_log << QObject::tr("Сервис опубликован по адресу %1").arg(netserver->address());
    
    // debug_log << "ADDR =" << rpc::Address( "127.0.0.1", meteo::global::serviceAddress( meteo::settings::proto::kFile ).port() ).asString();
    // rpc::Server* netserver = rpc::Server::create(
    //   service,
    //   rpc::Address( "127.0.0.1", meteo::global::serviceAddress( meteo::settings::proto::kFile ).port() ) );
    // if ( 0 == netserver ) {
    //   error_log << QObject::tr("Не создан сервер связи с клиентами ");
    //   return EXIT_FAILURE;
    // }
    
    QObject::connect(netserver, &meteo::rpc::Server::clientConnected, service, &mappi::FileService::slotOnConnected);
    QObject::connect(netserver, &meteo::rpc::Server::clientDisconnected, service, &mappi::FileService::slotOnDisconnected);

    QThread* thread = new QThread;
    thread->moveToThread(thread);
    thread->start();

    info_log << QObject::tr("Файловый сервис запущен успешно");

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


