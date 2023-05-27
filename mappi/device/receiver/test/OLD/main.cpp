// #include <mappi/device/receiver/gnuradio.h>
//#include <mappi/global/global.h>
//#include <cross-commons/app/paths.h>
//#include <cross-commons/debug/tlog.h>
// #include <qapplication.h>


#define CTRL_APP MnCommon::binPath("mappi") + "/mappi_rxwrapper.py"
// #define CTRL_APP MnCommon::binPath("mappi") + "/mappi_rxstub.py"
// #define MAPPIWEATHER_FILE MnCommon::varPath("mappi") + "/weather.txt"


void usage(const QString& app)
{
  info_log << QObject::tr("\nИспользование: %1 [option]\n").arg(app)
           << QObject::tr("Опции:\n")
           << QObject::tr("-f <freq>\t Частота, MГц [83-97, по умолчанию 94.0237]\n")
           << QObject::tr("-g <gain>\t Усиление [14-40, по умолчанию 24.1]\n")
           << QObject::tr("-r <rate>\t Частота дискретизации, МГц [по умолчанию 5]\n")
           << QObject::tr("-n <name>\t Название спутника\n")
           << QObject::tr("-a <app> \t Полный путь к скрипту для настройки [%1]\n").arg(CTRL_APP)
           << QObject::tr("-c       \t Подстройка частоты\n")
           // << QObject::tr("-d\t\t Запустить как демон\n")
           << QObject::tr("-h\t\t Справка\n");
}


int main(int argc, char* argv[])
{
  TAPPLICATION_NAME("meteo");

  QString appName(CTRL_APP);

  // создать файл raw

  mappi::receiver::Configuration conf;
  conf.rootPath = ;
  conf.apch = true;
  conf.mode = ;
  conf.rawFile = ;

  mappi::receiver::Configuration::param_t param;
  param.freq = 94.0237;
  param.gain = 24.1;
  param.rate = 5;


{
public :
  struct param_t {
    float freq;
    float gain;
    float rate;
  };
/*
  bool isDemoMode;
  bool apch;
  conf::RateMode mode;
  QString rawFile;
  conf::TransportImpl impl;
  QHostAddress host;
  quint16 port;
  QString script;

  QString rootPath;
  Coords::GeoCoord site;
*/

  mappi::receiver::GnuRadio::param_t param;
  param.site = Coords::GeoCoord(1.04614, 0.52857, 6);

  param.freq = 94.0237;
  param.gain = 24.1;
  param.rate = 5;

  param.adjust_freq = false;

  QString val;
  QString name;

  int opt;
  while ((opt = getopt(argc, argv, "hf:g:r:a:n:c")) != -1) {
    switch (opt) {
      case 'f' : val = optarg; param.freq = val.toFloat(); break;
      case 'g' : val = optarg; param.gain = val.toFloat(); break;
      case 'r' : val = optarg; param.rate = val.toFloat(); break;
      case 'c' : param.adjust_freq = true; break;
      case 'a' : appName = optarg; break;
      case 'n' : name = optarg; break;

      case 'h' :
        usage(argv[0]);
        exit(0);
      break;

      default : /* '?' */
        exit(-1);
    }
  }

  if (param.gain <= 0 || param.rate == 0 || appName.isEmpty() || name.isEmpty()) {
    error_log << QObject::tr("Не верно заданы параметры");
    var(param.freq);
    var(param.gain);
    var(param.rate);
    var(appName);
    var(name);

    usage(argv[0]);
    exit(-1);
  }

  var(appName);

  param.freq *= 1000000;
  param.rate *= 1000000;

  QString tleFile = meteo::global::findWeatherFile(MnCommon::varPath("mappi"), QDateTime::currentDateTimeUtc());

  MnSat::STLEParams tle;
  bool ok = SatelliteBase::getSTLE(name, tleFile, &tle);
  var(ok);

  Satellite satellite;
  satellite.readTLE(tle);

  QApplication app(argc, argv);

  mappi::receiver::GnuRadio radio;
  if (radio.start(appName, &satellite, param))
    return app.exec();

  return -1;
}
