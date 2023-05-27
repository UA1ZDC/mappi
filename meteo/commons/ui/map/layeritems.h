#ifndef METEO_COMMONS_UI_MAP_LAYERITEMS_H
#define METEO_COMMONS_UI_MAP_LAYERITEMS_H

#include "layer.h"

#include <commons/funcs/tcolorgrad.h>
#include <meteo/commons/radar/radar.h>
#include <meteo/commons/proto/map_radar.pb.h>
#include <meteo/commons/proto/weather.pb.h>
#include <meteo/commons/proto/map_radar.pb.h>

class QGraphicsItem;
class QStyleOptionGraphicsItem;

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

class LayerItems: public Layer
{
  public:
    LayerItems( Document* map );
    ~LayerItems();

    enum {
      Type = meteo::kLayerItems
    };
    int type() const { return Type; }

    void addItem( QGraphicsItem* item );
    void removeItem( QGraphicsItem* item );

    bool hasItem( QGraphicsItem* item ) const ;

    void render( QPainter* painter, const QRect& target, Document* document );

    QList<QGraphicsItem*> items() const { return items_; }
    
  private:
    QList<QGraphicsItem*> items_;

    bool hasItem( QGraphicsItem* item, const QList<QGraphicsItem*>& childs ) const ;

    void renderItem( QPainter* painter, QGraphicsItem* item, QStyleOptionGraphicsItem* style, QList<QGraphicsItem*>* already );
};

}
}

#endif
