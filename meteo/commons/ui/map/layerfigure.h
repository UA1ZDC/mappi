#ifndef METEO_COMMONS_UI_MAP_LAYER_FIGURE_H
#define METEO_COMMONS_UI_MAP_LAYER_FIGURE_H

#include "layer.h"
#include <meteo/commons/proto/meteo.pb.h>
#include <meteo/commons/proto/surface_service.pb.h>
#include <meteo/commons/proto/meteotablo.pb.h>
#include <commons/meteo_data/meteo_data.h>

namespace meteo {

class Projection;

namespace map {

class WeatherFigure;

class Map;
class Object;
class GeoPolygon;
class GeoText;
class Property;
class Document;
class Puanson;

class LayerFigure: public Layer
{
  public:
    LayerFigure( Document* map, const tablo::Settings& pattern );
    LayerFigure( Document* map );
    virtual ~LayerFigure();

    enum {
      Type = kLayerFigure
    };
    int type() const { return Type; }

    void render( QPainter* painter, const QRect& target, Document* document );

    void setData( const surf::DataReply& data );
    void setData(const std::unordered_set<Puanson *> &data );
    void clearData();
    //
//    LayerMenu* layerMenu();

    const tablo::Settings& pattern() const { return pattern_; }
    void setPattern( const tablo::Settings& p );

    int32_t dataSize()  const ;
    int32_t serializeToArray( char* arr ) const ;
    int32_t parseFromArray( const char* data );

  private:
    tablo::Settings pattern_;
    QMap< tablo::Color, QList<WeatherFigure*> > figures_;

    LayerFigure( Map* map, const QString& name = QString() );
    bool addFigure(TMeteoData *md);



  friend class WeatherFigure;
};

}
}

#endif
