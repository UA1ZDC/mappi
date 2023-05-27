#include <obnclimatsaver.h>

#include <cross-commons/app/paths.h>
#include <cross-commons/debug/tlog.h>
#include <meteo/commons/rpc/rpc.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/settings/settings.h>

#include <sql/psql/psql.h>

#include <QCoreApplication>

#include <unistd.h>
#include <getopt.h>

//~/build_dir/bin/meteo.obnclimatsaver --start 1990-02-02 --end 1990-02-02 --station 26063 --host 10.10.11.76 --port 5433

void printHelp(const QString& progName)
{
  info_log << QObject::tr("\nИспользование: %1 [option]\n").arg(progName)
	   << QObject::tr("Опции:\n")
	   << QObject::tr("--start 'yyyy-MM-dd' - начало интервала\n")
	   << QObject::tr("--end 'yyyy-MM-dd' - конец интервала\n")
	   << QObject::tr("--station <станция> - станция\n")
	   << QObject::tr("--host  <host>  - хост с БД Обнинска (обязательный)\n")
	   << QObject::tr("--port <порт>   - порт БД (обязательный)\n");
}

int main(int argc, char* argv[])
{
  TAPPLICATION_NAME("meteo");
  meteo::global::setLogHandler();
  ::meteo::gSettings(meteo::global::Settings::instance());
  
  QCoreApplication app(argc, argv);

  if ( !::meteo::gSettings()->load() ) {
    error_log << QObject::trUtf8("Не удалось загрузить настройки.");
    return -1;
  }  

  QDate dtStart;
  QDate dtEnd;
  QString station;
  QString host;
  uint port;
  
  int opt;
  //  while ((opt = getopt(argc, argv, "hs:e:h:p:")) != -1) {
  while(true) {
    int option_index = 0;
    static struct option long_options[] = {
      {"start",   required_argument, 0, 's'},
      {"end",     required_argument, 0, 'e'},
      {"station",     required_argument, 0, 'n'},
      {"host",    required_argument, 0, 'i'},
      {"port",    required_argument, 0, 'p'},
      {"help",    no_argument, 0, 'h' },
      {0,         0,           0,  0 }
    };
    opt = getopt_long(argc, argv, "hd:p:r", long_options, &option_index);
    if (opt == -1) {
      break;
    }
    switch (opt) {
    case 's':
      dtStart = QDate::fromString(optarg, "yyyy-MM-dd");
      break;
    case 'e':
      dtEnd = QDate::fromString(optarg, "yyyy-MM-dd");
      break;
    case 'n':
      station = optarg;
      break;      
    case 'i':
      host = optarg;
      break;
    case 'p':
      port = QString(optarg).toUInt();
      break;      
    case 'h':
      printHelp(argv[0]);
      exit(0);
      break;
    default: /* '?' */
      exit(-1);
    }
  }

  // var(dtStart.toString());
  // var(dtEnd.toString());
  setlocale(LC_NUMERIC, "C"); // локаль для записи float в mongo
  
  if (host.isEmpty() || port <= 0) {
    debug_log << QObject::tr("Необходимо задать хост и порт с БД");
    printHelp(argv[0]);
    return -1;
  }
  

  ::meteo::rpc::Channel* channel = ::meteo::global::serviceChannel(meteo::settings::proto::kSrcData);
  if (nullptr == channel) {
    return -1;
  }


  //  "10.10.11.76", 5433,
  std::shared_ptr<meteo::Psql> db = std::make_shared<meteo::Psql>( host, port,
								   "clidb_gis",
								   "postgres", "1");
  
  meteo::climat::ObnClimatSaver saver(dtStart, dtEnd, station, db);

  saver.updateClimatData(channel);

  delete channel;

  return 0;
}
