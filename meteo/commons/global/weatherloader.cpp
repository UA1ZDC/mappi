#include <meteo/commons/global/weatherloader.h>

#include <qdir.h>
#include <qfile.h>
#include <qdir.h>

#include <cross-commons/app/paths.h>
#include <meteo/commons/global/global.h>
#include <commons/textproto/tprototext.h>
#include <commons/textproto/pbtools.h>
#include <meteo/commons/proto/cron.pb.h>
#include <meteo/commons/global/gradientparams.h>
#include <meteo/commons/global/radarparams.h>

namespace meteo {
namespace map {

template<> internal::WeatherLoader* WeatherLoader::_instance = 0;

namespace internal {

WeatherLoader::WeatherLoader()
{
  reloadTypes();
}

WeatherLoader::~WeatherLoader()
{
}

bool WeatherLoader::reloadTypes()
{
  loadPunchLibrary();
  loadIsolineLibrary();
  loadRadarLibrary();
  loadPuncmaps();
  loadWeathermaps();
  loadFormals();
  loadOrnamentLibrary();
  loadMapJobs();
  return true;
}

void WeatherLoader::addPunchToLibrary( const meteo::puanson::proto::Puanson& punch )
{
  QDir dir(MnCommon::sharePath("meteo") + "/punch/");
  QString filename = dir.filePath( QString::fromStdString( punch.code() ) + ".template" );
  QFile file(filename);
  if ( false == file.open( QIODevice::WriteOnly ) ) {
    error_log << QObject::tr("Не удалось сохранить шаблон пуансона в файл %1")
      .arg(filename);
    return;
  }
  meteo::puanson::proto::Puanson p;
  p.CopyFrom(punch);
  p.set_path( filename.toStdString() );
  QByteArray arr = TProtoText::toText(p);
  file.write( arr.data(), arr.size() );
  file.flush();
  file.close();
  loadPunchLibrary();
}

void WeatherLoader::rmPunchFromLibrary( const meteo::puanson::proto::Puanson& punch )
{
  QDir dir(MnCommon::sharePath("meteo") + "/punch/");
  QString filename = dir.filePath( QString::fromStdString( punch.code() ) + ".template" );
  QFile file(filename);
  if ( false == file.exists() ) {
    warning_log << QObject::tr("Попытка удалить несуществуующий шаблон пуансона %1")
      .arg(filename);
  }
  else {
    warning_log << QObject::tr("Удаляется шаблон пуансона %1")
      .arg(filename);
  }
  dir.remove(filename);
  loadPunchLibrary();
}

void WeatherLoader::loadPunchLibrary()
{
  punchlibrary_.clear();
  punchlibraryspecial_.clear();
  QDir dir(MnCommon::sharePath("meteo") + "/punch/");
  if ( false == dir.exists() ) {
    error_log << QObject::tr("Не найдена директория с библиотекой пуансонов %1")
      .arg( dir.absolutePath() );
    return;
  }
  QStringList list = dir.entryList( QDir::Files | QDir::NoDotAndDotDot );
  if ( 0 == list.size() ) {
    error_log << QObject::tr("Пуансоны не найдены");
    return;
  }
  for ( int i = 0, sz = list.size(); i < sz; ++i ) {
    QFile file( dir.absoluteFilePath( list[i] ) );
    if ( false == file.open( QIODevice::ReadOnly ) ) {
      error_log << QObject::tr("Не удалось открыть файл с пуансоном %1.")
        .arg( file.fileName() );
      continue;
    }
    QByteArray arr = file.readAll();
    meteo::puanson::proto::Puanson punch;
    if ( false == TProtoText::fillProto( arr, &punch ) ) {
      error_log << QObject::tr("Не удалось загрузить пуансон из файла %1")
        .arg( file.fileName() );
      continue;
    }
    for ( int k = 0; k < punch.rule_size(); ++k ) {
      puanson::proto::CellRule* rule = punch.mutable_rule(k);
      for ( int l = 0; l < rule->symbol().symbol_size(); ++l ) {
        if ( false == rule->mutable_symbol()->mutable_symbol(l)->has_patternba() && true == rule->mutable_symbol()->mutable_symbol(l)->has_pattern() ) {
          rule->mutable_symbol()->mutable_symbol(l)->set_patternba( QString::fromStdString(rule->symbol().symbol(l).pattern()).toUtf8().toHex() );
          rule->mutable_symbol()->mutable_symbol(l)->clear_pattern();
        }
      }
    }
//    TProtoText::toFile( punch, dir.absoluteFilePath( list[i] ) );
    QString code = QString::fromStdString( punch.code() );
    if ( true == punchlibrary_.contains(code) ) {
      warning_log << QObject::tr("Пуансон с идентификатором %1 встретился повторно в файле %2")
        .arg( code )
        .arg( file.fileName() );
    }
    if ( meteo::puanson::proto::kSpecial != punch.datatype() ) {
      punchlibrary_.insert( code, punch );
    }
    else {
      punchlibraryspecial_.insert( code, punch );
    }
  }
}

void WeatherLoader::loadIsolineLibrary()
{
  isolinelibrary_.clear();

  QString str( meteo::global::kIsoParamPath() );
  GradientParams params(str);
  const proto::FieldColors& colors = params.protoParams();
  for ( int i = 0, sz = colors.color_size(); i < sz; ++i ) {
    const proto::FieldColor& clr = colors.color(i);
    QString name = QString::number( clr.descr() );
    if ( true == isolinelibrary_.contains(name) ) {
      warning_log << QObject::tr("Настройка вида изолиний %1 встретилась повторно!")
        .arg(name);
    }
    isolinelibrary_.insert( name, clr );
  }
}

void WeatherLoader::loadRadarLibrary()
{
  radarlibrary_.clear();

  QString str( meteo::global::kRadarParamPath() );
  RadarParams params(str);
  const proto::RadarColors& colors = params.protoParams();
  for ( int i = 0, sz = colors.color_size(); i < sz; ++i ) {
    const proto::RadarColor& clr = colors.color(i);
    QString name = QString::fromStdString( clr.name() );
    if ( true == radarlibrary_.contains(name) ) {
      warning_log << QObject::tr("Настройка вида радаров %1 встретилась повторно!")
        .arg(name);
    }
    radarlibrary_.insert( name, clr );
  }
}

void WeatherLoader::loadOrnamentLibrary()
{
  ornamentlibrary_.clear();
  QString path = global::kOrnamentPath();
  QDir dir(path);
  if ( false == dir.exists() ) {
    warning_log << QObject::tr("Директория %1 не существует, но будет создана")
      .arg(path);
    if ( false == dir.mkpath(path) ) {
      error_log << QObject::tr("Не удалось создать директорию %1")
        .arg(path);
      return;
    }
  }
  QMap<QString, proto::Ornament > setts;
  bool res = TProtoText::loadProtosFromDirectory( path, &setts );
  if ( false == res ) {
    error_log << QObject::tr("Не удалось загрузить файлы из директории %1")
      .arg(path);
    return;
  }
  QMapIterator< QString, proto::Ornament> it(setts);
  while ( true == it.hasNext() ) {
    it.next();
    const proto::Ornament& o = it.value();
    ornamentlibrary_.insert( QString::fromStdString( o.name() ), o );
  }
}

void WeatherLoader::addJobToLibrary( const meteo::map::proto::Job& job )
{
  QString jobid = pbtools::toQString( job.name() );
  if ( true == mapjobs().contains(jobid) ) {
    warning_log << QObject::tr("Задание с наименованием %1 уже существует").arg(jobid);
    return;
  }
  QDir dir( global::kWeatherJobsPath() );
  QString filename = dir.filePath( QString::fromStdString( job.name() ) );
  if ( filename.right(4) != ".job" ) {
    filename += ".job";
  }
  QFile file(filename);
  if ( false == file.open( QIODevice::WriteOnly ) ) {
    error_log << QObject::tr("Не удалось сохранить шаблон задания в файл %1")
      .arg(filename);
    return;
  }
  meteo::map::proto::JobList jl;
  jl.add_job()->CopyFrom(job);
  QByteArray arr = TProtoText::toText(jl);
  file.write( arr.data(), arr.size() );
  file.flush();
  file.close();
  loadMapJobs();
}

void WeatherLoader::rmJobFromLibrary(const proto::Job &job)
{
  QDir dir( meteo::global::kWeatherJobsPath() );
  QString filename = dir.filePath( QString::fromStdString( job.name() ) + ".map" );
  QFile file(filename);
  if ( false == file.exists() ) {
    warning_log << QObject::tr("Попытка удалить несуществующую задачу по формированию карты погоды %1")
      .arg(filename);
  }
  else {
    warning_log << QObject::tr("Удаляется задача по формированию карты погоды %1")
      .arg(filename);
  }
  dir.remove(filename);
  loadMapJobs();
}

bool WeatherLoader::updateJobLibrary(const proto::JobList &jobs)
{
  QString cron = MnCommon::etcPath() + "/cron.d/create.map.conf";
  meteo::cron::Settings setts;

  QDir dir( meteo::global::kWeatherJobsPath() );
  QStringList list = dir.entryList( QDir::Files | QDir::NoDotAndDotDot );
  for ( int i = 0, sz = list.size(); i < sz; ++i ) {
    dir.remove( dir.absoluteFilePath(list[i]) );
  }
  bool ok = true;
  for ( int i = 0, sz = jobs.job_size(); i < sz; ++i )
  {

    QString filename = QString("%1/%2")
        .arg( meteo::global::kWeatherJobsPath() )
        .arg( QString::fromStdString( jobs.job(i).name()));
    if ( ".job" != filename.right(4) ) {
      filename += ".job";
    }
    proto::JobList jl;
    auto j = jl.add_job();
    j->CopyFrom(jobs.job(i));
    ok = ok && TProtoText::toFile( jl, filename );

    if(false == jobs.job(i).enabled()) continue;

    auto runApp = setts.add_run_app();
    runApp->set_timesheet(jobs.job(i).timesheet());
    QString path = QString("%1%2")
        .arg(MnCommon::binPath())
        .arg("meteo.map.client");
    auto arg1 = runApp->add_arg();
    arg1->set_key("-j");
    auto arg2 = runApp->add_arg();
    arg2->set_key(jobs.job(i).name());
    QString name = QString("%1 %2 %3")
        .arg(QObject::tr("Формирование карты"))
        .arg(QString::fromStdString(jobs.job(i).map().title()))
        .arg(QString::fromStdString(jobs.job(i).title()));
    runApp->set_path(path.toStdString());
    runApp->set_name(name.toStdString());
  }

  ok = ok && TProtoText::toFile(setts, cron);
  return ok;
}

void WeatherLoader::loadMapJobs()
{
  mapjobs_.clear();
  QString path = global::kWeatherJobsPath();
  QDir dir(path);
  if ( false == dir.exists() ) {
    warning_log << QObject::tr("Директория %1 не существует, но будет создана")
      .arg(path);
    if ( false == dir.mkpath(path) ) {
      error_log << QObject::tr("Не удалось создать директорию %1")
        .arg(path);
      return;
    }
  }
  QMap< QString, proto::JobList > setts;
  bool res = TProtoText::loadProtosFromDirectory( path, &setts );
  if ( false == res ) {
    error_log << QObject::tr("Не удалось загрузить файлы из директории %1")
      .arg(path);
    return;
  }
  QMapIterator< QString, proto::JobList > it(setts);
  while ( true == it.hasNext() ) {
    it.next();
    const proto::JobList& jobs = it.value();
    for ( int i = 0, sz = jobs.job_size(); i < sz; ++i ) {
      const proto::Job& job = jobs.job(i);
      if ( false == job.has_map_id() ) {
        continue;
      }
      mapjobs_.insert( pbtools::toQString( job.name() ), job );
    }
  }
}

void WeatherLoader::loadDocJobs()
{
  docjobs_.clear();
  QString path = global::kFormalJobsPath();
  QDir dir(path);
  if ( false == dir.exists() ) {
    warning_log << QObject::tr("Директория %1 не существует, но будет создана")
      .arg(path);
    if ( false == dir.mkpath(path) ) {
      error_log << QObject::tr("Не удалось создать директорию %1")
        .arg(path);
      return;
    }
  }
  QMap< QString, proto::JobList > setts;
  bool res = TProtoText::loadProtosFromDirectory( path, &setts );
  if ( false == res ) {
    error_log << QObject::tr("Не удалось загрузить файлы из директории %1")
      .arg(path);
    return;
  }
  QMapIterator< QString, proto::JobList > it(setts);
  while ( true == it.hasNext() ) {
    it.next();
    const proto::JobList& jobs = it.value();
    for ( int i = 0, sz = jobs.job_size(); i < sz; ++i ) {
      const proto::Job& job = jobs.job(i);
      if ( false == job.has_formal() ) {
        continue;
      }
      docjobs_.insert( pbtools::toQString( job.name() ), job );
    }
  }
}

void WeatherLoader::addMapToLibrary( const proto::Map& map )
{
  QString mapid = pbtools::toQString( map.name() );
  if ( true == weathermaps().contains(mapid) ) {
    warning_log << QObject::tr("Вид каты с наименованием %1 уже существует").arg(mapid);
    return;
  }
  QDir dir( meteo::global::kWeatherPatternPath() );
  QString filename = dir.filePath( pbtools::toQString( map.name() ) );
  if ( ".map" != filename.right(4) ) {
    filename += ".map";
  }
  QFile file(filename);
  if ( false == file.open( QIODevice::WriteOnly ) ) {
    error_log << QObject::tr("Не удалось сохранить шаблон карты погоды в файл %1")
      .arg(filename);
    return;
  }
  QByteArray arr = TProtoText::toText(map);
  file.write( arr.data(), arr.size() );
  file.flush();
  file.close();
  loadWeathermaps();
}

void WeatherLoader::rmMapFromLibrary( const proto::Map& map )
{
  QDir dir( meteo::global::kWeatherPatternPath() );
  QString filename = dir.filePath( QString::fromStdString( map.name() ) + ".map" );
  QFile file(filename);
  if ( false == file.exists() ) {
    warning_log << QObject::tr("Попытка удалить несуществуующий шаблон карты погоды %1")
      .arg(filename);
  }
  else {
    warning_log << QObject::tr("Удаляется шаблон карты погоды %1")
      .arg(filename);
  }
  dir.remove(filename);
  loadWeathermaps();
}

meteo::map::proto::FieldColor WeatherLoader::isoparams( const QString& templatename, bool* ok ) const
{
  if ( 0 != ok ) {
    *ok = false;
  }
  if ( false == isolinelibrary_.contains(templatename) ) {
    return emptyiso_;
  }
  if ( 0 != ok ) {
    *ok = true;
  }
  return isolinelibrary_[templatename];
}

meteo::map::proto::RadarColor WeatherLoader::radarparams( const QString& templatename, bool* ok ) const
{
  if ( 0 != ok ) {
    *ok = false;
  }
  if ( false == radarlibrary_.contains(templatename) ) {
    return emptyradar_;
  }
  if ( 0 != ok ) {
    *ok = true;
  }
  return radarlibrary_[templatename];
}

meteo::puanson::proto::Puanson WeatherLoader::punchparams( const QString& templatename, bool* ok ) const
{
  if ( 0 != ok ) {
    *ok = false;
  }
  if ( true == punchlibrary_.contains(templatename) ) {
    if ( 0 != ok ) {
      *ok = true;
    }
    return punchlibrary_[templatename];
  }
  if ( true == punchlibraryspecial_.contains(templatename) ) {
    if ( 0 != ok ) {
      *ok = true;
    }
    return punchlibraryspecial_[templatename];
  }
  return emptypunch_;
}

void WeatherLoader::loadPuncmaps()
{
  punchmaps_.clear();
  QMap<QString, proto::Map> protos;
  if ( false == TProtoText::loadProtosFromDirectory( meteo::global::kPunchMapPath(), &protos ) ) {
    error_log << QObject::tr("Не удалось загрузить шаблоны слоев с пуансонами. Шаблоны слоев с пуансонами недоступны.");
  }
  QMapIterator<QString, proto::Map> it(protos);
  while ( true == it.hasNext() ) {
    it.next();
    const proto::Map& map = it.value();
    QString name = QString::fromStdString( map.name() );
    if ( true == punchmaps_.contains(name) ) {
      warning_log << QObject::tr("Шаблон слоя пуансонов с идентификатором %1 встретился повторно в файле %2. Не надо так :(.")
        .arg( name )
        .arg( it.key() );
    }
    punchmaps_.insert( name, map );
  }
}

void WeatherLoader::loadWeathermaps()
{
  weathermaps_.clear();
  QMap<QString, proto::Map> protos;
  if ( false == TProtoText::loadProtosFromDirectory( meteo::global::kWeatherPatternPath(), &protos ) ) {
    error_log << QObject::tr("Не удалось загрузить шаблоны карт погоды. Шаблоны карт погоды недоступны.");
  }
  QMapIterator<QString, proto::Map> it(protos);
  while ( true == it.hasNext() ) {
    it.next();
    const proto::Map& map = it.value();
    QString name = QString::fromStdString( map.name() );
    if ( true == weathermaps_.contains(name) ) {
      warning_log << QObject::tr("Шаблон карты погоды с идентификатором %1 встретился повторно в файле %2. Не надо так :(.")
        .arg( name )
        .arg( it.key() );
    }
    weathermaps_.insert( name, map );
  }
}

void WeatherLoader::loadFormals()
{
  formals_.clear();
  QMap<QString, proto::Map> protos;
  if ( false == TProtoText::loadProtosFromDirectory( meteo::global::kDocumentPatternPath(), &protos ) ) {
    error_log << QObject::tr("Не удалось загрузить шаблоны карт погоды. Шаблоны карт погоды недоступны. Путь к директории = %1")
      .arg( meteo::global::kDocumentPatternPath() );
  }
  QMapIterator<QString, proto::Map> it(protos);
  while ( true == it.hasNext() ) {
    it.next();
    const proto::Map& formal = it.value();
    QString name = QString::fromStdString( formal.formal().name() );
    if ( true == formals_.contains(name) ) {
      warning_log << QObject::tr("Шаблон карты погоды с идентификатором %1 встретился повторно в файле %2. Не надо так :(.")
        .arg( name )
        .arg( it.key() );
    }
    formals_.insert( name, formal );
  }
}

}
}
}
