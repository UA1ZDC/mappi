#include <qcoreapplication.h>
#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>

#include <mappi/thematic/algs/themformat.h>
#include <mappi/thematic/algs/themalg.h>
#include <mappi/thematic/algs/dataservstore.h>

#include <mappi/settings/mappisettings.h>
#include <mappi/global/global.h>

#include <unistd.h>

#include <qfile.h>
#include <qdatastream.h>
#include <qimage.h>

using namespace mappi;
using namespace to;


void test_gray(const QString& satName, const QDateTime& dt)
{
  QSharedPointer<DataStore> store = QSharedPointer<DataStore>(new DataServiceStore());
  
  ThemAlg* alg = singleton::ThemFormat::instance()->createThemAlg(conf::kGrayScale, store);

  QList<conf::InstrumentType> instruments;
  instruments << conf::kMsuMr;
  
  bool ok = alg->process(dt, satName, instruments);
  var(ok);

  delete alg;
}


void test_ndvi(const QString& satName, const QDateTime& dt)
{
  QSharedPointer<DataStore> store = QSharedPointer<DataStore>(new DataServiceStore());
  
  ThemAlg* alg = singleton::ThemFormat::instance()->createThemAlg(conf::kNdvi, store);

  QList<conf::InstrumentType> instruments;
  instruments << conf::kAvhrr3 << conf::kAmsuA1;
  
  bool ok = alg->process(dt, satName, instruments);
  var(ok);

  delete alg;
}

void test_rgb(const QString& satName, const QDateTime& dt)
{
  QSharedPointer<DataStore> store = QSharedPointer<DataStore>(new DataServiceStore());
  
  //  ThemAlg* alg = singleton::ThemFormat::instance()->createThemAlg(conf::kRgbDayMicrophisics, store);
  ThemAlg* alg = singleton::ThemFormat::instance()->createThemAlg(conf::kRgbNatural, store);
  //ThemAlg* alg = singleton::ThemFormat::instance()->createThemAlg(conf::kFalseColor, store);

  QList<conf::InstrumentType> instruments;
  instruments << conf::kAvhrr3 << conf::kAmsuA1;
  
  bool ok = alg->process(dt, satName, instruments);
  var(ok);

  delete alg;
}

// void process(mappi::proto::ThemType type, const QString& baseName)
// {
//   ThemAlg* alg =  singleton::ThemFormat::instance()->createThemAlg(type);

//   bool ok = false;


//   //grayscale
//   if (type == proto::kGrayScale) {
//     for (int idx = 1; idx < 6; idx++) {
//       QString fileName = baseName + "_" + QString::number(idx).rightJustified(2, '0') + ".po";
//       alg->clear();
//       var(fileName);
//       ok = alg->readChannelData(fileName, "CH");
//       if (!ok) {
// 	var(ok);
// 	continue;
//       }
//       ok = alg->process();
//       if (!ok) {
// 	var(ok);
// 	continue;
//       }
//       ok = alg->saveImage(baseName.section("/", -1, -1));
//       if (!ok) {
// 	var(ok);
// 	continue;
//       }
//     }
//   }

//   //falsecolor
//   if (type == proto::kFalseColor) {
//     ok  = alg->readChannelData(baseName + "_02.po", "CH1");//r
//     ok &= alg->readChannelData(baseName + "_03.po", "CH2");//g
//     ok &= alg->readChannelData(baseName + "_01.po", "CH3");//b
//     if (!ok) {
//       var(ok);
//       return;
//     }
//     ok = alg->process();
//     if (!ok) {
//       var(ok);
//       return;
//     }
//     ok = alg->saveImage(baseName.section("/", -1, -1));
//     if (!ok) {
//       var(ok);
//       return;
//     }    
//   }


//   //ndvi
//   if (type == proto::kIndex8) {
//     ok = alg->readChannelData(baseName + "_01.po", "R0.6");
//     ok &= alg->readChannelData(baseName + "_02.po", "R0.9");
//     if (!ok) {
//       var(ok);
//       return;
//     }
//     ok = alg->process();
//     if (!ok) {
//       var(ok);
//       return;
//     }
//     ok = alg->saveImage(baseName.section("/", -1, -1));
//     if (!ok) {
//       var(ok);
//       return;
//     }
//   }

//   alg->saveMeta(baseName.section("/", -1, -1));
// }

int main(int argc, char* argv[])
{
  TAPPLICATION_NAME("meteo");

  int opt;
  mappi::conf::ThemType type = conf::kThemUnk;

  while ((opt = getopt(argc, argv, "t:h")) != -1) {
    switch (opt) {
      case 't':
        type = mappi::conf::ThemType(atoi(optarg));
      break;
      case 'h':
        info_log << "\nUsage:" << argv[0] << "-t 1|2|3 <fileName>\n"
	"  -t        тематическая обработка (ThemType)\n"
	"  baseName  raw satellite data baseName (напр. /<path>/201711011009_NOAA19)\n";
				exit(0);
			break;
			default: /* '?' */
				error_log<<"Option"<<opt<<"not realized";
				exit(-1);
		}
	}

	if (type == conf::kThemUnk) {
		error_log << "Need select test with option. \"" << argv[0] << "-h\" for help";
		//    exit(-1);
	}

	// if (optind >= argc) {
	//   error_log << "Need point file name. \"" << argv[0] << "-h\" for help";
	//   exit(-1);
	// }

	QString baseName = argv[optind];



	meteo::gSettings(mappi::inter::Settings::instance());

	if ( false == meteo::gSettings()->load() ){
		error_log << meteo::msglog::kSettingsLoadFailed;
		return -1;
	}

	QCoreApplication app(argc, argv);

	//test_gray("METEOR-M 2", QDateTime(QDate(2022, 10, 7), QTime(07, 51, 33)));
	test_ndvi("NOAA 19", QDateTime(QDate(2022, 07, 12), QTime(13, 41, 37)));
	//test_ndvi("NOAA 18", QDateTime(QDate(2021, 3, 1), QTime(12, 45, 01)));
	// test_rgb("NOAA 18", QDateTime(QDate(2021, 3, 1), QTime(12, 45, 01)));
	//test_rgb("GOMS-3", QDateTime(QDate(2021, 3, 1), QTime(12, 45, 01)));
	//process(type, baseName);

	return 0;
}
