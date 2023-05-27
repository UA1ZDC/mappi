#include "geopolygon.h"

#include <qpainter.h>
#include <qpainterpath.h>
#include <qdebug.h>
#include <qmath.h>

#include <commons/obanal/interpolorder.h>
#include <commons/mathtools/mnmath.h>
#include <commons/geom/geom.h>
#include <cross-commons/debug/tlog.h>

#include "layer.h"
#include "geotext.h"
#include "map.h"
#include "document.h"
#include "simple.h"
#include "ornament.h"

namespace meteo {
namespace map {

namespace {
Object* createGeoPolygon( Layer* l )
{
  return new GeoPolygon(l);
}
static const bool res = singleton::PtkppFormat::instance()->registerObjectHandler( GeoPolygon::Type, createGeoPolygon );
Object* createGeoPolygon2( Object* o )
{
  return new GeoPolygon(o);
}
static const bool res2 = singleton::PtkppFormat::instance()->registerChildHandler( GeoPolygon::Type, createGeoPolygon2 );
}

GeoPolygon::GeoPolygon( Layer* l )
  : Object(l)
{
}

GeoPolygon::GeoPolygon( Object* p )
  : Object(p)
{
}

GeoPolygon::GeoPolygon( Projection* p )
  : Object(p)
{
}

GeoPolygon::GeoPolygon( const meteo::Property& p )
  : Object(p)
{
}

GeoPolygon::~GeoPolygon()
{
}

Object* GeoPolygon::copy( Layer* l ) const
{
  if ( nullptr == l ) {
    error_log << QObject::tr("Нулевой указатель на слой");
    return nullptr;
  }
  GeoPolygon* gp = new GeoPolygon(l);
  gp->setProperty(property_);
  gp->setSkelet(skelet());
  foreach(Object* o, objects_){
    o->copy(gp);
  }
  return gp;
}

Object* GeoPolygon::copy( Object* o ) const
{
  if ( nullptr == o ) {
    error_log << QObject::tr("Нулевой указатель на объект");
    return nullptr;
  }
  GeoPolygon* gp = new GeoPolygon(o);
  gp->setProperty(property_);
  gp->setSkelet(skelet());
  foreach(Object* o, objects_){
    o->copy(gp);
  }
  return gp;
}

Object* GeoPolygon::copy( Projection* proj ) const
{
  if ( nullptr == proj ) {
    error_log << QObject::tr("Нулевой указатель на проекцию");
    return nullptr;
  }
  GeoPolygon* gp = new GeoPolygon(proj);
  gp->setProperty(property_);
  gp->setSkelet(skelet());
  foreach(Object* o, objects_){
    o->copy(gp);
  }
  return gp;
}

QList<QRect> GeoPolygon::boundingRect( const QTransform& transform ) const
{
  QList<QRect> list;
  QVector<QPolygon>::const_iterator it = cartesian_points_.constBegin();
  QVector<QPolygon>::const_iterator end = cartesian_points_.constEnd();
  for ( ; end != it; ++it ) {
    QRect r = transform.map(*it).boundingRect();
    QPoint addpnt( property_.pen().width()*2, property_.pen().width()*2 ) ;
    r.setTopLeft( r.topLeft() - addpnt );
    r.setBottomRight( r.bottomRight() + addpnt );
    list.append(r);
  }
  return list;
}


QList<GeoVector> GeoPolygon::skeletInRect( const QRect& rect, const QTransform& transform ) const
{
  QList<GeoVector> gvlist;
  Projection* proj = projection();
  if ( nullptr == proj ) {
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


void GeoPolygon::setSkelet( const GeoVector& skel )
{
  Layer* l = layer();
  QList<QRect> oldlist;
  if ( 0 != skelet_.size() ) {
    if ( nullptr != l ) {
      if ( nullptr != l->document() ) {
        if ( nullptr != l->document()->eventHandler() ) {
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
  
  if ( nullptr == document() || true == document()->property().smooth_iso() ) {
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
  
  if ( nullptr == l ) {
    return;
  }
  if ( nullptr != l->document() ) {
    if ( nullptr != l->document()->eventHandler() ) {
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

bool GeoPolygon::hasValue() const
{
  foreach(Object* o, objects_){
    if ( o->type() == kText ) {
      GeoText* gt = reinterpret_cast<GeoText*>(o);
      if ( true == gt->hasValue() ) {
        return true;
      }
    }
  }
  return false;
}

float GeoPolygon::value() const
{
  foreach(Object* o, objects_){
    if ( o->type() == kText ) {
      GeoText* gt = mapobject_cast<GeoText*>(o);
      if ( true == gt->hasValue() ) {
        return gt->value();
      }
    }
  }
  return kGeoPolygonInvalidValue;
}

void GeoPolygon::setValue( float value, const QString& format, const QString& unit )
{
  GeoText* gt = nullptr;
  foreach(Object* o, objects_){
    if ( o->type() == kText ) {
      GeoText* tmp = mapobject_cast<GeoText*>(o);
      if ( true == tmp->hasValue() ) {
        gt = tmp;
        break;
      }
    }
  }
  if ( nullptr == gt ) {
    gt = new GeoText(this);
  }
  setPosOnParent( kTopCenter );
  gt->setPosOnParent(kTopCenter);
  gt->setPos(kCenter);
  gt->setValue( value, format, unit );
}

void GeoPolygon::removeValue()
{
  std::unordered_set<GeoText*> list = childsByType<GeoText*>();
  foreach(Object* o, objects_){
    delete o;
  }
}

QPoint GeoPolygon::calcShift( const QPoint& pnt1, const QPoint& pnt2, int shift )
{
  QPoint delta = pnt2 - pnt1;
  float alpha = ::atan2f( delta.y(), delta.x() );
  delta = QPoint( shift*::cosf(alpha)+0.5, shift*::sinf(alpha)+0.5 );
  return delta;
}

bool GeoPolygon::render( QPainter* painter, const QRect& target, const QTransform& transform )
{
  wasdrawed_ = false;
  float scale = 0.0;
  if ( nullptr != curdoc_ ) {
    scale = curdoc_->scale();
  }
  else if ( nullptr != document() ) {
    scale = document()->scale();
  }
  if ( false == visible(scale) ) {
    return false;
  }
  cached_screen_points_.clear();

  painter->save();

  painter->setRenderHint( QPainter::Antialiasing, true );
  painter->setRenderHint( QPainter::HighQualityAntialiasing, true );

  QVector<QPolygon>::const_iterator it = cartesian_points_.constBegin();
  QVector<QPolygon>::const_iterator end = cartesian_points_.constEnd();
  for ( ; end != it; ++it ) {
    QPolygon screen = transform.map(*it);
    if ( 2 > screen.size() ) {
      continue;
    }
    if ( true == property_.closed() && screen.first() != screen.last() ) {
      screen.append( screen.first() );
    }
    bool need_draw = false;
    QRect screenrect = screen.boundingRect();
    bool screenrect_contains_target_whole = screenrect.contains( target, true );
    if ( false == screenrect_contains_target_whole && true == target.intersects(screenrect) ) {
      need_draw = true;
    }
    if ( false == need_draw && false == screenrect_contains_target_whole && true == screenrect.intersects(target) ) {
      need_draw = true;
    }
    if ( false == need_draw && true == target.contains(screenrect) ) {
      need_draw = true;
    }
    if ( false == need_draw ) {
      QPainterPath path;
      path.addPolygon(screen);
      bool screenpathcontains_target = path.contains(target);
      QBrush br = qbrush();
      if ( false == screenpathcontains_target && true == path.intersects(target) ) {
        need_draw = true;
      }
      else if ( Qt::NoBrush != br.style() && true == closed() ) {
        if ( true == screenpathcontains_target ) {
          need_draw = true;
        }
      }
    }
    if ( false == need_draw ) {
      continue;
    }

    if ( 0 != geolineStartShift() ) {
      screen[0] = screen[0] + calcShift( screen[0], screen[1], geolineStartShift() );
    }
    if ( 0 != geolineEndShift() ) {
      int sz = screen.size();
      screen[sz - 1] = screen[sz - 1] + calcShift( screen[sz - 2], screen[sz - 1], geolineEndShift() );
    }

    cached_screen_points_.append(screen);

    wasdrawed_ = true;
    QList<Label*> labels = Object::labels();
    QPainterPath pntrpath = painter->clipPath();

    QPainterPath recpath;
    for ( auto lbl : labels ) {
      if ( kText != lbl->type() ) {
        continue;
      }
      QList<QRect> rects = lbl->drawedrects();
      if ( rects.isEmpty() ) {
        continue;
      }
      for ( const auto& rect : rects ) {
        recpath.addRect(rect);
        pntrpath -= recpath;
      }
    }
    if(false == recpath.isEmpty()) {
      painter->setClipPath(pntrpath, Qt::ReplaceClip);
    }
    wasdrawed_ = false;

    if ( kBeginArrow == (property_.arrow_place() & kBeginArrow) ) {
      if ( kSimpleArrow == property_.arrow_type() ) {
        SimpleArrow arrow( kBeginArrow, &screen );
        arrow.render( painter, property_ );
      }
    }
    if ( kEndArrow == ( property_.arrow_place() & kEndArrow ) ) {
      if ( kSimpleArrow == property_.arrow_type() ) {
        SimpleArrow arrow( kEndArrow, &screen );
        arrow.render( painter, property_ );
      }
    }
    painter->setBrush( qbrush() );
    painter->setPen( qpen() );
    if ( nullptr != document()
         && true == document()->property().white_sea_under_gradient()
         && true == document()->hasGradientWithWhiteSea()
         && nullptr != layer()
         && true == layer()->isBase()  ) {
      QBrush brush = qbrush();
      QPen pen = qpen();

      if  ( kMAP_SEA_COLOR == brush.color() || kMAP_SEA_COLOR_SXF == brush.color() ) {
        brush.setColor(Qt::white);
        painter->setBrush(brush);
      }
      if ( kMAP_SEA_COLOR == pen.color() /*|| kMAP_SEA_COLOR_SXF == pen.color() */ ) {
        pen.setColor(Qt::white);
        painter->setPen(pen);
      }
    }
    if ( true == property_.closed() && 2 < screen.size() && qbrush() != Qt::NoBrush ) {
      if ( true == hasOrnament() && false == Ornament::ornament( ornament() ).skelet() ) {
        painter->save();
        QPen alphapen = painter->pen();
        alphapen.setColor( QColor(0,0,0,0) );
        painter->setPen(alphapen);
        paintSmoothPolygon( painter, screen );
        painter->restore();
      }
      else {
        paintSmoothPolygon( painter, screen );
      }
    }
    else {
      if ( false == hasOrnament() || true == Ornament::ornament( ornament() ).skelet() ) {
        paintSmoothLine( painter, screen );
      }
    }
    if ( true == hasOrnament() && false == doubleLine() ) {
      if ( true == property_.has_ornament_mirror() ) {
        Ornament o( ornament(), painter->pen().widthF(), painter->pen().color().rgba(), property_.ornament_mirror() );
        o.paintOnDocument( painter, screen );
      }
      else {
        Ornament o( ornament(), painter->pen().widthF(), painter->pen().color().rgba() );
        o.paintOnDocument( painter, screen );
      }
      //      if ( 0 != o.width( painter->pen().widthF() ) ) {
      //      }
    }
    wasdrawed_ = true;
  }
  painter->restore();
  foreach(Object* o, objects_){
    painter->save();
    o->render( painter, target, transform );
    painter->restore();
  }
  return true;
  
}

int GeoPolygon::minimumScreenDistance( const QPoint& point, QPoint* cross ) const
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
      if ( nullptr != cross ) {
        *cross = res;
      }
    }
  }
  return dist;
}

int32_t GeoPolygon::dataSize() const
{
  return Object::dataSize();
}

int32_t GeoPolygon::serializeToArray( char* arr ) const
{
  return Object::data(arr);
}

int32_t GeoPolygon::parseFromArray( const char* arr )
{
  return Object::setData(arr);
}

void GeoPolygon::paintSmoothPolygon( QPainter* painter, const QPolygon& polygon )
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

void GeoPolygon::paintSmoothLine( QPainter* painter, const QPolygon& polygon )
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

void GeoPolygon::calcDouble( const QPolygon& source, QPair< QPolygon, QPolygon >* target )
{
  float rast = property_.pen().width();
  for ( int i = 1, sz = source.size(); i < sz; ++i ) {
    int x1 = source[i-1].x();
    int y1 = source[i-1].y();
    int x2 = source[i].x();
    int y2 = source[i].y();

    float angle = atan2f( float(y2-y1),float(x2-x1)) + M_PIf/2.f;
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

} // map
} // meteo
