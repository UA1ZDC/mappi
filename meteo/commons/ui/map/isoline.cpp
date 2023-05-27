#include "isoline.h"

#include <qpainter.h>
#include <qpainterpath.h>
#include <qdebug.h>
#include <qmath.h>

#include <commons/obanal/interpolorder.h>
#include <commons/mathtools/mnmath.h>
#include <commons/geom/geom.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/global/common.h>

#include "layer.h"
#include "layeriso.h"
#include "geotext.h"
#include "map.h"
#include "document.h"
#include "simple.h"
#include <meteo/commons/global/gradientparams.h>

namespace meteo {
namespace map {

namespace {
  Object* createIso( Layer* l )
  {
    return new IsoLine(l);
  }
  static const bool res = singleton::PtkppFormat::instance()->registerObjectHandler( IsoLine::Type, createIso);
  Object* createIso2( Object* o )
  {
    return new IsoLine(o);
  }
  static const bool res2 = singleton::PtkppFormat::instance()->registerChildHandler( IsoLine::Type, createIso2 );
}

IsoLine::IsoLine( LayerIso* l, float val )
  : GeoPolygon(l)
{
  level_ = l->level();
  type_level_ = l->typeLevel();
  if ( false == MnMath::isEqual( kGeoPolygonInvalidValue, val ) ) {
    setValue(val);
  }
  loadFieldColorFromSettings( l->descriptor() );
}

IsoLine::IsoLine( Layer* l, int m, float val, int lvl, int tp_lvl )
  : GeoPolygon(l)
{
  level_ = lvl;
  type_level_ = tp_lvl;
  if ( false == MnMath::isEqual( kGeoPolygonInvalidValue, val ) ) {
    setValue(val);
  }
  loadFieldColorFromSettings(m);
}

IsoLine::IsoLine( Object* p, int m, float val, int lvl, int tp_lvl )
  : GeoPolygon(p)
{
  level_ = lvl;
  type_level_ = tp_lvl;
  if ( false == MnMath::isEqual( kGeoPolygonInvalidValue, val ) ) {
    setValue(val);
  }
  loadFieldColorFromSettings(m);
}

IsoLine::IsoLine( Projection* p, int m, float val, int lvl, int tp_lvl )
  : GeoPolygon(p)
{
  level_ = lvl;
  type_level_ = tp_lvl;
  if ( false == MnMath::isEqual( kGeoPolygonInvalidValue, val ) ) {
    setValue(val);
  }
  loadFieldColorFromSettings(m);
}

IsoLine::IsoLine( int m, float val, int lvl, int tp_lvl )
  : GeoPolygon( meteo::Property() )
{
  level_ = lvl;
  type_level_ = tp_lvl;
  if ( false == MnMath::isEqual( kGeoPolygonInvalidValue, val ) ) {
    setValue(val);
  }
  loadFieldColorFromSettings(m);
}

IsoLine::IsoLine( LayerIso* l, const proto::FieldColor& color, float val )
  : GeoPolygon(l)
{
  level_ = l->level();
  type_level_ = l->typeLevel();
  if ( false == MnMath::isEqual( kGeoPolygonInvalidValue, val ) ) {
    setValue(val);
  }
  setFieldColor(color);
}

IsoLine::IsoLine( Layer* l, const proto::FieldColor& color, float val, int lvl, int tp_lvl )
  : GeoPolygon(l)
{
  level_ = lvl;
  type_level_ = tp_lvl;
  if ( false == MnMath::isEqual( kGeoPolygonInvalidValue, val ) ) {
    setValue(val);
  }
  setFieldColor(color);
}

IsoLine::IsoLine( Object* p, const proto::FieldColor& color, float val, int lvl, int tp_lvl )
  : GeoPolygon(p)
{
  level_ = lvl;
  type_level_ = tp_lvl;
  if ( false == MnMath::isEqual( kGeoPolygonInvalidValue, val ) ) {
    setValue(val);
  }
  setFieldColor(color);
}

IsoLine::IsoLine( Projection* p, const proto::FieldColor& color, float val, int lvl, int tp_lvl )
  : GeoPolygon(p)
{
  level_ = lvl;
  type_level_ = tp_lvl;
  if ( false == MnMath::isEqual( kGeoPolygonInvalidValue, val ) ) {
    setValue(val);
  }
  setFieldColor(color);
}

IsoLine::IsoLine( const proto::FieldColor& color, float val, int lvl, int tp_lvl )
  : GeoPolygon( meteo::Property() )
{
  level_ = lvl;
  type_level_ = tp_lvl;
  if ( false == MnMath::isEqual( kGeoPolygonInvalidValue, val ) ) {
    setValue(val);
  }
  setFieldColor(color);
}

IsoLine::~IsoLine()
{
}

Object* IsoLine::copy( Layer* l ) const
{
  if ( nullptr == l ) {
    error_log << QObject::tr("Нулевой указатель на слой");
    return nullptr;
  }
  LayerIso* li = maplayer_cast<LayerIso*>(l);
  IsoLine* o = nullptr;
  if ( nullptr == li ) {
    o = new IsoLine( l, fieldcolor_, value(), level_, type_level_ );
  }
  else {
    o = new IsoLine( li, fieldcolor_, value() );
  }
  o->setProperty(property_);
  o->setSkelet(skelet());
  foreach(Object* ob, objects_){
    ob->copy(o);
  }
  return o;
}

Object* IsoLine::copy( Object* o ) const
{
  if ( nullptr == o ) {
    error_log << QObject::tr("Нулевой указатель на объект");
    return nullptr;
  }
  IsoLine* iso = new IsoLine(o, fieldcolor_, value(), level_, type_level_ );
  iso->setProperty(property_);
  iso->setSkelet(skelet());
  foreach(Object* o, objects_){
    o->copy(iso);
  }
  return iso;
}

Object* IsoLine::copy( Projection* proj ) const
{
  if ( nullptr == proj ) {
    error_log << QObject::tr("Нулевой указатель на проекцию");
    return nullptr;
  }
  IsoLine* iso = new IsoLine(proj, fieldcolor_, value(), level_, type_level_ );
  iso->setProperty(property_);
  iso->setSkelet(skelet());
  foreach(Object* o, objects_){
    o->copy(iso);
  }
  return iso;
}

QList<GeoVector> IsoLine::skeletInRect( const QRect& rect, const QTransform& transform ) const
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
      GeoVector gvcr;
      document()->projection()->X2F(cartesian_points_[i], &gvcr);
      gvlist.append(gvcr);
    }
  }
  return gvlist;
}
    
