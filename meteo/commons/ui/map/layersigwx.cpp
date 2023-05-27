#include "layersigwx.h"

#include <quuid.h>
#include <qpainter.h>
#include <qelapsedtimer.h>

#include <cross-commons/debug/tlog.h>
#include <commons/geobasis/projection.h>
#include <commons/textproto/pbtools.h>
#include <sql/psql/psqlquery.h>
#include <meteo/commons/global/global.h>

#include "map.h"
#include "document.h"
#include "object.h"
#include "geopolygon.h"
#include "geotext.h"
#include "geopixmap.h"
#include "puanson.h"
#include <meteo/commons/global/weatherloader.h>
#include "layermenu.h"
#include "wind.h"
#include "cloudwx.h"

namespace meteo {
namespace map {

namespace {

QString ornamentName( int frontype )
{
  QString str;
  switch ( frontype ) {
    case 0:
    case 1:
      break;
    case 2:
    case 3:
      str = QString("0000_warmfront");
      break;
    case 4:
    case 5:
      str = QString("0001_coldfront");
      break;
    case 6:
      str = QString("0002_occlusionfront");
      break;
    default:
      break;
  }
  return str;
}

QColor ornamentColor( int frontype )
{
  QColor clr( Qt::black );
  switch ( frontype ) {
    case 0:
    case 1:
      break;
    case 2:
    case 3:
      clr = QColor( 200, 0, 0, 255 );
      break;
    case 4:
    case 5:
      clr = QColor( 0, 50, 220, 255 );
      break;
    case 6:
      clr = QColor( 100, 50, 30, 255 );
      break;
    default:
      break;
  }
  return clr;
}

bool ornamentMirror( const QString& name )
{
  if ( "0002_occlusionfront" == name ) {
    return true;
  }
  return false;
}

Layer* createLayer( Document* d )
{
  return new LayerSigwx(d);
}
static const bool res = singleton::PtkppFormat::instance()->registerLayerHandler( LayerSigwx::Type, createLayer );
}

LayerSigwx::LayerSigwx( Document* d )
  : Layer(d)
{
  info_.set_type(kLayerSigwx);
}

LayerSigwx::~LayerSigwx()
{
}

void LayerSigwx::setCloud( const sigwx::SigCloud& c )
{
  for ( int i = 0, sz = c.cloud_size(); i < sz; ++i ) {
    CloudWx* wx = new CloudWx(this);
    wx->setProto( c.cloud(i) );
  }
}

void LayerSigwx::setTurbo( const sigwx::SigTurbulence& turbo )
{
  for ( int i = 0, sz = turbo.turb_size(); i < sz; ++i ) {
    const sigwx::Turbulence& t = turbo.turb(i);
    GeoVector gv( t.pnt_size() );
    for ( int j = 0, jsz = t.pnt_size(); j < jsz; ++j ) {
      gv[j] = pbgeopoint2geopoint( t.pnt(j) );
    }
    if ( 0 == gv.size() ) {
      continue;
    }
    GeoPolygon* poly = new GeoPolygon(this);
    poly->setPenColor(Qt::darkRed);
    QPen p = poly->qpen();
    p.setWidth(1);
    p.setStyle( Qt::CustomDashLine );
    p.setDashPattern( QVector<qreal>() << 18 << 6 );
    poly->setPen(p);
    poly->setSplineFactor(10);
    poly->setSkelet(gv);

    Puanson* puanson = new Puanson(this);
    puanson->setPunch( WeatherLoader::instance()->punchlibraryspecial()["turbo"] );
    TMeteoData md;
    md.set( "B", "", t.degree(), control::RIGHT );
    if ( true == t.has_h_lo() ) {
      md.set( "hB", "", t.h_lo(), control::RIGHT );
    }
    if ( true == t.has_h_hi() ) {
      md.set( "h_hi", "", t.h_hi(), control::RIGHT );
    }
    puanson->setMeteodata(md);
    puanson->setSkelet(gv[0]);
  }
}

void LayerSigwx::setJet( const sigwx::SigJetStream& jet )
{
  for ( int i = 0, sz = jet.jet_size(); i < sz; ++i ) {
    const sigwx::JetStream& js = jet.jet(i);
    GeoVector gv( js.pnt_size() );
    GeoPolygon* poly = new GeoPolygon(this);
    for ( int j = 0, jsz = js.pnt_size(); j < jsz; ++j ) {
      const sigwx::JetPoint& pnt = js.pnt(j);
      gv[j] = GeoPoint::fromDegree( pnt.lat(), pnt.lon() );
//      if ( true == pnt.has_ff() ) {
//        GeoPixmap* gpix = new GeoPixmap(poly);
//        gpix->setSkelet( gv[j] );
//        gpix->setBindedToSkelet(true);
//        QImage img = jetImage( pnt );
//        gpix->setImage(img);
//      }
      if ( true == pnt.has_ff() ) {
        Puanson* puanson = new Puanson(poly);
        puanson->setLenft(5.0);
        puanson->setPunch( WeatherLoader::instance()->punchlibraryspecial()["jetstream"] );
        TMeteoData md;
        md.set( "dd", "", 270, control::RIGHT );
        md.set( "ff", "", pnt.ff(), control::RIGHT );
        md.set( "fl_lev", "", pnt.fl(), control::RIGHT );
        if ( true == pnt.has_fl_blw() ) {
          md.set( "h0", "", pnt.fl_blw(), control::RIGHT );
          md.set( "h_hi", "", pnt.fl_abv(), control::RIGHT );
        }
        puanson->setMeteodata(md);
        puanson->setSkelet( gv[j] );
        puanson->setBindedToSkelet(true);
//        QImage img = jetImage( pnt );
//        gpix->setImage(img);
      }
    }
    poly->setSplineFactor(20);
    poly->setPenWidth(2);
    poly->setArrowPlace(kEndArrow);
    poly->setSkelet(gv);
  }
}

void LayerSigwx::setFront( const sigwx::SigFront& front )
{
//  debug_log << "FRONT  = " << front.Utf8DebugString();
  for ( int i = 0, sz = front.front_size(); i < sz; ++i ) {
    const sigwx::Front& f = front.front(i);
    QString frontype = ornamentName( f.phen() );
    if ( true == frontype.isEmpty() ) {
      error_log << QObject::tr("Неизвестный тип фронта = %1").arg( f.phen() );
      continue;
    }
    GeoVector gv( f.pnt_size() );
    for ( int j = 0, jsz = f.pnt_size(); j < jsz; ++j ) {
      gv[j] = GeoPoint::fromDegree( f.pnt(j).lat(), f.pnt(j).lon() );
    }
    if ( 0 == gv.size() ) {
      continue;
    }
    GeoPolygon* poly = new GeoPolygon(this);
    poly->setOrnament(frontype);
    poly->setOrnamentMirror( ornamentMirror(frontype) );
    poly->setSplineFactor(20);
    poly->setPenWidth(2);
    poly->setPenColor( ornamentColor( f.phen() ) );
    poly->setSkelet(gv);
//    debug_log << "I =" << i;
  }
}

void LayerSigwx::setTropo( const sigwx::SigTropo& tropo )
{
  Q_UNUSED(tropo);
  const puanson::proto::Puanson& punch = WeatherLoader::instance()->punchlibraryspecial()["tropo"];
  for ( int i = 0, sz = tropo.tropo_size(); i < sz; ++i ) {
    Puanson* p = new Puanson(this);
    GeoPoint gp = pbgeopoint2geopoint( tropo.tropo(i) );
    TMeteoData md;
    md.set( "level_type", "", 0.0, control::RIGHT );
    md.set( "fl_lev", "", gp.alt(), control::RIGHT );
    p->setPunch(punch);
    p->setMeteodata(md);
    p->setSkelet(gp);
  }
  for ( int i = 0, sz = tropo.tmin_size(); i < sz; ++i ) {
    Puanson* p = new Puanson(this);
    GeoPoint gp = pbgeopoint2geopoint( tropo.tmin(i) );
    TMeteoData md;
    md.set( "level_type", "", 2.0, control::RIGHT );
    md.set( "fl_lev", "", gp.alt(), control::RIGHT );
    p->setPunch(punch);
    p->setMeteodata(md);
    p->setSkelet(gp);
  }
  for ( int i = 0, sz = tropo.tmax_size(); i < sz; ++i ) {
    Puanson* p = new Puanson(this);
    GeoPoint gp = pbgeopoint2geopoint( tropo.tmax(i) );
    TMeteoData md;
    md.set( "level_type", "", 1.0, control::RIGHT );
    md.set( "fl_lev", "", gp.alt(), control::RIGHT );
    p->setPunch(punch);
    p->setMeteodata(md);
    p->setSkelet(gp);
  }
//  debug_log << "TROPO =" << tropo.Utf8DebugString();
}

void LayerSigwx::setVulkan( const sigwx::SigVolcano& vulkan )
{
  for ( int i = 0, sz = vulkan.volcano_size(); i < sz; ++i ) {
    const sigwx::Volcano& v = vulkan.volcano(i);
    GeoPoint gp = pbgeopoint2geopoint( v.pnt() );
    Puanson* p = new Puanson(this);
    p->setPunch( WeatherLoader::instance()->punchlibraryspecial()["vulkan"] );
    TMeteoData md;
    md.set( "h_v", "", 1, control::RIGHT );
    md.set( "station", pbtools::toQString( v.name() ), 0, control::RIGHT );
    md.set( "La", "", gp.latDeg(), control::RIGHT );
    md.set( "Lo", "", gp.lonDeg(), control::RIGHT );
    p->setMeteodata(md);
    p->setSkelet(gp);
  }
}

void LayerSigwx::setStorm( const sigwx::SigStorm& storm )
{
  for ( int i = 0, sz = storm.storm_size(); i < sz; ++i ) {
    const sigwx::Storm& s = storm.storm(i);
    GeoPoint gp = pbgeopoint2geopoint( s.pnt() );
    Puanson* p = new Puanson(this);
    p->setPunch( WeatherLoader::instance()->punchlibraryspecial()["storm"] );
    TMeteoData md;
    md.set( "storm", "", 1, control::RIGHT );
    md.set( "storm_name", pbtools::toQString( s.name() ), 0, control::RIGHT );
    p->setMeteodata(md);
    p->setSkelet(gp);
  }
}

void LayerSigwx::setData( const sigwx::SigWx& data )
{
//  debug_log << "FRONT SIZE =" << data.front_size();
//  for ( int i = 0, sz = data.front_size(); i < sz; ++i ) {
//    debug_log << "FRONT SIZE =" << data.front(i).front_size();
//  }
//  debug_log << "CLOUD SIZE =" << data.cloud_size();
//  for ( int i = 0, sz = data.cloud_size(); i < sz; ++i ) {
//    debug_log << "CLOUD SIZE =" << data.cloud(i).cloud_size();
//  }
//  debug_log << "TURB  SIZE =" << data.turb_size();
//  for ( int i = 0, sz = data.turb_size(); i < sz; ++i ) {
//    debug_log << "TURB SIZE =" << data.turb(i).turb_size();
//  }
//  debug_log << "JET   SIZE =" << data.jet_size();
//  for ( int i = 0, sz = data.jet_size(); i < sz; ++i ) {
//    debug_log << "JET SIZE =" << data.jet(i).jet_size();
//  }
//  debug_log << "TROPO SIZE =" << data.tropo_size();
//  for ( int i = 0, sz = data.tropo_size(); i < sz; ++i ) {
//    debug_log << "TROPO SIZE =" << data.tropo(i).tropo_size();
//  }
//  debug_log << "VOLC  SIZE =" << data.volcano_size();
//  for ( int i = 0, sz = data.volcano_size(); i < sz; ++i ) {
//    debug_log << "VOLC SIZE =" << data.volcano(i).volcano_size();
//  }
//  debug_log << "STORM SIZE =" << data.storm_size();
//  for ( int i = 0, sz = data.storm_size(); i < sz; ++i ) {
//    debug_log << "STORM SIZE =" << data.storm(i).storm_size();
//  }
  for ( int i = 0, sz = data.front_size(); i < sz; ++i ) {
    setFront( data.front(i) );
  }
  for ( int i = 0, sz = data.cloud_size(); i < sz; ++i ) {
    setCloud( data.cloud(i) );
  }
  for ( int i = 0, sz = data.turb_size(); i < sz; ++i ) {
    setTurbo( data.turb(i) );
  }
  for ( int i = 0, sz = data.jet_size(); i < sz; ++i ) {
    setJet( data.jet(i) );
  }
  for ( int i = 0, sz = data.tropo_size(); i < sz; ++i ) {
    setTropo( data.tropo(i) );
  }
  for ( int i = 0, sz = data.volcano_size(); i < sz; ++i ) {
    setVulkan( data.volcano(i) );
  }
  for ( int i = 0, sz = data.storm_size(); i < sz; ++i ) {
    setStorm( data.storm(i) );
  }
}

int32_t LayerSigwx::dataSize() const
{
  int32_t pos = Layer::dataSize();
  return pos;
}

int32_t LayerSigwx::serializeToArray( char* arr ) const
{
  int32_t pos = Layer::serializeToArray(arr);
  return pos;
}

int32_t LayerSigwx::parseFromArray( const char* arr )
{
  int32_t pos = Layer::parseFromArray(arr);
  return pos;
}

}
}
