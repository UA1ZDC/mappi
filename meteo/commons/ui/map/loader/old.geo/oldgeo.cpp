#include "oldgeo.h"

#include <qfile.h>
#include <qprocess.h>
#include <qdir.h>
#include <qvariant.h>
#include <qsettings.h>
#include <qfileinfo.h>
#include <qtextcodec.h>
#include <qpair.h>

#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>
#include <commons/geobasis/geopoint.h>
#include <commons/geobasis/geovector.h>
#include <commons/geobasis/projection.h>
#include <meteo/commons/ui/map/geopolygon.h>
#include <meteo/commons/ui/map/document.h>
#include <meteo/commons/ui/map/layer.h>
#include <meteo/commons/ui/map/geotext.h>
#include <meteo/commons/ui/map/geopolygon.h>
#include <meteo/commons/ui/map/loader.h>
#include <meteo/commons/global/common.h>
#include <meteo/commons/fonts/weatherfont.h>
#include <commons/textproto/tprototext.h>
#include <meteo/commons/proto/map_isoline.pb.h>
#include <meteo/commons/global/global.h>

namespace {
  const QString kLoaderType("geo.old");

  static bool res = meteo::map::Loader::instance()->registerDataHandler(kLoaderType, &meteo::map::oldgeo::loadGeoData );
}

namespace meteo {
namespace map {
namespace oldgeo {

QMap<QString, meteo::Property> geocolors;
QMap<QString, meteo::map::proto::CitySettings> citymap;

const QString kMapSettingsFile( MnCommon::etcPath("meteo") + "/map.settings.conf" );

meteo::map::proto::MapSettings mapSettingsOld()
{
  meteo::map::proto::MapSettings setts;
  QFile file(kMapSettingsFile);
  if ( false == file.open( QIODevice::ReadOnly ) ) {
    error_log << QObject::tr("Не могу открыть файл %1 для чтения")
      .arg(kMapSettingsFile);
    return setts;
  }
  QString str = QString::fromLocal8Bit( file.readAll() );
  meteo::map::proto::MapSettingsConfig mcfg;
  if ( false == TProtoText::fillProto( str, &mcfg) ) {
    error_log << QObject::tr("Не удалось загрузить настройки из файла %1")
      .arg(kMapSettingsFile);
  }
  setts.CopyFrom(mcfg.settings());
  return setts;
}

namespace internal {

class GeoRecord
{
  public:
    GeoRecord();

    int16_t index;
    int16_t map;
    int16_t bot;
    int16_t top;
    int32_t type;
    int16_t count;
    int16_t tlength;
    bool readFromFile( QFile* file );
    bool writeToFile( QFile* file ) const ;

