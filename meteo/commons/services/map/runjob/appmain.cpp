#include <stdlib.h>

#include <qapplication.h>

#include <cross-commons/app/paths.h>
#include <cross-commons/debug/tlog.h>
#include <meteo/commons/settings/settings.h>
#include <meteo/commons/settings/tusersettings.h>
#include <meteo/commons/global/global.h>

#include "stdinreader.h"



/*! 
 * \brief Программа для выполнения заданий по формированию документов
 * Выполнение одного задания выполняется в отдельном процессе, для того, чтобы распределить нагрузку
 * между ядрами процессора
 * Класс StdinReader получает задание от родительского процесса через stdtin. Выполнив задание он 
 * отправляеи результат выполнения родительскому процессу через stdout
 */
int appMain( int argc, char* argv[] )
{
  TAPPLICATION_NAME("meteo");
  QApplication app( argc, argv );

  gSettings(meteo::global::Settings::instance());
  if ( false == meteo::global::Settings::instance()->load() ) {
    error_log << QObject::tr("Не удалось загрузить настройки.");
    return EXIT_FAILURE;
  }

  meteo::TUserSettings::instance()->load();

  meteo::global::setLogHandler();

  meteo::map::StdinReader stdinreader;
  if ( 1 < argc && QString("test") == argv[1] ) {
    stdinreader.runTestJob();
  }
  stdinreader.connectStdinout();
  app.exec();
  return EXIT_SUCCESS;
}
