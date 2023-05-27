#include "layergrid.h"

#include <qbuffer.h>

#include <meteo/commons/global/global.h>

namespace meteo {
namespace map {

namespace {
  Layer* createLayer( Document* d )
  {
    return new LayerGrid(d);
  }
  static const bool res = singleton::PtkppFormat::instance()->registerLayerHandler( LayerGrid::Type, createLayer );
}
    
LayerGrid::LayerGrid( Document* m, const QString& n )
  : Layer( m, n )
{
  info_.set_type(kLayerGrid);
  isbase_ = true;
}

LayerGrid::~LayerGrid()
{
}

int32_t LayerGrid::dataSize() const
{
  int32_t pos = Layer::dataSize();
  QStringList latuuids;
  foreach(Object* o, latitudes_){
    latuuids.append( o->uuid() );
  }
  QStringList lonuuids;
  foreach(Object* o, longitudes_){
    lonuuids.append( o->uuid() );
  }
  if ( 0 != lonuuids.size() || 0 != latuuids.size() ) {
    QByteArray loc;
    QDataStream stream( &loc, QIODevice::WriteOnly );
    stream << latuuids;
    stream << lonuuids;
    int32_t sz = loc.size();
    pos += sizeof(sz);
    pos += sz;
  }
  else {
    int32_t sz = 0;
    pos += sizeof(sz);
  }
  return pos;
}

int32_t LayerGrid::serializeToArray( char* arr ) const
{
  int32_t pos = Layer::serializeToArray(arr);
  QStringList latuuids;
  foreach(Object* o, latitudes_){
    latuuids.append( o->uuid() );
  }
  QStringList lonuuids;
  foreach(Object* o, longitudes_){
    lonuuids.append( o->uuid() );
  }
  if ( 0 != lonuuids.size() || 0 != latuuids.size() ) {
    QByteArray loc;
    QDataStream stream( &loc, QIODevice::WriteOnly );
    stream << latuuids;
    stream << lonuuids;
    int32_t sz = loc.size();
    ::memcpy( arr + pos, &sz, sizeof(sz) );
    pos += sizeof(sz);
    ::memcpy( arr + pos, loc.data(), sz );
    pos += sz;
  }
  else {
    int32_t sz = 0;
    ::memcpy( arr + pos, &sz, sizeof(sz) );
    pos += sizeof(sz);
  }
  return pos;
}

int32_t LayerGrid::parseFromArray( const char* arr )
{
  int32_t pos = Layer::parseFromArray( arr );
  if ( -1 == pos ) {
    error_log << QObject::tr("Не удалось открыть документ");
    return pos;
  }
  int32_t sz;
  global::fromByteArray( arr + pos, &sz );
  pos += sizeof(sz);
  if ( 0 != sz ) {
    QByteArray loc( arr + pos, sz );
    QDataStream stream( &loc, QIODevice::ReadOnly );
    QStringList latuuids;
    stream >> latuuids;
    QStringList lonuuids;
    stream >> lonuuids;
    pos += sz;
    foreach(Object* o, objects_){
      QString uuid = o->uuid();
      if ( -1 != latuuids.indexOf(uuid) ) {
        addLatitude(o);
      }
      else if ( -1 != lonuuids.indexOf(uuid) ) {
        addLongitude(o);
      }
    }
  }
  return pos;
}

void LayerGrid::rmObject( Object* o )
{
  if ( 0 != latitudes_.count(o) ) {
    latitudes_.erase(o);
  }
  if ( 0 != longitudes_.count(o) ) {
    longitudes_.erase(o);
  }
  Layer::rmObject(o);
}

}
}
