#include <meteo/commons/services/esimo_nc/esimonc.h>

#include <cross-commons/app/paths.h>
#include <cross-commons/debug/tlog.h>
#include <commons/proc_read/daemoncontrol.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/settings/settings.h>

#include <QCoreApplication>

#include <unistd.h>
#include <getopt.h>

#define APP_NAME "pak.esimo.service"

void printHelp(const QString& progName)
{
  info_log << QObject::tr("\nИспользование: %1 [option]\n").arg(progName)
	   << QObject::tr("Опции:\n")
	   << QObject::tr("-d          \t\t Запустить как демон\n")
	   << QObject::tr("-p, --path <path>\t Путь к проверяемой директории\n")
	   << QObject::tr("-r, --remove\t\t Удалять обработанные поддиректории\n")
	   << QObject::tr("-h, --help\t\t Справка\n")
	   << QObject::tr("");
}

int main(int argc, char* argv[])
{
  TAPPLICATION_NAME("meteo");
  meteo::global::setLogHandler();
  ::meteo::gSettings(meteo::global::Settings::instance());
  
  QCoreApplication app(argc, argv);

  if ( !::meteo::gSettings()->load() ) {
    error_log << QObject::tr("Не удалось загрузить настройки.");
    return -1;
  }  

  QString path;
  bool rmdir = false;
  bool isdaemon = false;
  
  int opt;
  while (1) {
    int option_index = 0;
    //  while ((opt = getopt(argc, argv, "hd:p:r")) != -1) {
    static struct option long_options[] = {
					   {"path",    required_argument, 0,  'p' },
					   {"remove",  no_argument, 0, 'r'},
					   {"help",    no_argument, 0, 'h' },
					   {0,         0,           0,  0 }
    };
    opt = getopt_long(argc, argv, "hd:p:r", long_options, &option_index);
    if (opt == -1) {
      break;
    }
    switch (opt) {
    case 'd':
      isdaemon = true;
      break;
    case 'p':
      path = optarg;
      break;
    case 'r':
      rmdir = true;
      break;
    case 'h':
      printHelp(argv[0]);
      exit(0);
      break;
    default: /* '?' */
      exit(-1);
    }
  }

  
  if (path.isEmpty()) {
    error_log << QObject::tr("Необходимо указать путь к директории");
    return -1;
  }
  
  if (isdaemon) {
    if ( 0 != ProcControl::daemonizeSock(APP_NAME)) {
      error_log << QObject::tr("Такой процесс уже запущен");
      return -1;
    }
    info_log << QObject::tr("Демон %1 запущен").arg(argv[0]);
  } else {
    if (0 != ProcControl::daemonizeSock(APP_NAME, false)) {
      error_log << QObject::tr("Такой процесс уже запущен");
      return -1;
    }
  }

  
  //пока не настоящий сервис)
  meteo::EsimoNc esimo(path, rmdir);
  esimo.start();

  try {
    app.exec();
  }
  catch (const std::bad_alloc &) {
    critical_log << QObject::tr("Недостаточно памяти для функционирования приложения");
    return EXIT_FAILURE;
  }
  
  return 0;
}
