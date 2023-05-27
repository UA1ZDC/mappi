#include "object.h"

#include "geogroup.h"
#include "geopixmap.h"
#include "geopolygon.h"
#include "geotext.h"
#include "puanson.h"
#include "isoline.h"
#include "layeriso.h"
#include "geomrl.h"
#include "wind.h"
#include "flow.h"
#include "label.h"


#include <quuid.h>

#include <commons/geobasis/projection.h>
#include <commons/mathtools/mnmath.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/proto/map_ornament.pb.h>
#include <cross-commons/debug/tlog.h>

#include "layer.h"
#include "event.h"
#include "document.h"
#include "geopolygon.h"
#include "geotext.h"
#include <meteo/commons/global/weatherloader.h>

namespace meteo {
namespace map {
//
//int Object::OBJECT_COUNTER = 0;
//static QMutex gMutex(QMutex::Recursive);

Object::Object( Layer* l )
  : uuid_( QUuid::createUuid().toString() ),
    layer_(l),
    parent_(nullptr),
    projection_(nullptr),
    curdoc_(nullptr),
    wasdrawed_(false)
{
  //  QMutexLocker loc(&gMutex);
  //  OBJECT_COUNTER += 1;
  layer_->addObject(this);
}

Object::Object( Object* p )
  : uuid_( QUuid::createUuid().toString() ),
    layer_(nullptr),
    parent_(p),
    projection_(nullptr),
    curdoc_(nullptr),
    wasdrawed_(false)
{
  //  QMutexLocker loc(&gMutex);
  //  OBJECT_COUNTER += 1;
  parent_->addObject(this);
  setProperty(parent_->property());
  if ( true == property_.has_general() ) {
    generalization_.loadProto( property_.general() );
  }
}

Object::Object( Projection* proj )
  : uuid_( QUuid::createUuid().toString() ),
    layer_(nullptr),
    parent_(nullptr),
    projection_(proj),
    curdoc_(nullptr),
    wasdrawed_(false)
{
  //  QMutexLocker loc(&gMutex);
  //  OBJECT_COUNTER += 1;
}

Object::Object( const meteo::Property& p )
  : uuid_( QUuid::createUuid().toString() ),
    layer_(nullptr),
    parent_(nullptr),
    projection_(nullptr),
    curdoc_(nullptr),
    wasdrawed_(false)
{
  //  QMutexLocker loc(&gMutex);
  //  OBJECT_COUNTER += 1;
  setProperty(p);
  if ( true == property_.has_general() ) {
    generalization_.loadProto( property_.general() );
  }
}

Object::~Object()
{
  //  QMutexLocker loc(&gMutex);
  //  OBJECT_COUNTER -= 1;
  foreach(Object* o, objects_){
    o->parent_ = nullptr;
    delete o;
  }
  Document* doc = document();
  Layer* l = layer();
  if ( nullptr != layer_ ) {
    layer_->rmObject(this);
  }
  else if ( nullptr != parent_ ) {
    parent_->rmObject(this);
  }
  if ( nullptr != doc && nullptr != l ) {
    if ( nullptr != doc->eventHandler() ) {
      for ( int i = 0, sz = cached_screen_points_.size(); i < sz; ++i ) {
        const QRect& r = cached_screen_points_[i].boundingRect();
        LayerEvent* ev = new LayerEvent( l->uuid(), LayerEvent::ObjectChanged, r );
        doc->eventHandler()->postEvent(ev);
      }
    }
  }
}

void Object::updateProjection()
{
  foreach(Object* o, objects_){
    o->updateProjection();
  }
  calcCartesianPoints();
}

QList<Label*> Object::labels( bool childs ) const
{
  QList<Label*> list;
  for ( auto o : objects_ ) {
    if ( true == o->isLabel() ) {
      list.append( static_cast<Label*>( o ) );
    }
    if ( true == childs ) {
      list.append( o->labels(childs) );
    }
  }
  return list;
}

bool Object::visible( float scalepower ) const
{
  if ( false == property_.visible() ) {
    return false;
  }
  if ( false == generalization_.scaleVisible(scalepower) ) {
    return false;
  }
  return true;
}

bool Object::onDocument() const
{
  if ( nullptr == document() ) {
    return false;
  }
  Document* d = document();
  QRect r = d->documentRect();
  QList<QRect> list = boundingRect( d->transform() );
  bool fl = false;
  for ( int i = 0, sz = list.size(); i < sz; ++i ) {
    if ( true == r.contains(list[i]) || true == r.intersects(list[i]) || true == list[i].contains(r) ) {
      fl = true;
      break;
    }
  }
  return fl;
}

bool Object::render( QPainter* painter, const QRect& target, Document* doc )
{
  curdoc_ = doc;
  bool ret_val = render( painter, target, doc->transform() );
  curdoc_ = nullptr;
  return ret_val;
}

bool Object::containingInRect( const QRect& rect, const QTransform& tr ) const
{
  QList<QRect> list = boundingRect(tr);
  if ( 0 == list.size() ) {
    return false;
  }
  QList<QRect> retlist;
  for ( int i = 0, sz = list.size(); i < sz; ++i ) {
    const QRect& r = list[i];
    if ( true == rect.contains(r, false ) ) {
      return true;
      break;
    }
  }
  return false;
}

void Object::setSkelet( const GeoPoint& skel )
{
  Layer* l = layer();
  QList<QRect> oldlist;
  if ( 0 != skelet_.size() ) {
    if ( nullptr != l ) {
      if ( nullptr != l->document() ) {
        if ( nullptr != document()->eventHandler() && false == document()->eventHandler()->muted() ) {
          oldlist = boundingRect( l->document()->transform() );
        }
      }
    }
  }
  skelet_.clear();
  skelet_.append(skel);
  if ( false == calcCartesianPoints() ) {
    //    error_log << QObject::tr("Прямоугольные координаты не рассчитаны");
    return;
  }
  if ( nullptr == l ) {
    //    error_log << QObject::tr("Неудалось определить слой, на котором расположен примитив");
    return;
  }
  if ( nullptr != document() ) {
    if ( nullptr != document()->eventHandler() && false == document()->eventHandler()->muted() ) {
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


void Object::setSkelet( const GeoVector& skel )
{
  Layer* l = layer();
  QList<QRect> oldlist;
  if ( 0 != skelet_.size() ) {
    if ( nullptr != l ) {
      if ( nullptr != l->document() ) {
        if ( nullptr != document()->eventHandler() && false == document()->eventHandler()->muted() ) {
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
  if ( nullptr == l ) {
    //    error_log << QObject::tr("Неудалось определить слой, на котором расположен примитив");
    return;
  }
  if ( nullptr != document() ) {
    if ( nullptr != document()->eventHandler() && false == document()->eventHandler()->muted() ) {
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

void Object::setProperty( const meteo::Property& prop )
{
  property_.CopyFrom(prop);
  if ( true == property_.has_general() ) {
    generalization_.loadProto( property_.general() );
  }
}

Document* Object::document() const
{
  if ( nullptr != curdoc_ ) {
    return curdoc_;
  }
  if ( nullptr == layer() ) {
    return nullptr;
  }
  return layer()->document();
}

Layer* Object::layer() const
{
  if ( nullptr != layer_ ) {
    return layer_;
  }
  else if ( nullptr != parent_ ) {
    return parent_->layer();
  }
  return nullptr;
}

void Object::setParent( Object* p )
{
  if ( nullptr != layer_ ) {
    layer_->rmObject(this);
    layer_ = nullptr;
  }
  if ( parent_ == p ) {
    return;
  }
  if ( nullptr != parent_ ) {
    parent_->rmObject(this);
  }
  parent_ = p;
  if ( nullptr != parent_ ) {
    parent_->addObject(this);
  }
}

void Object::setParent( Layer* p )
{
  if ( nullptr != parent_ ) {
    parent_->rmObject(this);
    parent_ = nullptr;
  }
  if ( layer_ == p ) {
    return;
  }
  if ( nullptr != layer_ ) {
    layer_->rmObject(this);
  }
  layer_ = p;
  if ( nullptr != layer_ ) {
    layer_->addObject(this);
  }
}

bool Object::underGeoPoint( const GeoPoint& gp ) const
{
  QPoint cartesian;
  Layer* l = layer();
  Projection* proj = projection_;
  if ( nullptr == proj && nullptr != l ) {
    proj = l->projection();
  }
  if ( nullptr == proj ) {
    error_log << QObject::tr("Проекция не установлена");
    return false;
  }
  bool res = proj->F2X_one( gp, &cartesian );
  if ( false == res ) {
    return false;
  }
  for ( int i = 0, sz = cartesian_points_.size(); i < sz; ++i ) {
    if ( true == cartesian_points_[i].containsPoint(cartesian, Qt::OddEvenFill ) ) {
      return true;
    }
  }
  return false;
}

std::unordered_set<Object*> Object::objectsByValue( float val ) const
{
  std::unordered_set<Object*> olist;
  foreach(Object* o, objects_){
    if ( o->hasValue() && true == MnMath::isEqual( val, o->value() ) ) {
      olist.insert(o);
    }
  }
  return olist;
}

std::unordered_set<Object*> Object::objectsByType( int atype ) const
{
  std::unordered_set<Object*> olist;
  foreach(Object* o, objects_){
    if (o &&  (o->type() == atype)  ) {
      olist.insert(o);
    }
  }
  return olist;
}



std::unordered_set<GeoPolygon*> Object::polygonsByValue( float val ) const
{
  std::unordered_set<GeoPolygon*> olist;
  foreach(Object* o, objects_){
    if ( false == o->hasValue() || kPolygon != o->type() ) {
      continue;
    }
    GeoPolygon* gp = mapobject_cast<GeoPolygon*>(o);
    if ( true == MnMath::isEqual( val, gp->value() ) ) {
      olist.insert(gp);
    }
  }
  return olist;
}

std::unordered_set<GeoText*> Object::textsByValue( float val ) const
{
  std::unordered_set<GeoText*> olist;
  foreach(Object* o, objects_){
    if ( false == o->hasValue() || kText != o->type() ) {
      continue;
    }
    GeoText* gt = mapobject_cast<GeoText*>(o);
    if ( true == MnMath::isEqual( val, gt->value() ) ) {
      olist.insert(gt);
    }
  }
  return olist;
}

Projection* Object::projection() const
{
  Layer* l = layer();
  if ( nullptr != l ) {
    return l->projection();
  }
  if ( nullptr != parent_ ) {
    return parent_->projection();
  }
  return projection_;
}

void Object::addObject( Object* o )
{
  if ( nullptr == o ) {
    error_log << QObject::tr("Нулевой указатель");
    return;
  }
//  if ( 0 != objects_.count(o) ) {
//    return;
//  }
  objects_.insert(o);
}

void Object::rmObject( Object* o )
{
  if ( nullptr == o ) {
    error_log << QObject::tr("Нулевой указатель");
    return;
  }
 // if ( false == objects_.contains(o) ) {
 //   return;
 // }
  objects_.erase(o);
}

bool Object::setCartesianSkelet(const QPolygon &acs)
{
  cartesian_points_.clear();
  skelet_.clear();
  Projection* proj = projection();
  if ( nullptr == proj ) {
    error_log << QObject::tr("Проекция не установлена");
    return false;
  }
  bool res = proj->X2F( acs, &skelet_);

  cartesian_points_<<acs;

  return res;

}


bool Object::calcCartesianPoints()
{
  cartesian_points_.clear();
  Projection* proj = projection();
  if ( nullptr == proj ) {
    error_log << QObject::tr("Проекция не установлена");
    return false;
  }
  bool closed = property_.closed();
  if ( kIsoline == type() ) {
    closed = false;
  }
  bool res = proj->F2X( skelet_, &cartesian_points_, closed );
  return res;
}

bool Object::isClosed() const
{
  if ( 1 > skelet_.count() ) {
    return false;
  }
  return (skelet_.first() == skelet_.last());
}

//! если на обрезе
bool Object::isObrez() const
{
  if(cartesian_points_.count() < 1) return false;

  bool is = ( 3 > abs(cartesian_points_.at(0).first().x() - cartesian_points_.at(0).last().x()))||
      (3 > abs(cartesian_points_.at(0).first().y() - cartesian_points_.at(0).last().y()));
  if(cartesian_points_.count() == 1) return is;
  if(cartesian_points_.count() > 1) {
    return (is|| 3 > abs(cartesian_points_.at(1).first().x() - cartesian_points_.at(1).last().x()) ||
            (3 > abs(cartesian_points_.at(1).first().y() - cartesian_points_.at(1).last().y())));
  }
  return false;
}

void Object::setVisible( bool fl )
{
  property_.set_visible(fl);
  cached_property_string_.clear();
}

void Object::setGeneral( const General& g )
{
  property_.mutable_general()->CopyFrom(g);
  generalization_.loadProto(g);
  cached_property_string_.clear();
}

void Object::setGeneral( const Generalization& g )
{
  property_.mutable_general()->CopyFrom( g.proto() );
  generalization_ = g;
  cached_property_string_.clear();
}

void Object::setZlevel( int32_t zlevel )
{
  property_.set_zlevel(zlevel);
  cached_property_string_.clear();
}

void Object::setPen( const Pen& p )
{
  property_.mutable_pen()->CopyFrom(p);
  cached_property_string_.clear();
}

void Object::setPen( const QPen& p )
{
  property_.mutable_pen()->CopyFrom( qpen2pen(p) );
  cached_property_string_.clear();
}

void Object::setPenStyle( Qt::PenStyle st )
{
  property_.mutable_pen()->set_style( static_cast<PenStyle>(st) );
  cached_property_string_.clear();
}

void Object::setPenWidth( int w )
{
  property_.mutable_pen()->set_width(w);
  cached_property_string_.clear();
}

void Object::setPenColor( const QColor& c )
{
  property_.mutable_pen()->set_color( c.rgba() );
  cached_property_string_.clear();
}

void Object::setBrush( const Brush& b )
{
  property_.mutable_brush()->CopyFrom(b);
  cached_property_string_.clear();
}

void Object::setBrush( const QBrush& b )
{
  property_.mutable_brush()->CopyFrom( qbrush2brush( b ) );
  cached_property_string_.clear();
}

void Object::setBrushStyle( Qt::BrushStyle st )
{
  property_.mutable_brush()->set_style( static_cast<BrushStyle>(st) );
  cached_property_string_.clear();
}

void Object::setBrushColor( const QColor& c )
{
  property_.mutable_brush()->set_color( c.rgba() );
  cached_property_string_.clear();
}

void Object::setSplineFactor( int32_t s )
{
  property_.set_splinefactor(s);
  cached_property_string_.clear();
}

void Object::setClosed( bool fl )
{
  property_.set_closed(fl);
  cached_property_string_.clear();
}

void Object::setDoubleLine( bool fl )
{
  property_.set_double_line(fl);
  cached_property_string_.clear();
}

void Object::setArrowPlace( ArrowPlace ap )
{
  property_.set_arrow_place(ap);
  cached_property_string_.clear();
}

void Object::setArrowType( ArrowType at )
{
  property_.set_arrow_type(at);
  cached_property_string_.clear();
}

void Object::setPos( Position p )
{
  property_.set_pos(p);
  cached_property_string_.clear();
}

void Object::setPos( Qt::AlignmentFlag p )
{
  property_.set_pos( qtalignmentflag2position(p) );
  cached_property_string_.clear();
}

void Object::setPosOnParent( Position p )
{
  property_.set_pos_on_parent(p);
  cached_property_string_.clear();
}

void Object::setPosOnParent( Qt::AlignmentFlag p )
{
  property_.set_pos_on_parent( qtalignmentflag2position(p) );
  cached_property_string_.clear();
}

void Object::setDrawOnOnEnds( bool fl )
{
  property_.set_draw_on_ends(fl);
  cached_property_string_.clear();
}

void Object::setGeolineStartShift( int pixnum )
{
  property_.set_geoline_start_shift(pixnum);
  cached_property_string_.clear();
}

void Object::setGeolineEndShift( int pixnum )
{
  property_.set_geoline_end_shift(pixnum);
  cached_property_string_.clear();
}

void Object::setDrawAlways( bool fl )
{
  property_.set_draw_always(fl);
  cached_property_string_.clear();
}

void Object::setFont( const Font& f )
{
  property_.mutable_font()->CopyFrom(f);
  cached_property_string_.clear();
}

void Object::setFont( const QFont& f )
{
  property_.mutable_font()->CopyFrom( qfont2font(f) );
  cached_property_string_.clear();
}

void Object::setFontFamily( const QString& f )
{
  property_.mutable_font()->set_family( f.toStdString() );
  cached_property_string_.clear();
}

void Object::setFontPointSize( int ps )
{
  property_.mutable_font()->set_pointsize(ps);
  cached_property_string_.clear();
}

void Object::setFontWeight( int fw )
{
  property_.mutable_font()->set_weight(fw);
  cached_property_string_.clear();
}

void Object::setFontBold( bool b )
{
  property_.mutable_font()->set_bold(b);
  cached_property_string_.clear();
}

void Object::setFontItalic( bool i )
{
  property_.mutable_font()->set_italic(i);
  cached_property_string_.clear();
}

void Object::setFontUnderline( bool u )
{
  property_.mutable_font()->set_underline(u);
  cached_property_string_.clear();
}

void Object::setRamka( TextRamka r )
{
  property_.set_ramka(r);
  cached_property_string_.clear();
}

void Object::setRotateAngle( float a )
{
  property_.set_rotateangle(a);
  cached_property_string_.clear();
}

void Object::setScaleXy( const PointF& sc )
{
  property_.mutable_scalexy()->CopyFrom(sc);
  cached_property_string_.clear();
}

void Object::setScaleXy( const QPointF& sc )
{
  property_.mutable_scalexy()->CopyFrom( qpointf2pointf(sc) );
  cached_property_string_.clear();
}

void Object::setScale( float dx, float dy )
{
  PointF sc;
  sc.set_x(dx);
  sc.set_y(dy);
  property_.mutable_scalexy()->CopyFrom(sc);
  cached_property_string_.clear();
}

void Object::setScale( float dxy )
{
  PointF sc;
  sc.set_x(dxy);
  sc.set_y(dxy);
  property_.mutable_scalexy()->CopyFrom(sc);
  cached_property_string_.clear();
}

void Object::setTranslateXy( const Point& pnt )
{
  property_.mutable_translatexy()->CopyFrom(pnt);
  cached_property_string_.clear();
}

void Object::setTranslateXy( const QPoint& pnt )
{
  property_.mutable_translatexy()->CopyFrom( qpoint2point(pnt) );
  cached_property_string_.clear();
}

void Object::setTranslate( int dx, int dy )
{
  Point tr;
  tr.set_x(dx);
  tr.set_y(dy);
  property_.mutable_translatexy()->CopyFrom(tr);
  cached_property_string_.clear();
}

void Object::setTranslate( int dxy )
{
  Point tr;
  tr.set_x(dxy);
  tr.set_y(dxy);
  property_.mutable_translatexy()->CopyFrom(tr);
  cached_property_string_.clear();
}

void Object::setTextScale( TextScale sc )
{
  property_.set_textscale(sc);
  cached_property_string_.clear();
}

void Object::setOriginScale( float sc )
{
  property_.set_originalscale(sc);
  cached_property_string_.clear();
}

void Object::setOutlineColor( uint32_t c )
{
  property_.set_outlinecolor(c);
  cached_property_string_.clear();
}

void Object::setAlphaPercent( int a )
{
  if ( 100 < a ) {
    a = 100;
  }
  if ( 0 > a ) {
    a = 0;
  }
  property_.set_alphapercent(a);
  foreach(Object* o, objects_){
    o->setAlphaPercent(a);
  }
  cached_property_string_.clear();
}

void Object::setPriority( int p )
{
  property_.set_priority(p);
  cached_property_string_.clear();
}

void Object::setOrnament( const QString& o )
{
  if ( true == o.isEmpty() ) {
    property_.clear_priority();
    property_.clear_ornament_mirror();
  }
  else {
    property_.set_ornament( o.toStdString() );
    QMap<QString, proto::Ornament > olist =  WeatherLoader::instance()->ornamentlibrary();
    if ( true == olist.contains(o) ) {
      const proto::Ornament& orn = olist[o];
      if ( orn.has_mirror() && proto::kMirrorV == orn.mirror() ) {
        setOrnamentMirror(true);
      }
    }
  }
  cached_property_string_.clear();

}

void Object::setOrnamentMirror( bool fl )
{
  property_.set_ornament_mirror(fl);
  cached_property_string_.clear();
}

void Object::setDrawOnRamka( bool fl )
{
  property_.set_draw_on_ramka(fl);
  cached_property_string_.clear();
}

void Object::setOutlineColor( const QColor& c )
{
  property_.set_outlinecolor( c.rgba() );
  cached_property_string_.clear();
}

bool Object::visible() const
{
  return property_.visible();
}
const Generalization& Object::general() const
{
  return generalization_;
}

int32_t Object::zlevel() const
{
  return property_.zlevel();
}

Pen Object::pen() const
{
  return property_.pen();
}

QPen Object::qpen() const
{
  return pen2qpen( pen() );
}

Brush Object::brush() const
{
  return property_.brush();
}

QBrush Object::qbrush() const
{
  return brush2qbrush( brush() );
}

int32_t Object::spinefactor() const
{
  return property_.splinefactor();
}

bool Object::closed() const
{
  return property_.closed();
}

bool Object::doubleLine() const
{
  return property_.double_line();
}

ArrowPlace Object::arrowPlace() const
{
  return property_.arrow_place();
}

ArrowType Object::arrowType() const
{
  return property_.arrow_type();
}

Position Object::pos() const
{
  return property_.pos();
}

int Object::qpos() const
{
  return position2qtalignmentflag( property_.pos() );
}

Position Object::posOnParent() const
{
  return property_.pos_on_parent();
}

int Object::qposOnParent() const
{
  return position2qtalignmentflag( property_.pos_on_parent() );
}

bool Object::drawOnOnEnds() const
{
  return property_.draw_on_ends();
}

bool Object::drawAlways() const
{
  return property_.draw_always();
}

const Font& Object::font() const
{
  return property_.font();
}

QFont Object::qfont() const
{
  return font2qfont( property_.font() );
}

TextRamka Object::ramka() const
{
  return property_.ramka();
}

float Object::rotateAngle() const
{
  return property_.rotateangle();
}

const PointF& Object::scaleXy() const
{
  return property_.scalexy();
}

QPointF Object::qscaleXy() const
{
  return pointf2qpointf( property_.scalexy() );
}

const Point& Object::translateXy() const
{
  return property_.translatexy();
}

QPoint Object::qtranslateXy() const
{
  return point2qpoint( property_.translatexy() );
}

TextScale Object::textScale() const
{
  return property_.textscale();
}

float Object::originScale() const
{
  return property_.originalscale();
}

uint32_t Object::outlineColor() const
{
  return property_.outlinecolor();
}

int Object::geolineStartShift() const
{
  return property_.geoline_start_shift();
}

int Object::geolineEndShift() const
{
  return property_.geoline_end_shift();
}

QColor Object::qoutlineColor() const
{
  return QColor::fromRgba( property_.outlinecolor() );
}

int Object::priority() const
{
  return property_.priority();
}

bool Object::drawOnRamka() const
{
  return property_.draw_on_ramka();
}

bool Object::hasOrnament() const
{
  if ( false == property_.has_ornament() || 0 == property_.ornament().size() ) {
    return false;
  }
  return true;
}

QString Object::ornament() const
{
  if ( false == property_.has_ornament() || 0 == property_.ornament().size() ) {
    return QString();
  }
  return QString::fromStdString( property_.ornament() );
}

bool Object::ornamentMirror() const
{
  return property_.ornament_mirror();
}

void Object::setProperty( const QString& n, const QString& v )
{
  std::string stdn = n.toStdString();
  SimpleProp* prop = nullptr;
  for ( int i = 0, sz = property_.prop_size(); i < sz; ++i ) {
    if ( stdn == property_.prop(i).name() ) {
      prop = property_.mutable_prop(i);
      break;
    }
  }
  if ( nullptr == prop ) {
    prop = property_.add_prop();
  }
  prop->set_name(stdn);
  prop->set_value(v.toStdString());
  cached_property_string_.clear();
}

QString Object::property( const QString& n ) const
{
  std::string stdn = n.toStdString();
  QString val;
  for ( int i = 0, sz = property_.prop_size(); i < sz; ++i ) {
    if ( stdn == property_.prop(i).name() ) {
      val = QString::fromStdString( property_.prop(i).value() );
      break;
    }
  }
  return val;
}

int32_t Object::dataSize() const
{
  int32_t selfsize = 0;
  selfsize += sizeof(int32_t);          //размер типа объекта
  int32_t sz = uuid_.toUtf8().size();
  selfsize += sizeof(sz);               //размер размера uuid
  selfsize += sz;                       //размер uuid

  const std::string& str = propertyString();
  sz = str.size();
  selfsize += sizeof(sz);               //размер размера property
  selfsize += sz;                       //размер property
  sz = skelet_.dataSize();
  //  selfsize += sizeof(sz);               //размер размера skelet
  selfsize += sz;                       //размер skelet

  return selfsize;
}

int32_t Object::data( char* arr ) const
{
  int32_t pos = 0;
  int32_t t = type();
  int32_t sz = sizeof(t);
  ::memcpy( arr + pos, &t, sizeof(t) );
  pos += sz;
  QByteArray loc = uuid_.toUtf8();
  sz = loc.size();
  ::memcpy( arr + pos, &sz, sizeof(sz) );
  pos += sizeof(sz);
  ::memcpy( arr + pos, loc.data(), sz );
  pos += sz;

  const std::string& str = propertyString();
  sz = str.size();
  ::memcpy( arr + pos, &sz, sizeof(sz) );
  pos += sizeof(sz);
  ::memcpy( arr + pos, str.data(), sz );
  pos += sz;
  pos += skelet_.getData( arr + pos );
  return pos;
}

int32_t Object::setData( const char* arr )
{
  int32_t pos = 0;
  int32_t t;
  global::fromByteArray( arr + pos, &t );
  if ( t != type() ) {
    error_log << QObject::tr("Ошибка открытия документа. t = %1. type() = %2")
                 .arg( t )
                 .arg( type() );
    return -1;
  }
  pos += sizeof(t);
  int32_t sz;
  global::fromByteArray( arr + pos, &sz );
  pos += sizeof(sz);
  uuid_ = QString::fromUtf8( arr + pos, sz );
  pos += sz;
  global::fromByteArray( arr + pos, &sz );
  pos += sizeof(sz);
  std::string str( arr + pos, sz );
  pos += sz;
  meteo::Property prop;
  prop.ParseFromString(str);
  cached_property_string_.clear();
  setProperty(prop);
  GeoVector gv = GeoVector::fromData( arr + pos );
  setSkelet(gv);
  pos += gv.dataSize();
  return pos;
}

int32_t Object::childDataSize() const
{
  int32_t sz = objects_.size();
  int32_t childsize = sizeof(sz);
  foreach(Object* o, objects_){
    int32_t res = o->dataSize();
    childsize += res;
    if ( 0 != res ) {
      childsize += o->childDataSize();
    }
  }
  return childsize;
}

int32_t Object::serializeToArray( char* arr ) const
{
  return data(arr);
}
int32_t Object::parseFromArray( const char* arr )
{
  return setData(arr);
}

int32_t Object::serializeChildsToArray( char* arr ) const
{
  int32_t sz = 0;
  int32_t pos = 0;
  foreach(Object* o, objects_){
    int32_t res = o->dataSize();
    if ( 0 != res ) {
      sz += 1;
    }
  }
  ::memcpy( arr + pos, &sz, sizeof(sz) );
  pos += sizeof(sz);
  if ( 0 == sz ) {
    return pos;
  }
  foreach(Object* o, objects_){
    int32_t res = o->serializeToArray( arr + pos );
    pos += res;
    if ( 0 != res ) {
      pos += o->serializeChildsToArray(arr + pos);
    }
  }
  if ( pos == sizeof(sz) ) {
    pos = 0;
  }
  return pos;
}

int32_t Object::parseChildsFromArray( const char* arr )
{
  int32_t sz;
  int32_t pos = 0;
  global::fromByteArray( arr + pos, &sz );
  pos += sizeof(sz);
  for ( int i = 0; i < sz; ++i ) {
    int32_t t;
    global::fromByteArray( arr + pos, &t );
    Object* o = singleton::PtkppFormat::instance()->createChild( t, this );
    if ( nullptr == o ) {
      error_log << QObject::tr("Не удалось открыь документ. Тип объекта = %1").arg(t);
      return -1;
    }
    pos += o->parseFromArray( arr + pos );
    pos += o->parseChildsFromArray( arr + pos );
  }
  return pos;
}

const std::string& Object::propertyString() const
{
  if ( 0 == cached_property_string_.size() ) {
    property_.SerializeToString(&cached_property_string_);
  }
  return cached_property_string_;
}

}
}

