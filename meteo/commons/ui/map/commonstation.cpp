#include "commonstation.h"

#include <commons/meteo_data/meteo_data.h>
#include <meteo/commons/global/common.h>
#include <meteo/commons/proto/meteo.pb.h>
#include <meteo/commons/proto/sprinf.pb.h>
#include <meteo/commons/proto/surface_service.pb.h>
#include <meteo/commons/ui/map/document.h>
#include <meteo/commons/ui/map/geopolygon.h>
#include <meteo/commons/ui/map/puanson.h>
#include <meteo/commons/ui/map/puansonairport.h>
#include <meteo/commons/ui/map/layerpunch.h>
#include <meteo/commons/ui/map/layerairport.h>
#include <meteo/commons/ui/map/layerborder.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/global/weatherloader.h>

#include "loader.h"

namespace {
  static bool res = meteo::map::Loader::instance()->registerStationHandler(meteo::global::kStationLoaderCommon, &meteo::map::loadStationLayer );
}

namespace meteo {
namespace map {

const QString kGrid01("Grid-01");
const QString kGrid02("Grid-02");
const QString kGrid05("Grid-05");
const QString kGrid10("Grid-10");
const QString kGrid30("Grid-30");
const int kRus = 164;

QString getName( const QString& filename ) {
  QFile file(filename+".name");
  QString defname;
  if ( filename == global::kStationSynopCacheFile ) {
    defname = QObject::tr("Синоптические станции");
  }
  else if ( filename == global::kStationAeroCacheFile ) {
    defname = QObject::tr("Аэрологические станции");
  }
  else if ( filename == global::kStationAirportCacheFile ){
    defname = QObject::tr("Аэродромы");
  }
  else if ( filename == global::kStationAerodromeCacheFile ){
    defname = QObject::tr("Военные аэродромы");
  }
  else {
    return QString();
  }
  if ( false == file.exists() ) {
    file.open( QIODevice::WriteOnly );
    file.write( defname.toLocal8Bit() );
    file.flush();
    file.close();
  }
  if ( true == file.open( QIODevice::ReadOnly ) ) {
    defname = QString::fromLocal8Bit( file.readAll() );
  }
  return defname;
}

bool loadFromCache( Document* doc, const QString& filename, const puanson::proto::Puanson& punch, const QString& id_descr )
{
  QFile file(filename);
  if ( false == file.exists() ) {
    return false;
  }
  if ( false == file.open( QIODevice::ReadOnly ) ) {
    return false;
  }
  Stations cache;
  QByteArray arr = file.readAll();
  if ( false == cache.ParseFromArray( arr.data(), arr.size() ) ) {
    return false;
  }
  if ( 0 == cache.station_size() ) {
    return false;
  }
  LayerPunch* l = new LayerPunch( doc, punch );
  l->setName( getName(filename) );
  l->setBase(true);

  for ( int i = 0, sz = cache.station_size(); i < sz; ++i ) {
    const Station& st = cache.station(i);
    if ( 0 == st.index() && false == st.has_cccc() ) {
      continue;
    }
    Puanson* p = new Puanson(l);
    p->setPunch(punch);
    TMeteoData md;
    if ( "CCCC" == id_descr ) {
      TMeteoParam param( QString::fromStdString( st.cccc() ), 2, control::RIGHT );
      md.add( id_descr, param );
    }
    else {
      TMeteoParam param( QString::number( st.index() ), st.index(), control::RIGHT );
      md.add( id_descr, param );
    }
    p->setMeteodata(md);
    p->setSkelet( pbgeopoint2geopoint( st.coord() ) );
  }
  l->setVisisble(false);
  return true;
}

bool loadFromCacheAirporte( Document* doc, const QString& filename, const puanson::proto::Puanson& punch, const QString& id_descr )
{
  QFile file(filename);
  if ( false == file.exists() ) {
    return false;
  }
  if ( false == file.open( QIODevice::ReadOnly ) ) {
    return false;
  }
  meteo::sprinf::Stations cache;
  QByteArray arr = file.readAll();
  if ( false == cache.ParseFromArray( arr.data(), arr.size() ) ) {
    return false;
  }
  if ( 0 == cache.station_size() ) {
    return false;
  }
  const puanson::proto::Puanson& punchEnemy = WeatherLoader::instance()->punchlibraryspecial()["airport_enemy"];
  LayerAirport* l = new LayerAirport( doc, punch );
  l->setAllyPunch(punch);
  l->setEnemyPunch(punchEnemy);
  l->setName( getName(filename) );
  l->setBase(true);
  for ( int i = 0, sz = cache.station_size(); i < sz; ++i ) {
    const meteo::sprinf::Station& st = cache.station(i);
    if ( 0 == st.index() && false == st.has_cccc() ) {
      continue;
    }

    PuansonAirport* p = new PuansonAirport(l);
    if ( st.country().number() == kRus ) {
      p->setAirportType(PuansonAirport::AirportType::kAlly);
    }
    else {
      p->setAirportType(PuansonAirport::AirportType::kEnemy);
    }
    TMeteoData md;
    if ( "CCCC" == id_descr ) {
      QString cccc;
      if ( true == st.has_cccc() && "" != st.cccc() ) {
        cccc = QString::fromStdString( st.cccc());
      }
      else if ( true == st.has_station() ){
        cccc = QString::fromStdString( st.station() );
      }

      QString name;
      if (st.name().has_rus()) {
        name = QString::fromStdString( st.name().rus() );
      }
      else {
        name = QString::fromStdString( st.name().international() );
      }
      p->setCCCC(cccc);
      p->setName( name );
      TMeteoParam param;
      if (l->showCCCC()) {
        param = TMeteoParam( cccc, 2, control::RIGHT );
      }
      else {
        param = TMeteoParam( name, 2, control::RIGHT );
      }
      md.add( "CCCC", param );
    }
    else {
      TMeteoParam param( QString::number( st.index() ), st.index(), control::RIGHT );
      md.add( id_descr, param );
    }
    p->setMeteodata(md);
    GeoPointPb gp;
    gp.set_lat_radian( st.position().lat_radian() );
    gp.set_lon_radian( st.position().lon_radian() );
    gp.set_height_meters( st.position().height_meters() );
    p->setSkelet( pbgeopoint2geopoint( gp ) );
  }
  l->setVisisble(false);
  return true;
}

void saveToCache( Stations* stations, const QString& filename )
{
  QFile file(filename);
  if ( false == file.open( QIODevice::WriteOnly | QIODevice::Truncate ) ) {
    return;
  }
  std::string str = stations->SerializeAsString();
  int sz = str.size();
  int step = 128;
  int shift = 0;
  while ( 0 < sz ) {
    int locstep = ( step < sz ) ? step : sz;
    file.write( str.data() + shift, locstep );
    shift += locstep;
    sz -= locstep;
  }
  file.flush();
  file.close();
}

void saveToCacheAirport( meteo::sprinf::Stations* stations, const QString& filename )
{
  stations->set_result(true);
  QFile file(filename);
  if ( false == file.open( QIODevice::WriteOnly | QIODevice::Truncate ) ) {
    return;
  }
  std::string str = stations->SerializeAsString();
  int sz = str.size();
  int step = 128;
  int shift = 0;
  while ( 0 < sz ) {
    int locstep = ( step < sz ) ? step : sz;
    file.write( str.data() + shift, locstep );
    shift += locstep;
    sz -= locstep;
  }
  file.flush();
  file.close();
}

bool loadStationLayer( Document* doc, int type, const QString& file, const puanson::proto::Puanson& punch, const QString& id_descr )
{
  if ( true == loadFromCache( doc, file, punch, id_descr ) ) {
    return true;
  }
  meteo::rpc::Channel* ctrl = meteo::global::serviceChannel( meteo::settings::proto::kSprinf );
  if ( 0 == ctrl ) {
    warning_log << QObject::tr("Не удалось подключиться к сервису справки. Станции будут загружены из кэша, если он есть");
    return false;
  }
  meteo::sprinf::MultiStatementRequest request;
  request.add_type(type);
  meteo::sprinf::Stations* response = ctrl->remoteCall( &meteo::sprinf::SprinfService::GetStations, request, 30000 );
  delete ctrl;
  if ( nullptr == response ) {
    warning_log << QObject::tr("Не удалось подключиться к сервису справки. Станции будут загружены из кэша, если он есть");
    return false;
  }
  if ( 0 == response->station_size() ) {
    warning_log << QObject::tr("Пустой список станций. Загрузка из кэша");
    delete response;
    return false;
  }
  LayerPunch* l = new LayerPunch( doc, punch );
  l->setName( getName(file) );
  l->setBase(true);
  Stations cache;

  for ( int i = 0, sz = response->station_size(); i < sz; ++i ) {
    const meteo::sprinf::Station& st = response->station(i);
    if ( 0 == st.index() ) {
      continue;
    }
    Station* cache_st = cache.add_station();
    cache_st->set_index( st.index() );
    GeoPointPb gp;
    gp.set_lat_radian( st.position().lat_radian() );
    gp.set_lon_radian( st.position().lon_radian() );
    cache_st->mutable_coord()->CopyFrom(gp);
    Puanson* p = new Puanson(l);
    p->setPunch(punch);
    TMeteoData md;
    TMeteoParam param( QString::number( cache_st->index() ), cache_st->index(), control::RIGHT );
    md.add( id_descr, param );
    p->setMeteodata(md);
    p->setSkelet( pbgeopoint2geopoint( cache_st->coord() ) );
  }
  saveToCache(&cache,file);
  delete response;

  l->setVisisble(false);
  return true;
}

bool loadAirportLayer( Document* doc, const QString& file, const puanson::proto::Puanson& punch )
{
  if ( true == loadFromCacheAirporte( doc, file, punch, "CCCC" ) ) {
    return true;
  }
  meteo::rpc::Channel* ctrl = meteo::global::serviceChannel( meteo::settings::proto::kSprinf );
  if ( 0 == ctrl ) {
    warning_log << QObject::tr("Не удалось подключиться к сервису справки. Аэропорты будут загружены из кэша, если он есть");
    return false;
  }
  meteo::sprinf::MultiStatementRequest request;
  request.add_type(sprinf::kStationAirport);
  meteo::sprinf::Stations* response = ctrl->remoteCall( &meteo::sprinf::SprinfService::GetStations, request, 30000 );
  delete ctrl;
  if ( nullptr == response ) {
    warning_log << QObject::tr("Не удалось подключиться к сервису справки. Станции будут загружены из кэша, если он есть");
    return false;
  }
  if ( 0 == response->station_size() ) {
    warning_log << QObject::tr("Пустой список станций. Загрузка из кэша");
    delete response;
    return false;
  }

  const puanson::proto::Puanson& punchEnemy = WeatherLoader::instance()->punchlibraryspecial()["airport_enemy"];
  LayerAirport* l = new LayerAirport( doc, punch );
  l->setAllyPunch(punch);
  l->setEnemyPunch(punchEnemy);
  l->setName( getName(file) );
  l->setBase(true);
  meteo::sprinf::Stations cache;
  for ( int i = 0, sz = response->station_size(); i < sz; ++i ) {
    const meteo::sprinf::Station& st = response->station(i);
    auto cache_st = cache.add_station();
    cache_st->CopyFrom(st);
    PuansonAirport* p = new PuansonAirport(l);
    if ( kRus == st.country().number() ) {
      p->setAirportType( PuansonAirport::AirportType::kAlly );
    }
    else {
      p->setAirportType( PuansonAirport::AirportType::kEnemy );
    }
    TMeteoData md;
    QString cccc;
    if ( true == st.has_cccc() && "" != st.cccc() ) {
      cccc = QString::fromStdString( st.cccc());
    }
    else if ( true == st.has_station() ){
      cccc = QString::fromStdString( st.station() );
    }
    QString name;
    if (cache_st->name().has_rus()) {
      name = QString::fromStdString( cache_st->name().rus() );
    }
    else {
      name = QString::fromStdString( cache_st->name().international() );
    }
    TMeteoParam param;
    if (l->showCCCC()) {
      param = TMeteoParam( cccc, 2, control::RIGHT );
    }
    else {
      param = TMeteoParam( name, 2, control::RIGHT );
    }
    p->setCCCC(cccc);
    p->setName( name );
    md.add( "CCCC", param );
    p->setMeteodata(md);
    GeoPointPb gp;
    gp.set_lat_radian( st.position().lat_radian() );
    gp.set_lon_radian( st.position().lon_radian() );
    gp.set_height_meters( st.position().height_meters() );
    p->setSkelet( pbgeopoint2geopoint( gp ) );
  }
  saveToCacheAirport(&cache,file);
  delete response;

  l->setVisisble(false);
  return true;
}

bool loadAerodromeLayer( Document* doc, const QString& file, const puanson::proto::Puanson& punch )
{
  if ( true == loadFromCacheAirporte( doc, file, punch, "CCCC" ) ) {
    return true;
  }
  meteo::rpc::Channel* ctrl = meteo::global::serviceChannel( meteo::settings::proto::kSprinf );
  if ( 0 == ctrl ) {
    warning_log << QObject::tr("Не удалось подключиться к сервису справки. Аэродромы будут загружены из кэша, если он есть");
    return false;
  }
  meteo::sprinf::MultiStatementRequest request;
  request.add_type(sprinf::kStationAerodrome);
  meteo::sprinf::Stations* response = ctrl->remoteCall( &meteo::sprinf::SprinfService::GetStations, request, 30000 );
  delete ctrl;
  if ( 0 == response ) {
    warning_log << QObject::tr("Не удалось подключиться к сервису справки. Станции будут загружены из кэша, если он есть");
    return false;
  }
  if ( 0 == response->station_size() ) {
    warning_log << QObject::tr("Пустой список станций. Загрузка из кэша");
    delete response;
    return false;
  }
  const puanson::proto::Puanson& punchEnemy = WeatherLoader::instance()->punchlibraryspecial()["airport_enemy"];
  LayerAirport* l = new LayerAirport( doc, punch );
  l->setAllyPunch(punch);
  l->setEnemyPunch(punchEnemy);
  l->setName( getName(file) );
  l->setBase(true);
  meteo::sprinf::Stations cache;
  for ( int i = 0, sz = response->station_size(); i < sz; ++i ) {
    const meteo::sprinf::Station& st = response->station(i);
    auto cache_st = cache.add_station();
    cache_st->CopyFrom(st);
    PuansonAirport* p = new PuansonAirport(l);
    if ( kRus == st.country().number() ) {
      p->setAirportType(PuansonAirport::AirportType::kAlly);
    }
    else {
      p->setAirportType(PuansonAirport::AirportType::kEnemy);
    }
    TMeteoData md;
    QString cccc;
    if ( true == st.has_cccc() && "" != st.cccc() ) {
      cccc = QString::fromStdString( st.cccc());
    }
    else if ( true == st.has_station() ){
      cccc = QString::fromStdString( st.station() );
    }
    QString name;
    if (cache_st->name().has_rus()) {
      name = QString::fromStdString( cache_st->name().rus() );
    }
    else {
      name = QString::fromStdString( cache_st->name().international() );
    }
    p->setName( name );
    p->setCCCC(cccc);
    TMeteoParam param;
    if (l->showCCCC()) {
      param = TMeteoParam( cccc, 2, control::RIGHT );
    }
    else {
      param = TMeteoParam( name, 2, control::RIGHT );
    }

    md.add( "CCCC", param );
    p->setMeteodata(md);
    GeoPointPb gp;
    gp.set_lat_radian( st.position().lat_radian() );
    gp.set_lon_radian( st.position().lon_radian() );
    gp.set_height_meters( st.position().height_meters() );
    p->setSkelet( pbgeopoint2geopoint( gp ) );
  }
  saveToCacheAirport(&cache,file);
  delete response;
  l->setVisisble(false);
  return true;
}

bool loadVO( Document* doc )
{
  QFile file( global::kVoLayerFileName );
  if ( false == file.exists() ) {
    error_log << QObject::tr("Не найден файл с военными округами = %1").arg( global::kVoLayerFileName );
    return false;
  }
  if ( false == file.open( QIODevice::ReadOnly ) ) {
    error_log << QObject::tr("Не удалось прочитать файл с военными округами =").arg( global::kVoLayerFileName );;
    return false;
  }
  QByteArray arr = file.readAll();
  bool res = doc->parseFromArray(arr);
  if ( false == res ) {
    error_log << QObject::tr("Не удалось загрузить слой с военными округами");
    return false;
  }
  for ( int i = 0, sz = doc->layers().size(); i < sz; ++i ) {
    Layer* l = doc->layers()[i];
    if ( QObject::tr("Военные округа ") == doc->layers()[i]->name() ) {
      l->setBase(true);
      l->setVisisble(false);
      break;
    }
  }
  return true;
}

bool loadRFBorder( Document* doc )
{
  QFile file( global::kRFBorderFileName );
  if ( false == file.exists() ) {
    error_log << QObject::tr("Не найден файл с границей РФ = %1").arg( global::kRFBorderFileName );
    return false;
  }
  if ( false == file.open( QIODevice::ReadOnly ) ) {
    error_log << QObject::tr("Не удалось прочитать файл с границей РФ =").arg( global::kRFBorderFileName );;
    return false;
  }
  QByteArray arr = file.readAll();
  bool res = doc->parseFromArray(arr);
  if ( false == res ) {
    error_log << QObject::tr("Не удалось загрузить слой с границей РФ");
    return false;
  }
  for ( int i = 0, sz = doc->layers().size(); i < sz; ++i ) {
    LayerBorder* l = static_cast<LayerBorder*>( doc->layers()[i] );
    if ( QObject::tr("Граница РФ") == doc->layers()[i]->name() ) {
      l->loadParams();
      l->setBase(true);
      l->setVisisble(false);
      break;
    }
  }
  return true;
}

bool loadStationLayer( Document* doc )
{
  if ( false == WeatherLoader::instance()->punchlibraryspecial().contains("station") ) {
    error_log << QObject::tr("Не найден шаблон знака для станции. Станции не будут загружены");
    return false;
  }
  const puanson::proto::Puanson& punch = WeatherLoader::instance()->punchlibraryspecial()["station"];
  QString id_descr;
  for ( int i = 0, sz = punch.rule_size(); i < sz; ++i ) {
    const puanson::proto::CellRule& rule = punch.rule(i);
    if ( "dd" != rule.id().name() && "ff" != rule.id().name() ) {
      id_descr = QString::fromStdString( rule.id().name() );
    }
  }
  if ( true == id_descr.isEmpty() ) {
    error_log << QObject::tr("Шаблон знака для станции сломан. Станции не будут загружены");
    return false;
  }
  if ( false == global::createMapSettingsDir() ) {
    error_log << QObject::tr("Не удалось создать директорию настроек. Невозможно сохранить станции в кэше");
  }
  QMap< int, QString > maps;
  maps.insert( 0, global::kStationSynopCacheFile );
  maps.insert( 3, global::kStationAeroCacheFile );
  QMapIterator< int, QString > it(maps);
  while ( true == it.hasNext() ) {
    it.next();
    if ( false == loadStationLayer( doc, it.key(), it.value(), punch, id_descr ) ) {
      return false;
    }
  }
  if ( false == WeatherLoader::instance()->punchlibraryspecial().contains("airport") ) {
    error_log << QObject::tr("Не найден шаблон знака для аэропортов. Аэропорты не будут загружены");
    return false;
  }
  if ( false == WeatherLoader::instance()->punchlibraryspecial().contains("airport_enemy") ) {
    error_log << QObject::tr("Не найден шаблон знака для аэропортов противника. Аэропорты не будут загружены");
    return false;
  }
  const puanson::proto::Puanson& airportpunch = WeatherLoader::instance()->punchlibraryspecial()["airport"];
  if ( false == loadAirportLayer( doc, global::kStationAirportCacheFile, airportpunch ) ) {
    return false;
  }

  if ( false == loadAerodromeLayer( doc, global::kStationAerodromeCacheFile, airportpunch ) ) {
    return false;
  }

  if ( false == loadVO(doc) ) {
    return false;
  }

  if ( false == loadRFBorder(doc) ) {
    return false;
  }

  return true;
}
}
}
