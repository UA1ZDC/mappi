#include "loader.h"

#include <qobject.h>
#include <qstringlist.h>

#include <cross-commons/debug/tlog.h>
#include <meteo/commons/global/global.h>

#include "document.h"

template<> meteo::map::internal::Loader* meteo::map::Loader::_instance = 0;

namespace meteo {
namespace map {
namespace internal {

Loader::Loader()
{
}

Loader::~Loader()
{
}

bool Loader::registerDataHandler( const QString& id, FunctionType handler )
{
  if ( true == id.isEmpty() ) {
    error_log << QObject::tr("Не указан идентификатор данных");
    return false;
  }
  if ( true == handlers_.contains(id) ) {
    warning_log << QObject::tr("Обработчик для данных с идентификатором %1 уже установлен").arg(id);
//    return false;
  }
  handlers_.insert( id, handler );
  meteo::global::addGeoLoader(id);
  return true;
}

bool Loader::registerGridHandler( const QString& id, FunctionGridType handler )
{
  if ( true == id.isEmpty() ) {
    error_log << QObject::tr("Не указан идентификатор типа координатной сетки");
    return false;
  }
  if ( true == gridhandlers_.contains(id) ) {
    error_log << QObject::tr("Обработчик для координатной с идентификатором %1 уже установлен").arg(id);
    return false;
  }
  gridhandlers_.insert( id, handler );
  return true;
}

bool Loader::registerStationHandler( const QString& id, FunctionGridType handler )
{
  if ( true == id.isEmpty() ) {
    error_log << QObject::tr("Не указан идентификатор типа координатной сетки");
    return false;
  }
  if ( true == stationhandlers_.contains(id) ) {
    error_log << QObject::tr("Обработчик для слоя станций с идентификатором %1 уже установлен").arg(id);
    return false;
  }
  stationhandlers_.insert( id, handler );
  return true;
}

bool Loader::registerCitiesHandler(const QString &id, Loader::FunctionGridType handler)
{
    if ( true == id.isEmpty() ) {
      error_log << QObject::tr("Не указан идентификатор городов");
      return false;
    }
    if ( true == cityHandlers_.contains(id) ) {
      warning_log << QObject::tr("Обработчик для слоя городов с идентификатором %1 уже установлен").arg(id);
  //    return false;
    }
    cityHandlers_.insert( id, handler );
    return true;
}

bool Loader::handleData( const QString& id, Document* map )
{
  if ( true == id.isEmpty() ) {
    error_log << QObject::tr("Не указан идентификатор данных");
    return false;
  }
  if ( false == handlers_.contains(id) ) {
    error_log << QObject::tr("Обработчик для данных с идентификатором %1 не установлен").arg(id);
    return false;
  }
  map->muteEvents();
  bool res = handlers_[id](map);
  map->turnEvents();
  if ( nullptr != map->eventHandler() ) {
    map->eventHandler()->notifyLayerChanges( map->activeLayer(), LayerEvent::Added );
  }
//  map->setDefaultHash();
  if ( false == res ) {
    error_log << QObject::tr("Не удалось загрузить географическую основу формата %1")
      .arg(id);
  }
  return res;
}

bool Loader::handleGrid( const QString& id, Document* map )
{
  if ( true == id.isEmpty() ) {
    error_log << QObject::tr("Не указан идентификатор координатной сетки");
    return false;
  }
  if ( false == gridhandlers_.contains(id) ) {
    error_log << QObject::tr("Обработчик для координатной сетки с идентификатором %1 не установлен").arg(id);
    return false;
  }
  map->muteEvents();
  bool res = gridhandlers_[id](map);
  map->turnEvents();
  if ( true == res && nullptr != map->eventHandler() ) {
    map->eventHandler()->notifyLayerChanges( map->activeLayer(), LayerEvent::Added );
  }
  if ( false == res ) {
    error_log << QObject::tr("Не удалось загрузить координатную сетку формата %1")
      .arg(id);
  }
  return res;
}

bool Loader::handleStation( const QString& id, Document* map )
{
  if ( true == id.isEmpty() ) {
    error_log << QObject::tr("Не указан идентификатор координатной сетки");
    return false;
  }
  if ( false == stationhandlers_.contains(id) ) {
    error_log << QObject::tr("Обработчик для слоя со станциями  с идентификатором %1 не установлен").arg(id);
    return false;
  }
  map->muteEvents();
  bool res = stationhandlers_[id](map);
  map->turnEvents();
  if ( true == res && nullptr != map->eventHandler() ) {
    map->eventHandler()->notifyLayerChanges( map->activeLayer(), LayerEvent::Added );
  }
  if ( false == res ) {
    error_log << QObject::tr("Не удалось загрузить станции формата %1")
      .arg(id);
  }
  return res;
}

bool Loader::handleCities(const QString &id, Document *map)
{
  if ( true == id.isEmpty() ) {
    error_log << QObject::tr("Не указан идентификатор городов");
    return false;
  }
  if ( false == cityHandlers_.contains(id) ) {
    error_log << QObject::tr("Обработчик для слоя городов с идентификатором %1 не установлен").arg(id);
    return false;
  }
  map->muteEvents();
  bool res = cityHandlers_[id](map);
  map->turnEvents();
  if ( true == res && nullptr != map->eventHandler() ) {
    map->eventHandler()->notifyLayerChanges( map->activeLayer(), LayerEvent::Added );
  }
  if ( false == res ) {
    error_log << QObject::tr("Не удалось загрузить города формата %1")
      .arg(id);
  }
  return res;
}

QStringList Loader::geoLoaders() const
{
  QStringList list = handlers_.keys();
  QMap<QString,QString> tmp;
  for ( int i = 0, sz = list.size(); i < sz; ++i ) {
    tmp.insert( list[i].toLower(), list[i] );
  }
  list.clear();
  QMapIterator<QString,QString> it(tmp);
  while ( true == it.hasNext() ) {
    it.next();
    list.append( it.value() );
  }
  return list;
}

}
}
}
