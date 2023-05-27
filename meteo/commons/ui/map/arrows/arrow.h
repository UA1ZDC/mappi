#ifndef METEO_COMMONS_UI_MAP_ARROWS_ARROW_H
#define METEO_COMMONS_UI_MAP_ARROWS_ARROW_H

#include "../object.h"

class QPolygon;
class QPainter;
class QTransform;

namespace meteo {
namespace map {

class Property;

class Arrow
{
  public:
    Arrow( ArrowPlace place, QPolygon* gv )
      : place_(place), vector_(gv) {}
    virtual ~Arrow(){}

    virtual void render( QPainter* painter, const meteo::Property& property ) = 0;

  protected:
    const ArrowPlace place_;
    QPolygon* vector_;
};

}
}

#endif
