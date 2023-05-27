#include <stdlib.h>

#include <qapplication.h>
#include <qtextcodec.h>
#include <qobject.h>

#include <cross-commons/app/paths.h>
#include "../tappconf.h"
#include <meteo/commons/appmanager/tappcontrol.h>
#include <cross-commons/debug/tlog.h>
#include <meteo/commons/proto/appconf.pb.h>

int testLoadSave()
{
  meteo::app::AppConfig appconf;
  if ( false == meteo::app::Conf::load(&appconf) ) {
    error_log << QObject::tr("Ошибка загрузки конфигурации");
    return EXIT_FAILURE;
  }
  debug_log
    << QObject::tr("Содержимое файла конфигурации =\n")
    << appconf.DebugString().c_str();
  return EXIT_SUCCESS;
}

void testControl()
{
  meteo::app::Control* ctrl = new meteo::app::Control;
  if ( false == ctrl->loadConf() ) {
    return;
  }
  ctrl->startProcs();
}

int main( int argc, char* argv[] )
{
  TAPPLICATION_NAME("meteo");
  QApplication app( argc, argv );
  QTextCodec::setCodecForCStrings( QTextCodec::codecForLocale() );

  testLoadSave();
  testControl();
  app.exec();


  return EXIT_SUCCESS;
}
