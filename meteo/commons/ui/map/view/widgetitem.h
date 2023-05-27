#ifndef METEO_COMMONS_UI_MAP_VIEW_WIDGETITEM_H
#define METEO_COMMONS_UI_MAP_VIEW_WIDGETITEM_H

#include <qgraphicsitem.h>

class QWidget;

namespace meteo {
namespace map {

class WidgetItem : public QGraphicsItem
{
  Q_INTERFACES( QGraphicsItem )
  public:
    WidgetItem( QWidget* wgt );
    ~WidgetItem();

    QRectF boundingRect() const ;
    void paint( QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = 0 );

  private:
    QWidget* widget_;
};

}
}

#endif
