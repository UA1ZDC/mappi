#include <meteo/commons/global/global.h>
#include <meteo/commons/settings/settings.h>
#include <meteo/commons/proto/obanal.pb.h>
#include <meteo/commons/services/obanal_tfp/obanaltfp.h>

#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>
#include <commons/proc_read/daemoncontrol.h>

#include <qcoreapplication.h>

#include <stdlib.h>

#define APP_NAME "obanal.indexes"

void printHelp(const QString& progName)
{
  info_log << QObject::tr("\nИспользование: %1 [option]\n").arg(progName)
	   << QObject::tr("Опции:\n")
    	   << QObject::tr("-t\t\t Дата/время в ISO [по умолчанию последний шестичасовой срок]\n")
	   << QObject::tr("-h\t\t Справка\n");
}


void processField(const QDateTime& dt)
{
  var(dt);
  meteo::ObanalTfp obanal;
  if (dt.isValid()) {
    obanal.runField(dt);
  } // else {
  //   QDateTime cur = QDateTime::currentDateTimeUtc();
  //   int hour = (int)trunc(cur.time().hour() / 6) * 6;
  //   cur.setTime(QTime(hour, 0, 0));
  //   obanal.runField(cur);
  //   for (int idx = 1; idx >= 0; idx--) { //считаем на два срока от последнего
  //     obanal.runField(cur.addSecs(-6*3600*idx));
  //   }
  // }
}

int main( int argc, char* argv[] )
{ 
  try {
    TAPPLICATION_NAME("meteo");
    meteo::global::setLogHandler();
  ::meteo::gSettings(meteo::global::Settings::instance());
    meteo::global::Settings::instance()->load();
    
    QCoreApplication app(argc, argv);

    int opt;
    QDateTime dt;
    //    int flag = 0;

  while ((opt = getopt(argc, argv, "ht:f")) != -1) {
    switch (opt) {
    case 'h':
      printHelp(argv[0]);
      exit(0);
      break;
    case 't':
      dt = QDateTime::fromString(optarg, Qt::ISODate);
      break;
    case 'f':
      //flag = 1;
      break;      
    default: /* '?' */
      exit(-1);
    }
  }
    
    // if (0 != ProcControl::daemonizeSock(APP_NAME, false)) {
    //   error_log << QObject::tr("Такой процесс уже запущен");
    //   return -1;
    // }
    
    // ::setlocale(LC_NUMERIC, "C"); // локаль для записи вещественных чисел в mongo (libbson)

  if (!dt.isValid()) {
    dt = QDateTime::currentDateTimeUtc();
    //    int hour = fmodf(dt.time().hour(), 6) * 6;
    int hour = int(dt.time().hour() / 6) * 6;
    // if (hour < 12) {
    //   hour = 0;
    // } else {
    //   hour = 12;
    // }
    dt.setTime(QTime(hour, 0, 0));
  }
  
  //  if (flag == 1) {
      processField(dt);
      // }
    
  }
  catch (const std::bad_alloc & ) {
    critical_log << QObject::tr("Недостаточно памяти для функционирования приложения");
    return EXIT_FAILURE;
  }
}
