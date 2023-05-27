#include "layerpunch.h"

#include <qdebug.h>
#include <quuid.h>
#include <qpainter.h>
#include <qelapsedtimer.h>

#include <cross-commons/debug/tlog.h>
#include <sql/psql/psqlquery.h>
#include <commons/geobasis/projection.h>
#include <meteo/commons/global/global.h>

#include "map.h"
#include "document.h"
#include "object.h"
#include "geopolygon.h"
#include "geotext.h"
#include "puanson.h"
#include <meteo/commons/global/weatherloader.h>
#include "layermenu.h"

namespace meteo {
namespace map {

namespace {
  Layer* createLayer( Document* d )
  {
    return new LayerPunch(d);
  }
  static const bool res = singleton::PtkppFormat::instance()->registerLayerHandler( LayerPunch::Type, createLayer );
}

LayerPunch::LayerPunch( Document* d, const puanson::proto::Puanson& p )
  : Layer(d)
{
  info_.set_type(kLayerPunch);
  setPunch(p);
}

LayerPunch::LayerPunch( Document* d )
  : Layer(d)
{
  info_.set_type(kLayerPunch);
}

LayerPunch::~LayerPunch()
{
}

void LayerPunch::setData( const surf::DataReply& data )
{
  bool muted = true;
  if ( nullptr != document() ) {
    muted = document()->muted();
    if ( false == muted ) {
      document()->muteEvents();
    }
  }
  info_.clear_meteo_descr();
  for(auto r :punch_.rule()){
      info_.add_meteo_descr(r.id().descr());
  }
  TMeteoData md;
  for ( int i = 0, sz = data.meteodata_size(); i < sz; ++i ) {
    const std::string& str = data.meteodata(i);
    QByteArray arr(str.data(), str.size() );
    md << arr;
    bool fl = false;
    for ( int i = 0, sz = punch_.rule_size(); i < sz; ++i ) {
      QString name = QString::fromStdString( punch_.rule(i).id().name() );
      if ( false == md.hasParam(name) ) {
        continue;
      }
      fl = true;
      break;
    }
    if ( false == fl ) {
      continue;
    }
    GeoPoint gp;
    fl = TMeteoDescriptor::instance()->getCoord( md, &gp );
    if ( false == fl ) {
      continue;
    }
    meteo::map::Puanson* p = new meteo::map::Puanson(this);
    p->setPunch(punch_);
    p->setMeteodata(md);
    p->setSkelet(gp);
  }
  if ( nullptr != document() ) {
    if ( false == muted ) {
      document()->turnEvents();
    }
  }
  if(0 < data.meteodata_size()) {
      layer_data_type_ = srcData;
    }
}

void LayerPunch::getGeoData(meteo::GeoData *all_data, int64_t descr){
  std::unordered_set<Puanson*> list = objectsByType<Puanson*>();
  for ( auto g: list ) {
    g->setPunch(punch_);
  }
  for(auto puans: list){
    const TMeteoData& md = puans->meteodata();
    GeoPoint gpt;
    if(false == TMeteoDescriptor::instance()->getCoord(md, &gpt)) { continue; }
    const TMeteoParam& param = md.getParam(descr);
    const TMeteoParam& id = md.getParam(TMeteoDescriptor::instance()->descriptor("text"));
    if(control::SPECIAL_VALUE <= param.quality() )
     //  false == checkParam(descr, param.value(), req.level_p(), req.type_level()) )
    { continue; }
    float v = param.value();
    all_data->append(MData(gpt, v, 1, id.code()));
  }

}



void LayerPunch::setPunch( const puanson::proto::Puanson& p )
{

  cached_punch_string_.clear();
  punch_.CopyFrom(p);
  std::unordered_set<Puanson*> list = objectsByType<Puanson*>();
  for ( auto g: list ) {
    g->setPunch(punch_);
  }
  repaint();
}

const std::string& LayerPunch::punchString() const
{
  if ( 0 == cached_punch_string_.size() ) {
    punch_.SerializeToString( &cached_punch_string_ );
  }
  return cached_punch_string_;
}

int32_t LayerPunch::dataSize() const
{
  int32_t pos = Layer::dataSize();
  std::string str;
  if ( true == punch_.IsInitialized() ) {
    punch_.SerializeToString( &str );
  }
  int32_t sz = str.size();
  pos += sizeof(sz);
  pos += sz;
  return pos;
}

int32_t LayerPunch::serializeToArray( char* arr ) const
{
  int32_t pos = Layer::serializeToArray(arr);
  std::string str;
  if ( true == punch_.IsInitialized() ) {
    punch_.SerializeToString( &str );
  }
  int32_t sz = str.size();
  ::memcpy( arr + pos, &sz, sizeof(sz) );
  pos += sizeof(sz);
  ::memcpy( arr + pos, str.data(), sz );
  pos += sz;
  return pos;
}

int32_t LayerPunch::parseFromArray( const char* arr )
{
  int32_t pos = Layer::parseFromArray(arr);
  int32_t sz;
  global::fromByteArray( arr + pos, &sz );
  pos += sizeof(sz);
  if ( 0 != sz ) {
    punch_.ParseFromArray( arr + pos, sz );
  }
  pos += sz;
  return pos;
}

LayerMenu* LayerPunch::layerMenu()
{
  if ( nullptr == menu_ ) {
    menu_ = new PunchMenu(this);
  }
  return menu_;
}

}
}