void IsoLine::setValue( float val, const QString& frmt, const QString& unt )
{
  GeoPolygon::setValue( val, frmt, unt );
  updateObjectProto();
  foreach(Object* o, objects_){
    Position oldpop = o->posOnParent();
    Position oldpos = o->pos();
    o->setProperty(property_);
    o->setPosOnParent(oldpop);
    o->setPos(oldpos);
    o->setPriority(20);
  }
}

QPen IsoLine::pen() const
{
  return pen2qpen( fieldcolor_.pen() );
}

float IsoLine::minValue() const
{
  return GradientParams::isoMin( level_, type_level_, fieldcolor_ );
}

float IsoLine::maxValue() const
{
  return GradientParams::isoMax( level_, type_level_, fieldcolor_ );
}

QColor IsoLine::minColor() const
{
  return GradientParams::isoColorMin( level_, type_level_, fieldcolor_ );
}

QColor IsoLine::maxColor() const
{
  return GradientParams::isoColorMax( level_, type_level_, fieldcolor_ );
}

TColorGrad IsoLine::gradient() const
{
  return GradientParams::isoParams( level_, type_level_, fieldcolor_ );
}

float IsoLine::stepIso() const
{
  return GradientParams::isoStep( level_, type_level_, fieldcolor_ );
}

void IsoLine::setColor( const TColorGrad& grad )
{
  GradientParams::setIsoColor( level_, type_level_, grad, &fieldcolor_ );
  updateObjectProto();
}