  private:
    int size() const;
};

GeoRecord::GeoRecord() {
  index = 0;
  map = 0;
  bot = 0;
  top = 0;
  type = 0;
  count = 0;
  tlength = 0;
}

int GeoRecord::size() const {
  int s = 0;
  s += sizeof( index );
  s += sizeof( map );
  s += sizeof( bot );
  s += sizeof( top );
  s += sizeof( type );
  s += sizeof( count );
  s += sizeof( tlength );
  return s;
}


bool GeoRecord::readFromFile( QFile* file )
{
  int rsize = size();
  QByteArray arr( rsize, 0 );
  if ( rsize != file->read( arr.data(), rsize ) ) {
    error_log << QObject::tr("Не удалось прочитать геоданные из файла %1").arg(file->fileName());
    return false;
  }
  int shift = 0;
  memcpy( &index, &(arr.data()[shift]), sizeof(index) );
  shift += sizeof(index);
  memcpy( &map, &(arr.data()[shift]), sizeof(map) );
  shift += sizeof(map);
  memcpy( &bot, &(arr.data()[shift]), sizeof(bot) );
  shift += sizeof(bot);
  memcpy( &top, &(arr.data()[shift]), sizeof(top) );
  shift += sizeof(top);
  memcpy( &type, &(arr.data()[shift]), sizeof(type) );
  shift += sizeof(type);
  memcpy( &count, &(arr.data()[shift]), sizeof(count) );
  shift += sizeof(count);
  memcpy( &tlength, &(arr.data()[shift]), sizeof(tlength) );

  return true;
}

bool GeoRecord::writeToFile( QFile* file ) const
{
  int rsize = size();
  QByteArray arr( rsize, 0 );
  int shift = 0;
  memcpy( &(arr.data()[shift]), &index, sizeof(index) );
  shift += sizeof(index);
  memcpy( &(arr.data()[shift]), &map, sizeof(map) );
  shift += sizeof(map);
  memcpy( &(arr.data()[shift]), &bot, sizeof(bot) );
  shift += sizeof(bot);
  memcpy( &(arr.data()[shift]), &top, sizeof(top) );
  shift += sizeof(top);
  memcpy( &(arr.data()[shift]), &type, sizeof(type) );
  shift += sizeof(type);
  memcpy( &(arr.data()[shift]), &count, sizeof(count) );
  shift += sizeof(count);
  memcpy( &(arr.data()[shift]), &tlength, sizeof(tlength) );
  if ( rsize != file->write( arr.data(), rsize ) ) {
    error_log << QObject::tr("Не удалось записать файл");
    return false;
  }

  return true;
}

enum GeoDataSpec{
  UNKNOWNDATASPEC = -1,
  COAST       =         31110000,
  COAST2      =         32220000,
  LAKE        =         31120000,
  DELIM       =         81110000,
  STRIKE      =         31110011,
  ISLAND31    =         31110001,
  ISLAND34    =         34000000,
  ISLAND,
  HYDRO       =         31410000,
  STATION     =         41110000,
  STAT_HYDRO  =         41110001,
  CITY        =         41100000,
  RIVER,
  BORDER_MAP  =         60000002,
  STRIKE_BORDER =       60000002,
  DATATYPES_COUNT
};

void createDefaultColorSettings( const QString& filename )
{
  QMap< QString, meteo::Property > setts;

  meteo::Property prop;
  QPen pen;
  pen.setColor(kMAP_COAST_COLOR);
  pen.setWidth(1);
  QBrush brush;
  brush.setStyle( Qt::SolidPattern );
  brush.setColor(kMAP_SEA_COLOR);
  prop.set_closed(true);
  Generalization g;
  g.setLimits(0,30);
  prop.mutable_pen()->CopyFrom( qpen2pen(pen) );
  prop.mutable_brush()->CopyFrom( qbrush2brush(brush) );
  prop.mutable_general()->CopyFrom( g.proto() );
  prop.set_name( QString::number(COAST).toStdString() );
  setts.insert( QString::number(COAST), prop );

  g.setHighLimit(16);
  prop.mutable_general()->CopyFrom( g.proto() );
  prop.set_name( QString::number(LAKE).toStdString() );
  setts.insert( QString::number(LAKE), prop );

  brush.setColor(kMAP_ISLAND_COLOR);
  prop.mutable_brush()->CopyFrom( qbrush2brush(brush) );
  g.setHighLimit(30);
  prop.mutable_general()->CopyFrom( g.proto() );
  prop.set_name( QString::number(ISLAND31).toStdString() );
  setts.insert( QString::number(ISLAND31), prop );

  prop.set_name( QString::number(ISLAND34).toStdString() );
  setts.insert( QString::number(ISLAND34), prop );

  prop.set_closed(false);
  brush.setStyle( Qt::NoBrush );
  prop.mutable_brush()->CopyFrom( qbrush2brush(brush) );
  pen.setColor(kMAP_HYDRO_COLOR);
  prop.mutable_pen()->CopyFrom( qpen2pen(pen) );
  g.setHighLimit(14);
  prop.mutable_general()->CopyFrom( g.proto() );
  prop.set_name( QString::number(HYDRO).toStdString() );
  setts.insert( QString::number(HYDRO), prop );

  pen.setColor(kMAP_COAST_COLOR);
  prop.mutable_pen()->CopyFrom( qpen2pen(pen) );
  prop.set_closed(true);
  g.setHighLimit(30);
  prop.mutable_general()->CopyFrom(g.proto());
  prop.set_name( QString::number(BORDER_MAP).toStdString() );
  setts.insert( QString::number(BORDER_MAP), prop );

  pen = QPen();
  pen.setWidth(1);
  pen.setColor(kMAP_DELIM_COLOR);
  prop.mutable_pen()->CopyFrom( qpen2pen(pen) );
  prop.set_closed(false);
  g.setHighLimit(14);
  prop.mutable_general()->CopyFrom( g.proto() );
  prop.set_name( QString::number(DELIM).toStdString() );
  setts.insert( QString::number(DELIM), prop );

  pen.setColor(kMAP_SEA_COLOR);
  pen.setWidth(2);
  prop.mutable_pen()->CopyFrom( qpen2pen(pen) );
  prop.set_closed(false);
  g.setHighLimit(30);
  prop.mutable_general()->CopyFrom(g.proto());
  prop.set_name( QString::number(STRIKE).toStdString() );
  setts.insert( QString::number(STRIKE), prop );
  prop.set_name( QString::number(STRIKE_BORDER).toStdString() );
  setts.insert( QString::number(STRIKE_BORDER), prop );

  prop = meteo::Property();
  pen = QPen();
  pen.setWidth(1);
  prop.mutable_pen()->CopyFrom( qpen2pen(pen) );
  brush = QBrush();
  brush.setColor(kMAP_ISLAND_COLOR);
  prop.mutable_brush()->CopyFrom( qbrush2brush(brush) );
  prop.set_name( kMAPCOLORSETTINGS_IDENT.toStdString() );
  setts.insert( kMAPCOLORSETTINGS_IDENT, prop );
  meteo::Properties prots;
  QMapIterator<QString,meteo::Property> it(setts);
  while ( true == it.hasNext() ) {
    it.next();
    meteo::Property* p = prots.add_property();
    p->CopyFrom( it.value() );
  }
  TProtoText::toFile( prots, filename );
}

QMap< QString, meteo::Property > loadProperties()
{
  if ( false == global::createMapSettingsDir() ) {
    error_log << QObject::tr("Не удалось создать директорию настроек. Свойства картографической основы невозможно загрузить");
  }
  QFile file(global::kMapColorSettingsFilename);
  if ( false == file.exists() ) {
    createDefaultColorSettings( file.fileName() );
  }
  QMap< QString, meteo::Property > mapcolors = mapSettings();
  return mapcolors;
}

QMap<QString, meteo::map::proto::CitySettings> loadCitiesSettings(){
  QMap<QString, meteo::map::proto::CitySettings> cities;
  meteo::map::proto::MapSettings settings = mapSettingsOld();
  for(int i = 0; i < settings.city_settings_size(); i++){
    meteo::map::proto::CitySettings city = settings.city_settings(i);
    cities.insert(QString::fromStdString(city.title()), city);
  }
  return cities;
}

bool isOddMap( int map, Projection* proj )
{
  switch ( proj->type() ) {
    case STEREO:
      if ( Northern == proj->hemisphere() && 40 < map ) {
        return false;
      }
      else if ( Southern == proj->hemisphere() && ( 31 > map || 61 < map ) ) {
        return false;
      }
      break;
    case POLARORBSAT:
      return true;
      break;
    default:
      if ( 0 == map || 61 == map ) {
        return false;
      }
  }
  return true;
}

struct GP
{
  double lat;
  double lon;
} gp;

bool loadGeoPolygonObject( Layer* layer, const GeoRecord& rcrd, QFile* file )
{
  GeoPoint pnt;
  int pntsize = sizeof( gp.lat ) + sizeof( gp.lon );
  int arrsize = pntsize*rcrd.count;
  QVector<GP> vector( rcrd.count );
  if ( arrsize != file->read( reinterpret_cast<char*>(vector.data()), arrsize ) ) {
    error_log << QObject::tr("Не удалось прочитать данные для GeoVector из файла %1")
      .arg( file->fileName() );
    return false;
  }

  switch ( rcrd.type ) { //загружать только необходимое
    case COAST:
    case STRIKE:
    case STRIKE_BORDER:
    case LAKE:
    case DELIM:
    case ISLAND:
    case ISLAND31:
    case ISLAND34:
    case HYDRO:
    case RIVER:
      break;
    default:
      return true;
  }

  if ( false == isOddMap( rcrd.map, layer->projection() ) ) { //загружать только необходимое
    return true;
  }
  GeoVector gv(rcrd.count);
  bool fl = false;
  if ( ( DELIM == rcrd.type || RIVER == rcrd.type || LAKE == rcrd.type) && 0 != vector.size()  ) { //некорректные полигоны отбрасываются
    fl = true;
    for ( int i = 0, sz = vector.size(); i < sz; ++i ) {
      if ( 60.0 != vector[i].lat ) {
        fl = false;
        break;
      }
    }
  }
  if ( true == fl ) {
    return true;
  }
  fl = false;
  if ( LAKE == rcrd.type && 1996 == rcrd.index ) {
    return true;
  }

  for ( int i = 0, sz = vector.size(); i < sz; ++i ) {
    gv[i].setLatDeg(vector[i].lat);
    gv[i].setLonDeg(vector[i].lon);
  }

  //сначала установить свойства, затем установить вектор координат, потому что векторы прчмоугольных координат зависят от свойств (Property::closed)
  GeoPolygon* geo = new GeoPolygon( layer );
  if ( true == geocolors.contains( QString::number(rcrd.type) ) ) {
    geo->setProperty(geocolors[ QString::number(rcrd.type) ]);
  }
  if ( true == layer->document()->property().opengeopolygon() ) {
    geo->setClosed(false);
  }
  geo->setSkelet(gv);

  return true;
}

bool loadCityObject( Layer* layer, const GeoRecord& rcrd, QFile* file, QTextCodec* cdc )
{
  QByteArray arrname( rcrd.tlength+1, '\0' );
  if ( rcrd.tlength+1 != file->read( arrname.data(), rcrd.tlength+1 ) ) {
    warning_log << QObject::tr("Ошибка чтения населенного пункта");
    return false;
  }
  arrname = arrname.left( arrname.size() - 1 );
  QString name;
  if ( 0 == cdc ) {
    name = QObject::tr( arrname.data());
  }
  else {
    name = cdc->toUnicode(arrname);
  }
  GP gp;
  int pntsize = sizeof( gp.lat ) + sizeof( gp.lon );
  if ( pntsize != file->read( reinterpret_cast<char*>(&gp), pntsize ) ) {
    error_log << QObject::tr("Не удалось прочитать данные для GeoPoint из файла %1")
      .arg( file->fileName() );
    return false;
  }
  GeoPoint pnt = GeoPoint::fromDegree(  gp.lat, gp.lon );

  GeoVector gv(1);
  gv[0] = pnt;

  //сначала установить свойства, затем установить вектор координат, потому что векторы прчмоугольных координат зависят от свойств (Property::closed)
  GeoText* geo = new GeoText(layer);
  geo->setVisible(citymap[name].visible());
  Generalization g;
  g.setLimits(0,citymap[name].priority());
  geo->setGeneral(g);
  geo->setSkelet(gv);
  geo->setText(name);
  geo->setFont(meteo::WeatherFont::instance()->kFontHelvetica);
  return true;
}

bool loadGeoPolygonLayer( Document* doc, const QPair< QString, QString >& layername )
{
  QFile file( layername.second );
  if ( false == file.exists() ) {
    error_log << QObject::tr("Файл с геоданными %1 для слоя %2 не найден")
      .arg(layername.second)
      .arg(layername.first);
    return false;
  }
  if ( false == file.open( QIODevice::ReadOnly ) ) {
    error_log << QObject::tr("Не удалось открыть файл %1").arg(layername.second);
    return false;
  }
  Layer* layer = new Layer( doc, layername.first );
  layer->setBase(true);
  layer->setCache(true);
  GeoRecord rcrd;
  while ( false == file.atEnd() && true == rcrd.readFromFile(&file) ) {
    loadGeoPolygonObject( layer, rcrd, &file );
  }
  if ( 0 == layer->size() ) {
    error_log << QObject::tr("Не загружен ни один объект для слоя %1 из файла %2")
      .arg(layername.first)
      .arg(layername.second);
    return false;
  }
  return true;
}

bool loadCities( Document* doc, const QString& name, const QString& filename )
{
  QTextCodec* cdc = QTextCodec::codecForName("koi8r");
  QFile file(filename);
  if ( false == file.exists() ) {
    error_log << QObject::tr("Файл с геоданными %1 для слоя %2 не найден")
      .arg(filename)
      .arg(name);
    return false;
  }
  if ( false == file.open( QIODevice::ReadOnly ) ) {
    error_log << QObject::tr("Не удалось открыть файл %1")
      .arg(name);
    return false;
  }
  Layer* layer = new Layer( doc, name );
  GeoRecord rcrd;
  while ( false == file.atEnd() && true == rcrd.readFromFile(&file) ) {
    loadCityObject( layer, rcrd, &file, cdc );
  }
  if ( 0 == layer->size() ) {
    error_log << QObject::tr("Не загружен ни один объект для слоя %1 из файла %2")
      .arg(name)
      .arg(filename);
    return false;
  }
  return true;
}

}

int layerstype( Projection* proj )
{
  if ( MERCAT == proj->type() ) {
    return 0;
  }
  else if ( STEREO == proj->type() ) {
    if ( Northern == proj->hemisphere() ) {
      return 1;
    }
    return 2;
  }
  else if ( CONICH == proj->type() ) {
    if ( Northern == proj->hemisphere() ) {
      return 3;
    }
    return 4;
  }
  return -1;
}

bool loadGeoData( Document* doc )
{
  geocolors = internal::loadProperties();
  citymap = internal::loadCitiesSettings();
  if ( 0 == doc ) {
    error_log << QObject::tr("Нулевой указатель");
    return false;
  }
  QString path = MnCommon::sharePath("meteo") + "/geo/";
  QList< QPair< QString, QString > > layernames = QList< QPair< QString, QString > >()
    << qMakePair( QObject::tr("Озера"),             QString(path + "lakes.geo")      )
    << qMakePair( QObject::tr("Реки"),              QString(path + "hydroline.geo")  )
    << qMakePair( QObject::tr("Береговая черта"),   QString(path + "coastline.geo")  )
    << qMakePair( QObject::tr("Границы"),           QString(path + "delimiters.geo") )
  ;
  for ( int i = 0, sz = layernames.size(); i < sz; ++i ) {
    if ( false == internal::loadGeoPolygonLayer( doc, layernames[i] ) ) {
      error_log << QObject::tr("Не удалось добавить слой %1:%2 на карту ")
        .arg( layernames[i].first )
        .arg( layernames[i].second );
    }
  }
  /*if ( false == internal::loadCities( map, "Населенные пункты", path + "vcity.geo" ) ) {
    error_log << QObject::tr("Не удалось добавить слой c населенными пунктами на карту");
  }*/
  return true;
}

}
}
}
