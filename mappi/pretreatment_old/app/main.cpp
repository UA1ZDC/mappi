#include <cross-commons/app/paths.h>
#include <cross-commons/debug/tlog.h>
#include <commons/textproto/tprototext.h>
#include <commons/textproto/pbtools.h>

#include <mappi/global/global.h>
#include <mappi/pretreatment/formats/satformat.h>
#include <mappi/pretreatment/handler/handler.h>
#include <mappi/proto/satellite.pb.h>
#include <mappi/proto/reception.pb.h>
#include <sat-commons/satellite/satviewpoint.h>
#include <mappi/pretreatment/savenotify/savenotify.h>

#include <unistd.h>
#include <getopt.h>
#include <qdatetime.h>
#include <qcoreapplication.h>

#define RECEPTION_FILE MnCommon::etcPath("mappi") + "reception.conf"

namespace TSatPretr {

//! Опции запуска предварительной обработки
struct PretrOpt {
  int year;  //!< Год приёма данных
  int month; //!< Месяц приёма данных
  int day;   //!< День приёма данных
  mappi::conf::RateMode mode;//!< Режим приёма
  bool headerExist; //!< Наличие доп. заголовка
  bool oldHeaderExist; //!< Наличие доп. заголовка старого формата
  bool manchester;     //!< Необходимо снятие манчестера
  bool deframer;       //!< Необходим побитовый поиск синхропоследовательности
  bool invert;	 //!< Инвертированные биты в потоке
  QString satName; 	 //!< Название спутника
};
}

using namespace TSatPretr;

/*! 
 * \brief Создание даты приёма из опций
 * \param opt  опции
 * \param date дата приёма
 * \return ERR_NOERR в случае успеха, иначе код ошибки
 */
bool constructFromOpt(PretrOpt& opt, QDate& date)
{
  if (opt.year == 0) { opt.year = date.year();  }
  if (opt.month == 0) { opt.month = date.month();  }
  if (opt.day == 0) { opt.day = date.day();  }
  
  if (QDate::isValid( opt.year, opt.month, opt.day )) {
    date.setDate(opt.year, opt.month, opt.day);
  }//  else if (opt.ctype != mappi::conf::kHrpt) {
  //   error_log << "Ошибка! Неверно задана дата.\n";
  //   return false;
  // }
  return true;
}

void parseStream(const QString& fileName, const QString& weatherFile, const QString& path, TSatPretr::PretrOpt& opt)
{
  SatViewPoint sat;
  sat.readTLE(opt.satName, weatherFile);

  mappi::po::Handler handler;
  handler.setTle(sat.getTLEParams());
  handler.setFile(fileName);
  handler.setPath(path);

  //  QString stream;
  if (opt.mode != mappi::conf::kUnkRate) {
    handler.setRecvMode(opt.mode);
  }

  handler.setSatName(opt.satName);

  if (opt.manchester) {
    handler.setDataLevel(opt.invert ? mappi::conf::kManchesterInvertLevel : mappi::conf::kManchesterLevel);
  } else if (opt.deframer) {
    handler.setDataLevel(opt.invert ? mappi::conf::kDeframerIvertLevel : mappi::conf::kDeframerLevel);
  } else {
    handler.setDataLevel(mappi::conf::kFrameLevel);
  }

  if (opt.headerExist) {
    handler.setHeaderType(mappi::po::Handler::kMappiHeader);
  } else if (opt.oldHeaderExist) {
    if (opt.deframer) { //с такими параметрами виндовый cif формат
      handler.setHeaderType(mappi::po::Handler::kPreOldHeader);
    } else {
      handler.setHeaderType(mappi::po::Handler::kPostOldHeader);
    }
  } else {
    QDateTime dt = QDateTime::currentDateTimeUtc();
    QDate date = dt.date();
    constructFromOpt(opt, date);
    if ( date.isValid() ) {
      dt.setDate(date);
    }
    
    handler.setDateTime(dt);
  }

  //NOTE здесь не может быть другого типа, т.к. не загружены глобальные настройкиxs
  mappi::SaveNotify* notify = mappi::CreateNotify::createEmptyNotify();
  
  if (!handler.process(notify)) {
    error_log << QObject::tr("Ошибка обработки данных");
  }

  delete notify;
}

void readConfig(QString* weatherFile, QString* path)
{
  QFile file(RECEPTION_FILE);
  if ( !file.open(QIODevice::ReadOnly) ) {
    error_log << QObject::tr("Ошибка при загрузке настроек '%1'").arg(RECEPTION_FILE);
    return;
  }

  QString text = QString::fromUtf8(file.readAll());
  file.close();

  mappi::conf::Reception conf;
  if ( !TProtoText::fillProto(text, &conf) ) {
    error_log << QObject::tr("Ошибка в структуре файла настроек '%1'").arg(RECEPTION_FILE);
    return;
  }

  *weatherFile = meteo::global::findWeatherFile(pbtools::toQString(conf.file_storage().tle()),
                                                QDateTime::currentDateTimeUtc());
  *path = pbtools::toQString(conf.file_storage().root());

}


int main(int argc, char** argv)
{
  TAPPLICATION_NAME("meteo");

  //QTextCodec::setCodecForCStrings( QTextCodec::codecForLocale());
  QCoreApplication app(argc, argv, false);

  PretrOpt opt = {0, 0, 0, mappi::conf::kUnkRate, false, false, false, false, false, ""};

  static struct option long_options[] = {
  {"help", 0, 0, 'h'},
  {0,0,0,0}
};

  int option_index = 0;
  QString weatherFileName;
  QString path;
  readConfig(&weatherFileName, &path);


  QString optStr = QString("y:m:d:eocifs:glw:h");

  while (1) {
    int c = getopt_long(argc, argv,optStr.toUtf8(),long_options, &option_index);
    if (c == -1) break;
    switch (c) {
      case 'y':
        sscanf(optarg,"%d",&opt.year);
        if ( opt.year < 100 ) {
          opt.year += 2000;
        }
      break;
      case 'm':
        sscanf(optarg,"%d",&opt.month);
      break;
      case 'd':
        sscanf(optarg,"%d",&opt.day);
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
			case 'h': case '?':
				info_log << QObject::tr("Использование:\n"
				  "\t%1 -s <спутник> -l|-g [-e|-o] [-r] [-i] [-y <год>][-m <месяц>][-d <день>] [-w <TLE>] файл\n"
				  "Опции:\n"
				  "\t-e\t считать параметры из заголовка\n"
				  "\t-o\t считать параметры из заголовка старого (виндового) формата\n"
				  "\t-c\t поток закодированный манчестером\n"
				  "\t-f\t поток без разделения по кадрам\n"
				  "\t-i\t биты необходимо инвертировать\n"
				  "\t-s\t название спутника\n"
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

	QString fileName = argv[optind];

	parseStream(fileName, weatherFileName, path, opt);

	return 0;
}
