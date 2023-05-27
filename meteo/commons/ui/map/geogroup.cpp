#include "geogroup.h"

#include <qpainter.h>

#include "document.h"
#include <cross-commons/debug/tlog.h>

namespace meteo {
namespace map {

namespace {
  Object* createGroup( Layer* l )
  {
    return new GeoGroup(l);
  }
  static const bool res = singleton::PtkppFormat::instance()->registerObjectHandler( GeoGroup::Type, createGroup );
  Object* createGroup2( Object* o )
  {
    return new GeoGroup(o);
  }
  static const bool res2 = singleton::PtkppFormat::instance()->registerChildHandler( GeoGroup::Type, createGroup2 );
}

GeoGroup::GeoGroup( Layer* l, const QList<Object*> list )
  : Object(l)
{
  for ( int i = 0, sz = list.size(); i < sz; ++i ) {
    add(list[i]);
  }
}

GeoGroup::GeoGroup( Object* p, const QList<Object*> list )
  : Object(p)
{
  for ( int i = 0, sz = list.size(); i < sz; ++i ) {
    add(list[i]);
  }
}

GeoGroup::GeoGroup( Projection* proj, const QList<Object*> list )
  : Object(proj)
{
  for ( int i = 0, sz = list.size(); i < sz; ++i ) {
    add(list[i]);
  }
}

GeoGroup::GeoGroup( const meteo::Property& prop, const QList<Object*> list )
  : Object(prop)
{
  for ( int i = 0, sz = list.size(); i < sz; ++i ) {
    add(list[i]);
  }
}

GeoGroup::~GeoGroup()
{
}

std::unordered_set<Object *> GeoGroup::objects()
{
  return objects_;
}

Object* GeoGroup::copy( Layer* l ) const
{
  if ( 0 == l ) {
    error_log << QObject::tr("Нулевой указатель на слой");
    return 0;
  }
  GeoGroup* gr = new GeoGroup(l);
  gr->setProperty( property_ );
  gr->setSkelet( skelet() );
  foreach(Object* o, objects_){
    o->copy(gr);
  }
  return gr;
}

Object* GeoGroup::copy( Object* o ) const
{
  if ( 0 == o ) {
    error_log << QObject::tr("Нулевой указатель на объект");
    return 0;
  }
  GeoGroup* gr = new GeoGroup(o);
  gr->setProperty( property_ );
  gr->setSkelet( skelet() );
  foreach(Object* o, objects_){
    o->copy(gr);
  }
  return gr;
}

Object* GeoGroup::copy( Projection* proj ) const
{
  if ( 0 == proj ) {
    error_log << QObject::tr("Нулевой указатель на проекцию");
    return 0;
  }
  GeoGroup* gr = new GeoGroup(proj);
  gr->setProperty( property_ );
  gr->setSkelet( skelet() );
  foreach(Object* o, objects_){
    o->copy(gr);
  }
  return gr;
}

void GeoGroup::add( Object* o )
{
  if ( 0 == o ) {
    error_log << QObject::tr("Нулевой указатель");
    return;
  }
  o->setParent(this);
}

void GeoGroup::rm( Object* o )
{
  if ( 0 == o ) {
    error_log << QObject::tr("Нулевой указатель");
    return;
  }
  if ( 0 != parent_ ) {
    o->setParent(parent_);
  }
  else if ( 0 != layer_ ) {
    o->setParent(layer_);
  }
  else {
    o->setParent((Layer*)0);
  }
}

void GeoGroup::ungroup()
{
 // std::unordered_set<Object*> list = objects_;
  foreach(Object* o, objects_){
    rm(o);
  }
}

bool GeoGroup::render( QPainter* painter, const QRect& target, const QTransform& transform )
{
  wasdrawed_ = false;
  float scale = 0.0;
  if ( 0 != curdoc_ ) {
    scale = curdoc_->scale();
  }
  else if ( 0 != document() ) {
    scale = document()->scale();
  }

  if ( false == visible(scale) ) {
    return false;
  }
  foreach(Object* o, objects_){
    painter->save();
    o->render( painter, target, transform );
    if ( true == o->wasdrawed() ) {
      wasdrawed_ = true;
    }
    else {
      if ( o->type() == kText ) {
        wasdrawed_ = true;
      }
    }
    painter->restore();
  }
  return true;
}
    
QList<QRect> GeoGroup::boundingRect( const QTransform& transform ) const
{
  QList<QRect> list;
  foreach(Object* o, objects_){
    list.append( o->boundingRect(transform) );
  }
  return list;
}

QList<GeoVector> GeoGroup::skeletInRect( const QRect& rect, const QTransform& transform ) const
{
  Q_UNUSED(rect);
  Q_UNUSED(transform);
  return QList<GeoVector>();
}

QList<Object*> GeoGroup::objectsUnderGeoPoint( const GeoPoint& gp ) const
{
  QList<Object*> olist;
  foreach(Object* o, objects_){
    if ( true == o->underGeoPoint(gp) ) {
     olist.append(o);
    }
  }
  return olist;
}

bool GeoGroup::underGeoPoint( const GeoPoint& gp ) const
{
  foreach(Object* o, objects_){
    if ( true == o->underGeoPoint(gp) ) {
      return true;
    }
  }
  return false;
}

int GeoGroup::minimumScreenDistance( const QPoint& pos, QPoint* cross ) const
{
  int min = 1000000;
  foreach(Object* o, objects_){
    int lmin = o->minimumScreenDistance( pos, cross );
    if ( ::abs(lmin) < ::abs(min) ) {
      min = lmin;
    }
  }

  return min;
}

bool GeoGroup::hasValue() const
{
  foreach(Object* o, objects_){
    if ( true == o->hasValue() ) {
      return true;
    }
  }
  return false;
}

float GeoGroup::value() const
{
  foreach(Object* o, objects_){
    if ( true == o->hasValue() ) {
      return o->value();
    }
  }
  return 9999.0;
}

void GeoGroup::setValue( float val, const QString& format, const QString& unit )
{
  foreach(Object* o, objects_){
    o->setValue( val, format, unit );
  }
}

void GeoGroup::setProperty( const meteo::Property& prop )
{
  property_.CopyFrom(prop);
}

int32_t GeoGroup::dataSize() const
{
  int32_t objsz = Object::dataSize();
  return objsz;
}

int32_t GeoGroup::serializeToArray( char* arr ) const
{
  int32_t pos = Object::data(arr);
  return pos;
}

int32_t GeoGroup::parseFromArray( const char* arr )
{
  int32_t pos = Object::setData(arr);
  return pos;
}

}
}
