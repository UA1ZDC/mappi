#include <cross-commons/app/paths.h>
#include <cross-commons/debug/tlog.h>
#include <cross-commons/singleton/tsingleton.h>
#include <commons/textproto/tprototext.h>
#include <commons/textproto/pbtools.h>

#include <mappi/pretreatment/formats/satformat.h>
#include <mappi/pretreatment/handler/handler.h>
#include <mappi/proto/satellite.pb.h>
#include <mappi/proto/reception.pb.h>
#include <mappi/global/global.h>
#include <meteo/commons/global/global.h>
#include <mappi/pretreatment/savenotify/savenotify.h>

#include <unistd.h>
#include <getopt.h>
#include <qdatetime.h>
#include <qcoreapplication.h>
#include <QFile>
#include <QFileInfo>

using namespace mappi::po;

int main(int argc, char **argv) {
  TAPPLICATION_NAME("meteo");
  meteo::gSettings(mappi::inter::Settings::instance());

  if (false == meteo::gSettings()->load()) {
    error_log << meteo::msglog::kSettingsLoadFailed;
    return -1;
  }

  QCoreApplication app(argc, argv, false);

  PretrOpt opt = {0, 0, 0, mappi::conf::kNoSwap, mappi::conf::kUnkRate, false, false, false, false, false, "", "Санкт-Петербург", "", "{}"};

  static struct option long_options[] = {
      {"help", 0, 0, 'h'},
      {0,      0, 0, 0}
  };

  int option_index = 0;
  QString weatherFileName = mappi::po::singleton::SatFormat::instance()->getWeatherFilePath();
  QString path = MnCommon::varPath();

  QString optStr = QString("y:m:d:qeocifs:glw:p:h");

  while (1) {
    int c = getopt_long(argc, argv, optStr.toUtf8(), long_options, &option_index);
    if (c == -1) break;
    switch (c) {
      case 'y':
        sscanf(optarg, "%d", &opt.year);
        if (opt.year < 100) {
          opt.year += 2000;
        }
        break;
      case 'm':
        sscanf(optarg, "%d", &opt.month);
        break;
      case 'd':
        sscanf(optarg, "%d", &opt.day);
        break;
      case 'q':
        opt.swap = mappi::conf::kSwap;
        break;
      case 'e':
        opt.headerExist = true;
        break;
      case 'o':
        opt.oldHeaderExist = true;
        break;
      case 'c':
        opt.manchester = true;
        break;
      case 'f':
        opt.deframer = true;
        break;
      case 'i':
        opt.invert = true;
        break;
      case 'g':
      case 'l': {
        if (opt.mode != mappi::conf::kUnkRate) {
          warning_log << QObject::tr("Предупреждение! Задано одновременно несколько режимов приёма. "
                                     "Будет использован последний.");
        }
        if (c == 'g') {
          opt.mode = mappi::conf::kHiRate;
        } else {
          opt.mode = mappi::conf::kLowRate;
        }
      }
        break;
      case 's':
        opt.satName = optarg;
        break;
      case 'w':
        weatherFileName = optarg;
        break;
      case 'p':
        opt.pipelineName = optarg;
        break;
      case 'h':
      case '?':
        info_log << QObject::tr("Использование:\n"
                                "\t%1 -s <спутник> [-p <pipeline>] [-q] [-l|-g] [-e|-o] [-r] [-i] [-y <год>][-m <месяц>][-d <день>] [-w <TLE>] файл\n"
                                "Опции:\n"
                                "\t-q\t Поменять четные и нечетные байты данных местами\n"
                                "\t-e\t считать параметры из заголовка\n"
                                "\t-o\t считать параметры из заголовка старого (виндового) формата\n"
                                "\t-c\t поток закодированный манчестером\n"
                                "\t-f\t поток без разделения по кадрам\n"
                                "\t-i\t биты необходимо инвертировать\n"
                                "\t-s\t название спутника\n"
                                "\t-p\t название пайплайна satdump (берется из конфига)\n"
                                "\t-y\t год приёма данных, по умолчанию текущий\n"
                                "\t-m\t месяц приёма данных, по умолчанию текущий (для NOAA не надо)\n"
                                "\t-d\t день приёма данных, по умолчанию текущий (для NOAA не надо)\n"
                                "\t-g\t High rate\n"
                                "\t-l\t Low rate\n"
                                "\t-w\t Полный путь к TLE файлу (weather.txt)\n"
                                "\tфайл\t полное имя файла потока\n"
                                "\t-h\t эта справка\n").arg(argv[0]);
        return 0;
    }
  }

  if (optind >= argc) {
    info_log << QObject::tr("В качестве параметров надо задать полное имя raw-файла. (\"-h\" для помощи)");
    return -1;
  }

  if(opt.pipelineName == "") opt.pipelineName = singleton::SatFormat::instance()->getPipelineFor(opt.satName);
  if(opt.pipelineName == "") {
    error_log << QObject::tr("Не существует пайплайна для спутника %1").arg(opt.satName);
    return -1;
  }

  QString fileName = argv[optind];
  mappi::po::Handler handler;
  handler.parseStream(fileName, weatherFileName, path, opt);
  mappi::SaveNotify* notify = mappi::CreateNotify::createServiceNotify(mappi::CreateNotify::StubServiceNotify);
  handler.setDeleteInput(false);
  if (!handler.process(notify)) {
    error_log << QObject::tr("Ошибка обработки данных");
  }
  delete notify;

  return 0;
}
