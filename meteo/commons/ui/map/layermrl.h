#ifndef METEO_COMMONS_UI_MAP_LAYERMRL_H
#define METEO_COMMONS_UI_MAP_LAYERMRL_H

#include "layer.h"

#include <commons/funcs/tcolorgrad.h>
#include <meteo/commons/radar/radar.h>
#include <meteo/commons/proto/map_radar.pb.h>
#include <meteo/commons/proto/weather.pb.h>
#include <meteo/commons/proto/map_radar.pb.h>
#include <meteo/commons/proto/puanson.pb.h>

class TMeteoParam;

namespace obanal {
  class TField;
}

namespace meteo {

class Projection;

namespace map {

class Map;
class Object;
class GeoText;
class Property;
class Document;

class LayerMrl: public Layer
{
  public:
    LayerMrl( Document* map, const QString& name = QString() );
    LayerMrl( Document* map, const proto::RadarColor& clr );
    LayerMrl( Document* map, const proto::RadarColor& clr, const QList<Radar>& radar );
    ~LayerMrl();

    enum {
      Type = meteo::kLayerMrl
    };
    int type() const { return Type; }

    int descriptor() const ;

    void setRadar( const QList<Radar>& radar );
    void setProtoRadar( const surf::ManyMrlValueReply& proto );

    const proto::RadarColor& radarColor() const { return radarcolor_; }

    float fieldValue( const GeoPoint& gp, bool* ok = 0 ) const ;
    bool hasValue() const { return true; }
    valLabel valueLabel() const;

    bool hasPunchRule() const ;
    puanson::proto::CellRule punchrule() const ;

    int32_t dataSize()  const ;
    int32_t serializeToArray( char* arr ) const ;
    int32_t parseFromArray( const char* data );

    static QString layerNamePattern();

//    static proto::WeatherLayer info( LayerMrl* layer );
    
  private:
    proto::RadarColor radarcolor_;
    mutable QString lastvaluestr_;
    LayerMrl( Map* map, const QString& name = QString() );

    void buildMrlPrimitives( const surf::OneMrlValueReply& mrl );
    void createCircle( const meteo::GeoPoint& center, float rad, const QString& podpis );
    void createSpeedDirection( const GeoPoint& gp, float speed, float direct );

};

}
}

#endif