void IsoLine::setStepIso( float step )
{
  GradientParams::setIsoStep( level_, type_level_, step, &fieldcolor_ );
  updateObjectProto();
}

void IsoLine::setMin( float min )
{
  GradientParams::setIsoMin( level_, type_level_, min, &fieldcolor_ );
  updateObjectProto();
}

void IsoLine::setMax( float max )
{
  GradientParams::setIsoMax( level_, type_level_, max, &fieldcolor_ );
  updateObjectProto();
}

void IsoLine::setColorMin( const QColor& clr )
{
  GradientParams::setIsoColorMin( level_, type_level_, clr, &fieldcolor_ );
  updateObjectProto();
}

void IsoLine::setColorMax( const QColor& clr )
{
  GradientParams::setIsoColorMax( level_, type_level_, clr, &fieldcolor_ );
  updateObjectProto();
}

bool IsoLine::render( QPainter* painter, const QRect& target, const QTransform& transform )
{
  return GeoPolygon::render( painter, target, transform );
}

void IsoLine::loadFieldColorFromSettings( int descr )
{
  GradientParams params( meteo::global::kIsoParamPath() );
  fieldcolor_ = params.protoParams(descr);
  updateObjectProto();
}

void IsoLine::updateObjectProto()
{
  float val = value();
  if ( true == MnMath::isEqual( val, kGeoPolygonInvalidValue ) ) {
    Object::setFont( fieldcolor_.font() );
    Object::setPen( fieldcolor_.pen() );
    return;
  }
  TColorGrad grad = GradientParams::isoParams( level_, type_level_, fieldcolor_ );
  bool ok = false;
  QColor clr = grad.color( value(), &ok );
  if ( false == ok ) {
    error_log << QObject::tr("Цвет для изолинии %1 не найден. Уровень = %2. Тип уровня = %3. Значение = %4")
      .arg( fieldcolor_.descr() )
      .arg( level_ )
      .arg( type_level_ )
      .arg( value() );
    clr = Qt::black;
  }
  Object::setFont( fieldcolor_.font() );
  Pen pen = fieldcolor_.pen();
  pen.set_color( clr.rgba() );
  foreach(Object* o, objects_){
    o->prepareChange();
  }

  Object::setPen(pen);
}

int32_t IsoLine::dataSize() const
{
  int32_t objsz = Object::dataSize();
  objsz += sizeof(int32_t); //level
  objsz += sizeof(int32_t); //type_level
  std::string str;
  if ( true == fieldcolor_.IsInitialized() ) {
    fieldcolor_.SerializeToString(&str);
  }
  int32_t sz = str.size();
  objsz += sizeof(sz);
  objsz += sz;
  
  return objsz;
}

int32_t IsoLine::serializeToArray( char* arr ) const
{
  int32_t pos = Object::data(arr);
  int32_t var = level_;
  ::memcpy( arr + pos, &var, sizeof(var) );
  pos += sizeof(var);
  var = type_level_;
  ::memcpy( arr + pos, &var, sizeof(var) );
  pos += sizeof(var);
  std::string str;
  if ( true == fieldcolor_.IsInitialized() ) {
    fieldcolor_.SerializeToString(&str);
  }
  var = str.size();
  ::memcpy( arr + pos, &var, sizeof(var) );
  pos += sizeof(var);
  ::memcpy( arr + pos, str.data(), var );
  pos += var;
  return pos;
}

int32_t IsoLine::parseFromArray( const char* arr )
{
  int32_t pos = Object::setData(arr);
  if ( -1 == pos ) {
    return pos;
  }
  int32_t var;
  global::fromByteArray( arr + pos, &var );
  pos += sizeof(var);
  level_ = var;
  global::fromByteArray( arr + pos, &var );
  pos += sizeof(var);
  type_level_ = var;
  global::fromByteArray( arr + pos, &var );
  pos += sizeof(var);
  if ( 0 != var ) {
    fieldcolor_.ParseFromArray( arr + pos, var );
  }
  pos += var;
  return pos;
}

}
}
