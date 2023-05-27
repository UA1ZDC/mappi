#include "ptkppformat.h"

#include "document.h"
#include "map.h"
#include <cross-commons/debug/tlog.h>

namespace meteo {
namespace map {

namespace singleton {
template<> meteo::map::PtkppFormat* PtkppFormat::_instance = 0;
}

PtkppFormat::PtkppFormat()
{
}

PtkppFormat::~PtkppFormat()
{
}

bool PtkppFormat::registerObjectHandler( int type, CreateObject func )
{
  if ( true == ohandlers_.contains(type) ) {
    warning_log << QObject::tr("Обработчик для примитива с типом %1 уже установлен").arg(type);
    return false;
  }
  ohandlers_.insert( type, func );
  return true;
}

bool PtkppFormat::registerChildHandler( int type, CreateChild func )
{
  if ( true == chandlers_.contains(type) ) {
    warning_log << QObject::tr("Обработчик для примитива с типом %1 уже установлен").arg(type);
    return false;
  }
  chandlers_.insert( type, func );
  return true;
}

bool PtkppFormat::registerLayerHandler( int type, CreateLayer func )
{
  if ( true == lhandlers_.contains(type) ) {
    warning_log << QObject::tr("Обработчик для слоя с типом %1 уже установлен").arg(type);
    return false;
  }
  lhandlers_.insert( type, func );
  return true;
}

bool PtkppFormat::registerLayerHandler( int type, CreateBaseLayer func )
{
  if ( true == blhandlers_.contains(type) ) {
    warning_log << QObject::tr("Обработчик для слоя с типом %1 уже установлен").arg(type);
    return false;
  }
  blhandlers_.insert( type, func );
  return true;
}

Object* PtkppFormat::createChild( int type, Object* o ) const
{
  if ( false == chandlers_.contains(type) ) {
    return 0;
  }
  return chandlers_[type](o);
}

Object* PtkppFormat::createObject( int type, Layer* l ) const
{
  if ( false == ohandlers_.contains(type) ) {
    return 0;
  }
  return ohandlers_[type](l);
}

Layer* PtkppFormat::createLayer( int type, Document* d ) const
{
  if ( false == lhandlers_.contains(type) ) {
    return 0;
  }
  return lhandlers_[type](d);
}

Layer* PtkppFormat::createLayer( int type, Map* d ) const
{
  if ( false == blhandlers_.contains(type) ) {
    return 0;
  }
  return blhandlers_[type](d);
}

}
}
