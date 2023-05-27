#include <stdlib.h>

#include <qcoreapplication.h>

#include <meteo/commons/global/global.h>
#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>
#include <commons/proc_read/daemoncontrol.h>

#include "restore_aero.h"

#define APP_NAME "restore.aero"

void printHelp(const QString& progName)
{
	info_log << QObject::tr("\nИспользование: %1 [option]\n").arg(progName)
					 << QObject::tr("Опции:\n")
					 << QObject::tr("-t\t\t Дата/время в ISO [по умолчанию последний шестичасовой срок]\n")
					 << QObject::tr("-h\t\t Справка\n");
}



int main( int argc, char* argv[] )
{ 
  try {
    TAPPLICATION_NAME("meteo");
    meteo::global::setLogHandler();
    //  ::meteo::gSettings(meteo::global::Settings::instance());
    //    meteo::global::Settings::instance()->load();
    
    QCoreApplication app(argc, argv);

    int opt;
    QDateTime dt;
    while ((opt = getopt(argc, argv, "ht:f")) != -1) {
      switch (opt) {
        case 'h':
          printHelp(argv[0]);
          exit(0);
        break;
        case 't':
          dt = QDateTime::fromString(optarg, Qt::ISODate);
        break;
        default: /* '?' */
          exit(-1);
      }
    }
    if (!dt.isValid()) {
      dt = QDateTime::currentDateTimeUtc();
      int hour = int(dt.time().hour() / 6) * 6;
      dt.setTime(QTime(hour, 0, 0));
    }

  }
  catch (const std::bad_alloc & ) {
    critical_log << QObject::tr("Недостаточно памяти для функционирования приложения");
    return EXIT_FAILURE;
  }
}
