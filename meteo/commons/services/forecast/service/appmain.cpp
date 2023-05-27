#include <stdlib.h>

#include <iostream>

#include <qcoreapplication.h>
#include <qstringlist.h>
#include <qtextcodec.h>

#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>

#include <commons/proc_read/daemoncontrol.h>

#include <meteo/commons/rpc/rpc.h>

#include "tforecastservice.h"
#include <meteo/commons/settings/settings.h>
#include <meteo/commons/global/global.h>


int appMain( int argc, char* argv[] )
{  
  TAPPLICATION_NAME("meteo");
  meteo::global::setLogHandler();
  ::meteo::gSettings(meteo::global::Settings::instance());
  if ( false == meteo::global::Settings::instance()->load() ) {
    error_log << QObject::tr("Не удалось загрузить настройки.");
    return EXIT_FAILURE;
  }

  QCoreApplication* app = new QCoreApplication( argc, argv, false );
  ::setlocale(LC_NUMERIC, "C"); // локаль для записи координат в mongo (libbson)

  QString appName = app->applicationFilePath();
  if(ProcControl::isAlreadyRun(appName)) {
    std::cout <<QObject::tr("Процесс %1 уже запущен. Приложение может быть запущено только в единственном экземпляре ").arg(appName).toStdString()<<"\n";
    return EXIT_FAILURE;
  }

 /* int res = ProcControl::singleInstanceSock("novost.forecast.singleinstance");
  if ( 0 != res ) {
      QObject::tr("Не удалось указать, что приложение может быть запущено только в единственном экземпляре");
      return EXIT_FAILURE;
  }*/

  meteo::forecast::TForecastService * service = new (std::nothrow) meteo::forecast::TForecastService();
  meteo::rpc::Server netserver(service, meteo::global::serviceAddress(meteo::settings::proto::kForecastData));

  if (false ==  netserver.init() || false == netserver.isListening() ) {
    error_log << QObject::tr("Не создан сервер связи с клиентами ");
    return EXIT_FAILURE;
  }

  debug_log << "Starting...";

  app->exec();

  return EXIT_SUCCESS;
}


