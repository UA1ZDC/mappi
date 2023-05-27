#ifndef METEO_COMMONS_UI_MAP_LAYER_PUNCH_H
#define METEO_COMMONS_UI_MAP_LAYER_PUNCH_H

#include "layer.h"
#include <meteo/commons/proto/meteo.pb.h>
#include <meteo/commons/proto/surface_service.pb.h>
#include <meteo/commons/proto/puanson.pb.h>
#include <commons/funcs/tcolorgrad.h>
#include <commons/geobasis/geodata.h>

namespace obanal {
  class TField;
}

namespace meteo {

class Projection;
class Puanson;

namespace map {

class Puanson;

class Map;
class Object;
class GeoPolygon;
class GeoText;
class Property;
class Document;

class LayerPunch: public Layer
{
  public:
    LayerPunch( Document* map, const puanson::proto::Puanson& punch );
    LayerPunch( Document* map );
    virtual ~LayerPunch();

    enum {
      Type = meteo::kLayerPunch
    };
    int type() const { return Type; }

    void setData( const surf::DataReply& data );

    LayerMenu* layerMenu();

    const puanson::proto::Puanson& punch() const { return punch_; }
    void setPunch( const puanson::proto::Puanson& p );
    const std::string& punchString() const ;
    void getGeoData(meteo::GeoData *all_data, int64_t desr);

    int32_t dataSize()  const ;
    int32_t serializeToArray( char* arr ) const ;
    int32_t parseFromArray( const char* data );
    //virtual bool hasValue() const { return true; }

  private:
    puanson::proto::Puanson punch_;
    mutable std::string cached_punch_string_;
    LayerPunch( Map* map, const QString& name = QString() );

  friend class Puanson;
};

}
}

#endif
