#ifndef METEO_COMMONS_UI_MAP_LAYER_SIGWX_H
#define METEO_COMMONS_UI_MAP_LAYER_SIGWX_H

#include "layer.h"
#include <meteo/commons/proto/meteo.pb.h>
#include <meteo/commons/proto/surface_service.pb.h>
#include <meteo/commons/proto/puanson.pb.h>
#include <commons/funcs/tcolorgrad.h>

class TMeteoData;

namespace meteo {

class Projection;

namespace map {

class Puanson;

class Map;
class Object;
class GeoPolygon;
class GeoText;
class Property;
class Document;

class LayerSigwx: public Layer
{
  public:
    LayerSigwx( Document* map, const puanson::proto::Puanson& punch );
    LayerSigwx( Document* map );
    virtual ~LayerSigwx();

    enum {
      Type = kLayerSigwx
    };
    int type() const { return Type; }

    void setData( const sigwx::SigWx& data );
    void setCloud( const sigwx::SigCloud& cloud );
    void setTurbo( const sigwx::SigTurbulence& turbo );
    void setJet( const sigwx::SigJetStream& jet );
    void setFront( const sigwx::SigFront& front );
    void setTropo( const sigwx::SigTropo& tropo );
    void setVulkan( const sigwx::SigVolcano& vulkan );
    void setStorm( const sigwx::SigStorm& storm );

    int32_t dataSize()  const ;
    int32_t serializeToArray( char* arr ) const ;
    int32_t parseFromArray( const char* data );

  private:
    LayerSigwx( Map* map, const QString& name = QString() );
};

}
}

#endif
