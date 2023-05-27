#ifndef METEO_COMMONS_UI_MAP_ACTIONS_PUANSON_NAVYITEM_H
#define METEO_COMMONS_UI_MAP_ACTIONS_PUANSON_NAVYITEM_H

#include <qgraphicsitem.h>

namespace meteo {
namespace map {

class Puanson;

class NavyItem : public QGraphicsItem
{
  public:
    NavyItem( Puanson* origin );
    ~NavyItem();

    void setOrigin( Puanson* origin );

    QRectF boundingRect() const ;
    void paint( QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = 0 );

  private:
    Puanson* origin_;
    Puanson* copy_;
};

}
}

#endif
