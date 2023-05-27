#ifndef METEO_COMMONS_UI_MAP_ARROWS_SIMPLE_H
#define METEO_COMMONS_UI_MAP_ARROWS_SIMPLE_H

#include <commons/geobasis/geopoint.h>

#include "arrow.h"

class QPoint;

namespace meteo {
namespace map {

class Property;

class SimpleArrow : public Arrow
{
  public:
    SimpleArrow( ArrowPlace place, QPolygon* gv );
    ~SimpleArrow();

    void render( QPainter* painter, const meteo::Property& property );

    bool closurePoints( QPoint* begin, QPoint* end );

  private:
    int width_;
    int length_;
};

}
}

#endif
