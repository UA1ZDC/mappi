#include <mappi/proto/thematic.pb.h>
#include <meteo/commons/proto/field.pb.h>

#include <mappi/thematic/algs_calc/themformat.h>
#include <mappi/thematic/algs_calc/themalg.h>
#include <mappi/thematic/algs_calc/dataservstore.h>

#include <mappi/settings/mappisettings.h>
#include <mappi/global/global.h>
#include <commons/proc_read/daemoncontrol.h>

#include <QCoreApplication>
#include <QtConcurrent>

#include <unistd.h>
#include <getopt.h>

using namespace mappi;




bool getThemConf(const QString& satname, QMap< std::string,conf::ThemType>* typelist)
{
  conf::ThematicProcs conf;
  conf.CopyFrom(mappi::inter::Settings::instance()->thematicsCalc());
  
  if (!conf.IsInitialized()) {
    error_log << QObject::tr("Ошибка файла настроек тематических обработок");
    return false;
  }

  //var(conf.Utf8DebugString());
  var(satname);

  for(const auto &them : conf.thematics()) {
    for(const auto &vars : them.vars()) {
      for(const auto &channel : vars.channel()) {
        if (satname == QString::fromStdString(channel.satellite())) {
          //if(false == typelist->contains(them.type())){
            typelist->insert(them.name(),them.type());
          //}
          break;
        }
      }
    }
  }
  debug_log << satname << *typelist ;

  if (typelist->isEmpty()) {
    info_log << QObject::tr("Нет предустановленных настроек");
    return false;
  }

  return true;
}

bool processTasks(QSharedPointer<to::ThemAlg> &alg){
  debug_log << "Запускаем" << alg->name() << "в потоке:" << QThread::currentThread();

  if(!alg->loadData()) {
    error_log << "Ошибка загрузки данных" << alg->name() << "в потоке:" << QThread::currentThread();
    alg.clear();
    return false;
  }
  if(!alg->process()) {
    error_log << "Ошибка обработки" << alg->name() << "в потоке:" << QThread::currentThread();
    alg.clear();
    return false;
  }
  if(!alg->saveImg()) {
    error_log << "Ошибка сохранения изображения" << alg->name() << "в потоке:" << QThread::currentThread();
    alg.clear();
    return false;
  }
  if(!alg->saveData()) {
    error_log << "Ошибка сохранения данных" << alg->name() << "в потоке:" << QThread::currentThread();
    alg.clear();
    return false;
  }

  debug_log << "Завершен" << alg->name() << "в потоке:" << QThread::currentThread();
  alg.clear();
  return true;
}

void dataProcess(const QDateTime& start, const QString& satname, const QList<conf::InstrumentType>& )
{
  QMap<std::string,conf::ThemType> typelist;
  bool ok = getThemConf(satname, &typelist);
  if (!ok) return;
  QSharedPointer<to::DataStore> store = QSharedPointer<to::DataStore>(new to::DataServiceStore());

  QList<QSharedPointer<to::ThemAlg>> tasks;
  QMapIterator<std::string,conf::ThemType> i(typelist);
  while (i.hasNext()) {
    i.next();
    auto alg = QSharedPointer<to::ThemAlg>(to::singleton::ThemFormat::instance()->createThemAlg(i.value(), i.key(), store));
    if (alg.isNull()) continue;
    if (!alg->init(start, satname)) continue;
    tasks.append(alg);
  }
  const int maxThreads = 1; //fix
  QThreadPool::globalInstance()->setMaxThreadCount(maxThreads);
  QtConcurrent::map(tasks, processTasks).waitForFinished();
}

void printHelp(const QString& progName)
{
	info_log << QObject::tr("\nИспользование: %1 [option]\n").arg(progName)
					 << QObject::tr("Опции:\n")
					 << QObject::tr("-n <satname>\t\t Название спутника\n")
					 << QObject::tr("-s <start_dt>\t\t Дата/время начала прием\n")
					 << QObject::tr("-i <instrument_type>\t Тип прибора (через запятую, если несколько)\n")
					 << QObject::tr("-h \t\t Справка\n");
}

//-n 'NOAA 19' -s '2022-07-12 13:41:37'
//test_ndvi("NOAA 19", QDateTime(QDate(2022, 07, 12), QTime(13, 41, 37)));

int main(int argc, char *argv[])
{
  QCoreApplication app(argc, argv);
  TAPPLICATION_NAME("meteo");

  static struct option long_options[] = {
  {"satname", 1, 0, 'n'},
  {"instr",   1, 0, 'i'},
  {"start",   1, 0, 's'},
  {"help",    0, 0, 'h'},
  {0,0,0,0}
};

  QString optStr = QString("n:s:i:h");

  QString satname;
  QDateTime start;
  QStringList instrs;

  while (1) {
    int option_index = 0;
    int c = getopt_long(argc, argv, optStr.toUtf8(), long_options, &option_index);
    if (c == -1) break;
    switch (c) {
      case 's':
        start = QDateTime::fromString(optarg, Qt::ISODate);
      break;
      case 'n':
        satname = optarg;
      break;
      case 'i':
        instrs = QString(optarg).split(',', QString::SkipEmptyParts);
      break;
      case 'h': case '?':
        printHelp(argv[0]);
        exit(0);
    }
  }
  
  // ::meteo::gGlobalObj(new ::mappi::MappiGlobal);
  // meteo::mappi::TMeteoSettings::instance()->load();

  meteo::gSettings(mappi::inter::Settings::instance());
  
  if ( false == meteo::gSettings()->load() ) {
    error_log << meteo::msglog::kSettingsLoadFailed;
    return -1;
  }

  QList<conf::InstrumentType> itypes;
  for (const auto &one : qAsConst(instrs)) {
    bool ok;
    int type = one.toInt(&ok);
    if (ok) {
      itypes.append(conf::InstrumentType(type));
    }
  }

  if (/*itypes.isEmpty() ||*/ satname.isEmpty() || !start.isValid()) {
    error_log << QObject::tr("Ошибочное указание параметров") << satname << itypes << start;
    return -1;
  }
  /////////////

  try {
    dataProcess(start, satname, itypes);
  }
  
  catch (const std::bad_alloc &) {
    critical_log << QObject::tr("Недостаточно памяти для функционирования приложения");
    return EXIT_FAILURE;
  }
  
  return 0;
}
