#ifndef METEO_COMMONS_UI_PLUGINS_PUANSON_PUANSONITEM_H
#define METEO_COMMONS_UI_PLUGINS_PUANSON_PUANSONITEM_H

#include <qgraphicsitem.h>

namespace meteo {
namespace map {

class Puanson;
class MapScene;

class PuansonItem : public QGraphicsItem
{
  public:
    PuansonItem( Puanson* punch, MapScene* scene );
    ~PuansonItem();

    Puanson* punch() const { return punch_; }

    QRectF boundingRect() const ;

    void paint( QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr );

  private:
    Puanson* punch_;
    MapScene* scene_;
    Puanson* copy_;

  private:
    QRectF punchrect() const ;
};

}
}

#endif
