#include "cloudwx.h"

#include <qpainter.h>
#include <qdebug.h>
#include <qmath.h>

#include <commons/obanal/interpolorder.h>
#include <commons/mathtools/mnmath.h>
#include <commons/geom/geom.h>
#include <cross-commons/debug/tlog.h>

#include "layer.h"
#include "geotext.h"
#include "geopolygon.h"
#include "map.h"
#include "document.h"
#include "simple.h"
#include "ornament.h"
#include "puanson.h"
#include <meteo/commons/global/weatherloader.h>

namespace meteo {
namespace map {

namespace {
Object* createCloudWx( Layer* l )
{
  return new CloudWx(l);
}
static const bool res = singleton::PtkppFormat::instance()->registerObjectHandler( CloudWx::Type, createCloudWx );
Object* createCloudWx2( Object* o )
{
  return new CloudWx(o);
}
static const bool res2 = singleton::PtkppFormat::instance()->registerChildHandler( CloudWx::Type, createCloudWx2 );
}

CloudWx::CloudWx( Layer* l )
  : Object(l)
{
}

CloudWx::CloudWx( Object* p )
  : Object(p)
{
}

CloudWx::CloudWx( Projection* p )
  : Object(p)
{
}

CloudWx::CloudWx( const meteo::Property& p )
  : Object(p)
{
}

CloudWx::~CloudWx()
{
}

Object* CloudWx::copy( Layer* l ) const
{
  if ( 0 == l ) {
    error_log << QObject::tr("Нулевой указатель на слой");
    return 0;
  }
  CloudWx* gp = new CloudWx(l);
  gp->setProperty(property_);
  gp->setSkelet(skelet());
  foreach(Object* o, objects_){
    o->copy(gp);
  }
  return gp;
}

Object* CloudWx::copy( Object* o ) const
{
  if ( 0 == o ) {
    error_log << QObject::tr("Нулевой указатель на объект");
    return 0;
  }
  CloudWx* gp = new CloudWx(o);
  gp->setProperty(property_);
  gp->setSkelet(skelet());
  foreach(Object* o, objects_){
    o->copy(gp);
  }
  return gp;
}

Object* CloudWx::copy( Projection* proj ) const
{
  if ( 0 == proj ) {
    error_log << QObject::tr("Нулевой указатель на проекцию");
    return 0;
  }
  CloudWx* gp = new CloudWx(proj);
  gp->setProperty(property_);
  gp->setSkelet(skelet());
  foreach(Object* o, objects_){
    o->copy(gp);
  }
  return gp;
}

QList<QRect> CloudWx::boundingRect( const QTransform& transform ) const
{
  const std::unordered_set<Object*>& list = childs();
  QList<QRect> rlist;
  for ( auto o : list ) {
    rlist.append( o->boundingRect(transform) );
  }
  return rlist;
}


QList<GeoVector> CloudWx::skeletInRect( const QRect& rect, const QTransform& transform ) const
{
  QList<GeoVector> gvlist;
  Projection* proj = projection();
  if ( 0 == proj ) {
    return gvlist;
  }
  QList<QPoint> list;
  for ( int i = 0, isz = cartesian_points_.size(); i < isz; ++i ) {
    QPolygon screen = transform.map(cartesian_points_[i]);
    if ( 2 > screen.size() ) {
      continue;
    }
    bool need_draw = false;
    QRect screenrect = screen.boundingRect();
    bool screenrect_contains_target_whole = screenrect.contains( rect, true );
    if ( false == screenrect_contains_target_whole && true == rect.intersects(screenrect) ) {
      need_draw = true;
    }
    if ( false == need_draw && false == screenrect_contains_target_whole && true == screenrect.intersects(rect) ) {
      need_draw = true;
    }
    if ( false == need_draw && true == rect.contains(screenrect) ) {
      need_draw = true;
    }
    if ( false == need_draw ) {
      QPainterPath path;
      path.addPolygon(screen);
      bool screenpathcontains_target = path.contains(rect);
      QBrush br = qbrush();
      if ( false == screenpathcontains_target && true == path.intersects(rect) ) {
        need_draw = true;
      }
      else if ( Qt::NoBrush != br.style() && true == closed() ) {
        if ( true == screenpathcontains_target ) {
          need_draw = true;
        }
      }
    }
    if ( true == need_draw ) {
      gvlist.append(skelet_);
      return gvlist;
    }
  }
  return gvlist;
}


void CloudWx::setSkelet( const GeoVector& skel )
{
  Layer* l = layer();
  QList<QRect> oldlist;
  if ( 0 != skelet_.size() ) {
    if ( 0 != l ) {
      if ( 0 != l->document() ) {
        if ( 0 != l->document()->eventHandler() ) {
          oldlist = boundingRect( l->document()->transform() );
        }
      }
    }
  }

  skelet_ = skel;
  if ( false == calcCartesianPoints() ) {
    //    error_log << QObject::tr("Прямоугольные координаты не рассчитаны");
    return;
  }
  //debug_log<<"111property_.splinefactor()"<<property_.splinefactor();

  if ( 0 == document() || true == document()->property().smooth_iso() ) {
    int coun_sc = cartesian_points_.count();
    // debug_log<<"property_.splinefactor()"<<property_.splinefactor();
    if ( 1 < property_.splinefactor() ) {
      for(int i=0; i< coun_sc; ++i) {
        QPolygon& cart = cartesian_points_[i];
        if ( true == property_.closed() && 2 < cart.size() && cart.first() != cart.last() ) {
          cart.append(cart[0]);
        }
        QPolygon cartesian_points_vs;
        smootchLine( cart, &cartesian_points_vs, property_.splinefactor() );
        cartesian_points_[i] = cartesian_points_vs;
      }
    }
  }

  if ( 0 == l ) {
    return;
  }
  if ( 0 != l->document() ) {
    if ( 0 != l->document()->eventHandler() ) {
      QList<QRect> list = boundingRect( l->document()->transform() );
      for ( int i = 0, sz = list.size(); i < sz; ++i ) {
        const QRect& r = list[i];
        LayerEvent* ev = new LayerEvent( l->uuid(), LayerEvent::ObjectChanged, r );
        l->document()->eventHandler()->postEvent(ev);
      }
      for ( int i = 0, sz = oldlist.size(); i < sz; ++i ) {
        const QRect& r = oldlist[i];
        LayerEvent* ev = new LayerEvent( l->uuid(), LayerEvent::ObjectChanged, r );
        l->document()->eventHandler()->postEvent(ev);
      }
    }
  }
}

void CloudWx::setValue( float value, const QString& format, const QString& unit )
{
  Q_UNUSED(value);
  Q_UNUSED(format);
  Q_UNUSED(unit);
}

bool CloudWx::render( QPainter* painter, const QRect& target, const QTransform& transform )
{
  foreach(Object* o, objects_){
    painter->save();
    o->render( painter, target, transform );
    painter->restore();
  }
  return true;
}

int CloudWx::minimumScreenDistance( const QPoint& point, QPoint* cross ) const
{
  int dist = 10000000;
  QVector<QPolygon>::const_iterator it = cached_screen_points_.constBegin();
  QVector<QPolygon>::const_iterator end = cached_screen_points_.constEnd();
  QPoint res;
  for ( ; end != it; ++it ) {
    if ( 0 == (*it).size() ) {
      continue;
    }
    int tmp = meteo::geom::distance( point, *it, &res );
    if ( ::abs(tmp) < ::abs(dist) ) {
      dist = tmp;
      if ( 0 != cross ) {
        *cross = res;
      }
    }
  }
  return dist;
}

void CloudWx::setProto( const sigwx::Cloud& c )
{
  GeoVector gv( c.pnt_size() );
  for ( int j = 0, jsz = c.pnt_size(); j < jsz; ++j ) {
    gv[j] = pbgeopoint2geopoint( c.pnt(j) );
  }
  GeoPolygon* poly = new GeoPolygon(this);
  poly->setOrnament("cloud");
  poly->setOrnamentMirror(true);
  poly->setPenColor( Qt::darkGreen );
  //  poly->setClosed(true);
  poly->setClosed(false);
  poly->setSplineFactor(10);
  poly->setSkelet(gv);

  if ( 0 == document() ) {
    return;
  }

  QList<QRect> list = poly->boundingRect( document()->transform() );
  if ( 0 == list.size() ) {
    return;
  }
  const QRect& r = list[0];
  GeoPoint pnt = document()->screen2coord( r.center() );
  if ( 0 >= c.phen_size() ) {
    warning_log << QObject::tr("Непредвиденная ситуация! В явлениях облачности 0 явлений! ");
  }
  if ( 1 == c.phen_size() ) {
    if ( true == c.phen(0).has_cloud() && true ==  c.phen(0).has_cloud_amount() ) {
      Puanson* pns = new Puanson(this);
      pns->setPunch( WeatherLoader::instance()->punchlibraryspecial()["wxcloud"] );
      TMeteoData md;
      md.set( "C", "", c.phen(0).cloud(), control::RIGHT );
      md.set( "Na", "", c.phen(0).cloud_amount(), control::RIGHT );
      if ( true == c.phen(0).has_h_hi() ) {
        md.set( "h_hi", "", c.phen(0).h_hi(), control::RIGHT );
      }
      if ( true == c.phen(0).has_h_lo() ) {
        md.set( "h", "", c.phen(0).h_lo(), control::RIGHT );
      }
      pns->setMeteodata(md);
      pns->setSkelet(pnt);
    }
    //    else {
    //      warning_log << QObject::tr("Непонятная ситуация с описанием облачности");
    //    }
  }
  else if ( 2 == c.phen_size() ) {
    meteo::sigwx::SkyPhen ice;
    meteo::sigwx::SkyPhen turb;
    if ( true == c.phen(0).has_ice() ) {
      ice.CopyFrom( c.phen(0) );
    }
    if ( true == c.phen(1).has_ice() ) {
      ice.CopyFrom( c.phen(1) );
    }
    if ( true == c.phen(0).has_turb() ) {
      turb.CopyFrom( c.phen(0) );
    }
    if ( true == c.phen(1).has_turb() ) {
      turb.CopyFrom( c.phen(1) );
    }
    if ( true == turb.IsInitialized() && true == ice.IsInitialized() ) {
      Puanson* pns = new Puanson(this);
      pns->setPunch( WeatherLoader::instance()->punchlibraryspecial()["wxiceturb"] );
      TMeteoData md;
      md.set( "Ic", "", ice.ice(), control::RIGHT );
      if ( true == ice.has_h_hi() ) {
        md.set( "h_hi", "", ice.h_hi(), control::RIGHT );
      }
      if ( true == ice.has_h_lo() ) {
        md.set( "h", "", ice.h_lo(), control::RIGHT );
      }
      //      debug_log << "TURB =" << turb.turb();
      md.set( "B", "", turb.turb(), control::RIGHT );
      if ( true == turb.has_h_hi() ) {
        md.set( "hB", "", turb.h_hi(), control::RIGHT );
      }
      if ( true == turb.has_h_lo() ) {
        md.set( "tB", "", turb.h_lo(), control::RIGHT );
      }
      pns->setMeteodata(md);
      pns->setSkelet(pnt);
    }
    else {
      warning_log << QObject::tr("Непонятная ситуация с описанием обледенения и турбулентности");
    }
  }
}

int32_t CloudWx::dataSize() const
{
  return Object::dataSize();
}

int32_t CloudWx::serializeToArray( char* arr ) const
{
  return Object::data(arr);
}

int32_t CloudWx::parseFromArray( const char* arr )
{
  return Object::setData(arr);
}

void CloudWx::paintSmoothPolygon( QPainter* painter, const QPolygon& polygon )
{
  if ( 0 == polygon.size() ) {
    return;
  }
  if ( true == property_.double_line() ) {
    QPair<QPolygon,QPolygon> dbl;
    calcDouble(polygon, &dbl);
    painter->drawPolygon( dbl.first );
    painter->drawPolygon( dbl.second );
  }
  else {
    painter->drawPolygon(polygon);
  }
}

void CloudWx::paintSmoothLine( QPainter* painter, const QPolygon& polygon )
{
  if ( 0 == polygon.size() ) {
    return;
  }
  if ( true == property_.double_line() ) {
    QPair<QPolygon,QPolygon> dbl;
    calcDouble(polygon, &dbl);
    painter->drawPolyline( dbl.first );
    painter->drawPolyline( dbl.second );
  }
  else {
    painter->drawPolyline(polygon);
  }
}

void CloudWx::calcDouble( const QPolygon& source, QPair< QPolygon, QPolygon >* target )
{
  float rast = property_.pen().width();
  for ( int i = 1, sz = source.size(); i < sz; ++i ) {
    int x1 = source[i-1].x();
    int y1 = source[i-1].y();
    int x2 = source[i].x();
    int y2 = source[i].y();

    float angle = atan2( float(y2-y1),float(x2-x1)) + M_PI/2;
    int sx = MnMath::ftoi_norm(rast*sin(angle));
    int cx = MnMath::ftoi_norm(rast*cos(angle));
    if ( 1 == i ) {
      target->first.append(QPoint(x1+cx,y1+sx));
      target->second.append(QPoint(x1-cx,y1-sx));
      target->first.append(QPoint(x2+cx,y2+sx));
      target->second.append(QPoint(x2-cx,y2-sx));
    }
    else {
      target->first.append(QPoint(x2+cx,y2+sx));
      target->second.append(QPoint(x2-cx,y2-sx));
    }
  }
}



}
}
