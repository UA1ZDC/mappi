#include "layermrl.h"

#include <qdebug.h>
#include <quuid.h>
#include <qpainter.h>

#include <meteo/commons/global/dateformat.h>
#include <cross-commons/debug/tlog.h>
#include <sql/psql/psqlquery.h>
#include <commons/geobasis/projection.h>
#include <meteo/commons/global/global.h>

#include "map.h"
#include "document.h"
#include "object.h"
#include "geomrl.h"
#include "geotext.h"
#include "puanson.h"
#include "geopolygon.h"
#include "event.h"
#include <meteo/commons/global/weatherloader.h>
#include "puanson.h"
#include <meteo/commons/global/radarparams.h>

namespace {
  const float kRad100km = 100000.0/meteo::kEarthRadius;
  const float kRad200km = 200000.0/meteo::kEarthRadius;
}

namespace meteo {
namespace map {

namespace {
  Layer* createLayer( Document* d )
  {
    return new LayerMrl(d);
  }
  static const bool res = singleton::PtkppFormat::instance()->registerLayerHandler( LayerMrl::Type, createLayer );
}

LayerMrl::LayerMrl ( Document* d, const QString& n )
  : Layer ( d, n )
{
  info_.set_type(kLayerMrl);
}
    
LayerMrl::LayerMrl( Document* map, const proto::RadarColor& clr )
  : Layer(map),
  radarcolor_(clr)
{
  info_.set_type(kLayerMrl);
}
    
LayerMrl::LayerMrl( Document* map, const proto::RadarColor& clr, const QList<Radar>& r )
  : Layer(map),
  radarcolor_(clr)
{
  info_.set_type(kLayerMrl);
  setRadar(r);
}

LayerMrl::~LayerMrl()
{
}

int LayerMrl::descriptor() const
{
  if ( false == radarcolor_.has_descr() ) {
    return -1;
  }
  return radarcolor_.descr();
}

void LayerMrl::setRadar( const QList<Radar>& r )
{
  std::unordered_set<GeoMrl*> list = objectsByType<GeoMrl*>(true);
  foreach(Object* o, list){
    delete o;
  }
  for ( int i = 0, sz = r.size(); i < sz; ++i ) {
    /*GeoMrl* mrl =*/ new GeoMrl( this, r[i], radarcolor_ );
  }
}

void LayerMrl::setProtoRadar( const surf::ManyMrlValueReply& proto )
{
  QList<Radar> rlist;
  for ( int i = 0, sz = proto.values_size(); i < sz; ++i ) {
    const surf::OneMrlValueReply& mrl = proto.values(i);
    Radar r;
    r.setEmpty( radarcolor_.empty_value() );
    r.setProto(mrl);
    rlist.append(r);
  }
  setRadar(rlist);
  for ( int i = 0, sz = proto.values_size(); i < sz; ++i ) {
    const surf::OneMrlValueReply& mrl = proto.values(i);
    buildMrlPrimitives(mrl);
  }
}

float LayerMrl::fieldValue( const GeoPoint& gp, bool* ok ) const
{
  if ( 0 != ok ) {
    *ok = false;
  }
  float val = -9999.0;
  if ( 0 == document() ) {
    return val;
  }
  QPoint pnt = document()->coord2screen(gp);
  bool locok = false;
  std::unordered_set<GeoMrl*> olist = objectsByType<GeoMrl*>(true);
  foreach(GeoMrl* o, olist){
    val = o->pointValue( pnt, &locok );
    if ( true == locok ) {
      if ( 0 != ok ) {
        *ok = true;
      }
      if ( true == hasPunchRule() ) {
        lastvaluestr_.clear();
        puanson::proto::CellRule rule = punchrule();
        for ( int k = 0, ksz = rule.symbol().symbol_size(); k < ksz; ++k ) {
          const puanson::proto::CellRule::Symbol& smb = rule.symbol().symbol(k);
          if ( smb.minval() <= val && smb.maxval() >= val ) {
            if ( true == smb.has_label() ) {
              lastvaluestr_ = QString::fromStdString( smb.label() );
              lastvaluestr_.replace( "@chcm@", "Ch и Cm" );
            }
            break;
          }
        }
      }
      return val;
    }
  }
  return val;
}

QString LayerMrl::layerNamePattern()
{
  return QString("[template] за $dd.MM.yy hh:mm$ [layer] Центр: [center]");
}

//proto::WeatherLayer LayerMrl::info( LayerMrl* l )
//{
//  Q_UNUSED(l);
//  proto::WeatherLayer i;
//  return i;
//}

void LayerMrl::buildMrlPrimitives( const surf::OneMrlValueReply& mrl )
{
  meteo::GeoPoint gp = GeoPoint::fromDegree( mrl.center().fi(), mrl.center().la() );
  createCircle( gp, kRad100km, QObject::tr("100 км") );
  createCircle( gp, kRad200km, QObject::tr("200 км") );
  createSpeedDirection( gp, mrl.synopspeed(), mrl.synopdirection() );
}

void LayerMrl::createCircle(const meteo::GeoPoint& center, float rad, const QString& podpis )
{
  Q_UNUSED(podpis);
  meteo::map::GeoPolygon* gp = new GeoPolygon(this);
  meteo::GeoVector gv;
  for ( int i = 0; i < 365; i += 5 ) {
    meteo::GeoPoint gp = center.findSecondCoord( rad, meteo::DEG2RAD*i );
    gv.append(gp);
  }
  gp->setPenStyle( Qt::DotLine );
  gp->setClosed(true);
  gp->setSkelet(gv);
//  meteo::map::GeoText* txt = new meteo::map::GeoText(gp);
//  txt->setText(podpis);
//  txt->setPenStyle( Qt::SolidLine );
//  txt->setPos( proto::kCenter );
//  txt->setPosOnParent( proto::kBottomCenter );
}
  
void LayerMrl::createSpeedDirection( const GeoPoint& gp, float speed, float direct )
{
  if ( MnMath::isEqual( 9999, speed ) || MnMath::isEqual( -9999, speed ) ) {
    return;
  }
  if ( MnMath::isEqual( 9999, direct ) || MnMath::isEqual( -9999, direct ) ) {
    return;
  }
  if ( false == WeatherLoader::instance()->punchlibraryspecial().contains("radar_speed_direct") ) {
    return;
  }
  const puanson::proto::Puanson& punch = WeatherLoader::instance()->punchlibraryspecial()["radar_speed_direct"];
  Puanson* puanson = new Puanson(this);
  TMeteoData data;
  TMeteoParam dndn( "", direct, control::RIGHT );
  data.add( "dndn", dndn );
  TMeteoParam cncn( "", speed, control::RIGHT );
  data.add( "cncn", cncn );
  puanson->setSkelet(gp);
  puanson->setMeteodata(data);
  puanson->setPunch(punch);
}
/*
valLabel LayerMrl::valueLabel() const { 
  valLabel sss;
  sss.name = QString::fromStdString(radarcolor_.name() );
  return sss;
}
*/
valLabel LayerMrl::valueLabel() const
{
  valLabel sss;
  if(0 == info_.h1() && 0 == info_.h2() ){
    //FIXME sss.level = QObject::tr("у земли");
  } else {
    if(0 != info_.h1() && 0 == info_.h2() ){
      sss.level = QObject::tr("на высоте %1 м").arg(info_.h1());
    } else { 
      if( 0 != info_.h2() ){
      sss.level = QObject::tr("в слое %1 - %2 м").arg(info_.h1()).arg(info_.h2());
      }
    }
  }
  QString code = QString::fromStdString( info_.template_name() );
  if ( true == WeatherLoader::instance()->radarlibrary().contains(code) ) {
    sss.name = QString::fromStdString( WeatherLoader::instance()->radarlibrary()[code].name() );
  } else {
    sss.name = TMeteoDescriptor::instance()->property(descriptor()).description;
  }
  sss.unit = TMeteoDescriptor::instance()->property(descriptor()).unitsRu;
  QDateTime dt = PsqlQuery::datetimeFromString( QString::fromStdString( info_.datetime() ) );
  if ( true == dt.isValid() ) {
    sss.date = meteo::dateToHumanTimeShort(dt);
  }
/*  if ( false == lastvaluestr_.isEmpty() ) {
    sss.unit = lastvaluestr_;
  }*/
  if ( false == lastvaluestr_.isEmpty() ) {
    sss.val = lastvaluestr_;
    sss.unit = QString();
  }
  //FIXME Нужно сюда данные о станции
  if(info_.has_center_name()){
    sss.center  = QObject::tr("МРЛ") ;
  }
  
  
  return sss;
}

bool LayerMrl::hasPunchRule() const
{
  if ( true == radarcolor_.has_punch_id() && 0 != radarcolor_.punch_id().size() ) {
    return true;
  }
  return false;
}

puanson::proto::CellRule LayerMrl::punchrule() const
{
  puanson::proto::CellRule rule;
  QString punchid = QString::fromStdString( radarcolor_.punch_id() );
  if ( WeatherLoader::instance()->punchlibraryspecial().contains( punchid ) ) {
    const puanson::proto::Puanson& p = WeatherLoader::instance()->punchlibraryspecial()[punchid];
    if ( 0 != p.rule_size() ) {
      rule.CopyFrom( p.rule(0) );
    }
  }
  return rule;
}

int32_t LayerMrl::dataSize() const
{
  int32_t pos = Layer::dataSize();
  std::string str;
  if ( true == radarcolor_.IsInitialized() ) {
    radarcolor_.SerializeToString( &str );
  }
  int32_t sz = str.size();
  pos += sizeof(sz);
  pos += sz;
  return pos;
}

int32_t LayerMrl::serializeToArray( char* arr ) const
{
  int32_t pos = Layer::serializeToArray(arr);
  std::string str;
  if ( true == radarcolor_.IsInitialized() ) {
    radarcolor_.SerializeToString( &str );
  }
  int32_t sz = str.size();
  ::memcpy( arr + pos, &sz, sizeof(sz) );
  pos += sizeof(sz);
  ::memcpy( arr + pos, str.data(), sz );
  pos += sz;
  return pos;
}

int32_t LayerMrl::parseFromArray( const char* arr )
{
  int32_t pos = Layer::parseFromArray(arr);
  int32_t sz;
  global::fromByteArray( arr + pos, &sz );
  pos += sizeof(sz);
  if ( 0 != sz ) {
    radarcolor_.ParseFromArray( arr + pos, sz );
  }
  pos += sz;
  return pos;
}

}
}
