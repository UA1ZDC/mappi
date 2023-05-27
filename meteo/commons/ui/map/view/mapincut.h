#ifndef METEO_MAP_VIEW_MAP_INCUT_H
#define METEO_MAP_VIEW_MAP_INCUT_H

#include <qgraphicsitem.h>

#include <meteo/commons/ui/map/incut.h>

namespace meteo {
namespace map {

class Document;

class MapIncut : public QGraphicsItem
{
  Q_INTERFACES( QGraphicsItem* )
  public:
    MapIncut( Document* document, Incut* incut );
    ~MapIncut(){}

    Incut::Position position() const { return incut_->position(); }

    QRectF boundingRect() const ;
    void paint( QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = 0 );

  private:
    Document* document_;
    Incut* incut_;


};

}
}

#endif
