#ifndef METEO_COMMONS_UI_MAP_LAYER_AERODIAGH
#define METEO_COMMONS_UI_MAP_LAYER_AERODIAGH

#include <meteo/commons/ui/map/layer.h>
#include <meteo/commons/proto/meteo.pb.h>
#include <commons/funcs/tcolorgrad.h>
#include <meteo/commons/zond/placedata.h>


namespace meteo {

class Projection;

namespace map {

class Map;
class Object;
class GeoPolygon;
class GeoText;
class Property;
class Document;

class LayerAeroDiag: public Layer
{
  public:
    LayerAeroDiag( Document* map, const QString& name = QString() );
    virtual ~LayerAeroDiag();

    enum {
      Type = kLayerAero
    };
    virtual int type() const { return Type; }

    void setPlaceData(const zond::PlaceData &, const QVector<double> &levels);

    int32_t dataSize() const ;
    int32_t serializeToArray( char* arr ) const ;
    int32_t parseFromArray( const char* data );
      
  private:
    zond::PlaceData place_;
};

}
}

#endif
