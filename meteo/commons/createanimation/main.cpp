#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>
#include <meteo/commons/global/log.h>
#include <meteo/commons/global/global.h>

#include <qdir.h>
#include <qfiledialog.h>
#include <qapplication.h>
#include <qtextcodec.h>
#include <qdebug.h>
#include <meteo/commons/settings/settings.h>

#include <Magick++.h>

#include <unistd.h>

//delay в мс
bool createImg(const Magick::Blob& map, Magick::Image* one, int delay)
{
  bool ok = true;
  try {
    one->read(map);
    one->magick("GIF");
    one->animationDelay(delay / 10);
    one->animationIterations(0);
    one->quantize();
  }
  catch ( ... ) {
    ok = false;
  }
  return ok;
}

bool createAnime(const QString& resFile, const QString& dataFile, int delay)
{
  bool ok = true;

  Magick::InitializeMagick(0);

  QFile file(dataFile);
  if (!file.open(QIODevice::ReadOnly)) {
    return false;
  }

  QFileInfo fi(resFile);
  QDir dir;
  dir.mkpath(fi.dir().path());

  QDataStream ds(&file);

  int cnt = 0;
  ds >> cnt;

  if (0 != cnt) {
    std::list<Magick::Image> imgs;
    for (int idx = 0; idx < cnt; idx++) {
      int size;
      ds >> size;
      char* data = new char[size]; //очистка в Blob
      ds.readRawData(data, size);
      
      Magick::Blob map;//(img.data(), img.size());
      map.updateNoCopy(data, size);
      
      Magick::Image one;
      if (createImg(map, &one, delay)) {
	imgs.push_back(one);
      }
    }

    try {
      Magick::mapImages(imgs.begin(), imgs.end(), imgs.front(), false);
      Magick::writeImages(imgs.begin(), imgs.end(), resFile.toStdString());
    }
    catch (Magick::Error& my_error) {
      qCritical() << meteo::msglog::kImageCreateFailed + QString(": %1").arg(my_error.what());
      ok = false;
    }
    catch ( ... ) {
      ok =  false;
    }
  } else {
    ok = false;
  }

  file.close();

  // QFile er("/tmp/err.txt");
  // er.open(QIODevice::WriteOnly);
  // QTextStream ts(&er);
  // ts << "res=" << resFile << "\n";
  // ts << "src=" << dataFile << "\n";
  // ts << "delay=" << delay << "\n";
  // ts << "ok=" << ok;

  // er.close();

  return ok;
}

void printHelp(const QString& progName)
{
  info_log << QObject::tr("\nИспользование: %1 [option] <source>\n").arg(progName)
	   << QObject::tr("source\t\t Файл с данными\n")
	   << QObject::tr("Опции:\n")
    	   << QObject::tr("-r <name>\t Файл для сохранения (по умолчанию используется название файла с данными)\n")
	   << QObject::tr("-d <delay>\t Пауза между кадрами в мс (по умолчанию 1 секунда)\n")
	   << QObject::tr("-h\t\t Справка\n");
}


int main( int argc, char* argv[] )
{
  TAPPLICATION_NAME("meteo");

  QApplication app( argc, argv );

  meteo::Settings* sett = meteo::global::Settings::instance();
  if ( false == sett->load() ){
    error_log << meteo::msglog::kSettingsLoadFailed.arg(sett->name());
    return EXIT_FAILURE;
  }

  meteo::global::setLogHandler();
//  //QTextCodec::setCodecForCStrings( QTextCodec::codecForLocale() );
  
  int opt;
  int delay = 1000;
  QString result, source;
  while ((opt = getopt(argc, argv, "hr:d:")) != -1) {
    switch (opt) {
    case 'h':
      printHelp(argv[0]);
      exit(0);
      break;
    case 'r':
      result = QString(optarg).trimmed();
      break;
    case 'd':
      delay = atoi(optarg);
      break;
    default: /* '?' */
      exit(-1);
    }
  }

  if (optind >= argc) {
    error_log << QObject::tr("Необходимо задать файл с данными. \"%1 -h\" для вызова справки").arg(argv[0]);
    exit(-1);
  }
  
  source = QString(argv[optind]).trimmed();
  
  if (result.isEmpty()) {
    result = source.remove(QRegExp("\\.\\w+$")) + ".gif";
  }

  // QFile er("/tmp/run.txt");
  // er.open(QIODevice::WriteOnly);
  // QTextStream ts(&er);
  // ts << "res=" << result << "\n";
  // ts << "src=" << source << "\n";
  // ts << "delay=" << delay << "\n";
  // er.close();

  bool ok = createAnime(result, source, delay);
  if (!ok) return 2;

  return 0;  
}
       
