#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>
#include <sat-commons/satellite/satellitebase.h>
#include <mappi/global/global.h>
#include <mappi/device/receiver/receiver.h>
#include <qcoreapplication.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <iostream>

#define CONF_FILE (MnCommon::etcPath("mappi") + "reception.conf")

using namespace mappi;
using namespace receiver;


static struct option long_options[] = {
  { "satellite", required_argument, nullptr, 's' },
  { "list",      no_argument,       nullptr, 'l' },
  { "help",      no_argument,       nullptr, 'h' },
  { nullptr,     0,                 nullptr,  0  }
};


void usage(const QString& app)
{
  QStringList list;
  list.append(QObject::tr("использование: %1 [option]\nопции:").arg(app));
  list.append(QObject::tr("\t-s, --satellite = название спутника;"));
  list.append(QObject::tr("\t-l, --list перечень доступных dump files;"));
  list.append(QObject::tr("\t-h, --help показать справку и выйти.\n"));

  std::cout << list.join('\n').toStdString();
}


int main(int argc, char* argv[])
{
  QCoreApplication app(argc, argv);

  QString name;
  bool showUsage = false;
  bool showDumpList = false;

  int value = 0;
  while ((value = ::getopt_long(argc, argv, ":s:lh", long_options, 0)) != -1) {
    switch (value) {
      case 's' : name = ::optarg; break;
      case 'l' : showDumpList = true;  break;
      case 'h' : showUsage = true;     break;

      case ':' :
        error_log << QObject::tr("Не задан аргумент для -%1, выход").arg((char)::optopt);
        return EXIT_FAILURE;

      case '?' :
      default :
        info_log << QObject::tr("Неизвестный опция -%1, ошибка игнорируется").arg((char)::optopt);
        break;
    }
  };

  if (showUsage) {
    usage(app.applicationName());
    return EXIT_SUCCESS;
  }

  info_log << QObject::tr("Конфигурационный файл: %1").arg(CONF_FILE);
  Configuration conf;
  if (conf.load(CONF_FILE) == false) {
    error_log << QObject::tr("Не удалость загрузить конфигурационный файл");
    return EXIT_FAILURE;
  }

  // debug_log << conf.toString();
  // warning_log << QObject::tr("ВНИМАНИЕ: impl игнорируется");
  // conf.apch = false;
  // conf.isDemoMode = true;

  if (showDumpList) {
    QDir dir(QString("%1/stub").arg(conf.dataDir));
    std::cout << dir.entryList(QDir::NoDotDot | QDir::NoDot | QDir::Files).join('\n').toStdString() << std::endl;

    return EXIT_SUCCESS;
  }

  if (name.isEmpty()) {
    error_log << QObject::tr("Не задан спутник");
    return EXIT_FAILURE;
  }

  MnSat::STLEParams tleParams;
  bool isOk = SatelliteBase::getSTLE(name,
    meteo::global::findWeatherFile(MnCommon::varPath("mappi"), QDateTime::currentDateTimeUtc()),
    &tleParams
  );
  if (!isOk) {
    error_log << QObject::tr("Ошибка получения tle параметров спутника: %1").arg(name);
    return EXIT_FAILURE;
  }

  QString rawPath = QString("%1/%2").arg(conf.dataDir).arg(conf.rawFile);
  QFile file(rawPath);
  file.remove();
  if (file.open(QIODevice::WriteOnly) == false) {
    error_log << QObject::tr("Ошибка открытия файла: %1").arg(rawPath);
    return EXIT_FAILURE;
  }

  Satellite satellite;
  satellite.readTLE(tleParams);
  // qDebug() << satellite.name();

  Receiver receiver;
  if (receiver.tuned(conf) == false ||
      receiver.start(&satellite, conf.parameters(satellite.name())) == false) {

    error_log << QObject::tr("Аварийное завершение");
    return EXIT_FAILURE;
  }

  return app.exec();
}

