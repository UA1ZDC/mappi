#include "postcontrol.h"


#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>
#include <commons/proc_read/daemoncontrol.h>

#include <meteo/commons/global/global.h>
#include <meteo/commons/settings/settings.h>
#include <meteo/commons/services/state/rpctimer.h>

#include <qcoreapplication.h>
#include <qtextcodec.h>

#include <unistd.h>


void printHelp(const QString& progName)
{
  info_log << QObject::tr("\nИспользование: %1 [option]\n").arg(progName)
	   << QObject::tr("Опции:\n")
	   << QObject::tr("-s 'yyyy-MM-dd' - начало интервала\n")
	   << QObject::tr("-e 'yyyy-MM-dd' - конец интервала")
    	   << QObject::tr("");
}


int main(int argc, char** argv)
{
  TAPPLICATION_NAME("meteo");
  meteo::global::setLogHandler();
  ::meteo::gSettings(meteo::global::Settings::instance());

  QCoreApplication app(argc, argv);

  if ( !::meteo::gSettings()->load() ) {
    error_log << QObject::tr("Не удалось загрузить настройки.");
    return -1;
  }
  
  QDateTime dtEnd = QDateTime::currentDateTimeUtc();
  dtEnd.setTime(QTime(dtEnd.time().hour(), dtEnd.time().minute(), 0));
  QDateTime dtStart = dtEnd.addSecs(-6*60);

  int opt;

  while ((opt = getopt(argc, argv, "hs:e:")) != -1) {
    switch (opt) {
    case 's':
      dtStart = QDateTime::fromString(optarg, "yyyy-MM-ddThh:mm");
      break;
    case 'e':
      dtEnd = QDateTime::fromString(optarg, "yyyy-MM-ddThh:mm");
      break;
    case 'h':
      printHelp(argv[0]);
      exit(0);
      break;
    default: /* '?' */
      exit(-1);
    }
  }

  setlocale(LC_NUMERIC, "C"); // локаль для записи float в mongo

  meteo::PostControl control;
  
  try {
    control.timeControl(dtStart, dtEnd);
  }
  catch (const std::bad_alloc &) {
    critical_log << QObject::tr("Недостаточно памяти для функционирования приложения");
    return EXIT_FAILURE; 
  }
  
  return 0;
}
