#ifndef RENDERITEM_H
#define RENDERITEM_H

#include <QList>
#include <QAction>
#include <QGraphicsPixmapItem>
#include <commons/geobasis/geopoint.h>
#include <qgraphicsproxywidget.h>
//#include "QToolButton.h"
#include <qtoolbutton.h>

class QPixmap;
class QGraphicsItem;
class QGraphicsScene;
class QTextEdit;
class QGraphicsSceneMouseEvent;
class QMenu;
class QGraphicsSceneContextMenuEvent;
class QPainter;
class QStyleOptionGraphicsItem;
class QWidget;
class QPolygonF;
class Arrow;

namespace meteo {
namespace map {

class MapScene;

class RenderItem : public QGraphicsItem
{
public:
    RenderItem(const meteo::GeoPoint& gp, QGraphicsItem *parent = 0, MapScene *sc = 0 );
    ~RenderItem();
    void setGeoPoint(const GeoPoint& gp);
    GeoPoint gp() const { return geoPoint_; }
    void prepareChange();
    QRectF boundingRect() const ;
    QRectF labelrect() const ;
    QToolButton* buttonClose();
    void setPixmap( const QPixmap& p ) { pix_ = p; }
    const QPixmap& pixmap() const { return pix_; }
    void setItemSize(const QSize& s) { itemsize_ = s; }
    QSize itemSize() const { return itemsize_; }

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

private:
    meteo::GeoPoint geoPoint_;
    MapScene* scene_;
    QGraphicsProxyWidget* proxy_;
    QToolButton* btn_;
    QPixmap pix_;
    QSize itemsize_;

signals:
    void close();

};

}
}
#endif
