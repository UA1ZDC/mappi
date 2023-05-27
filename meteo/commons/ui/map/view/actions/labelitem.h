#ifndef LabelItem_H
#define LabelItem_H

#include <QList>
#include <QAction>
#include <QGraphicsPixmapItem>
#include <commons/geobasis/geopoint.h>
#include "traceitem.h"

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

class LabelItem : public QGraphicsItem
{
public:
    LabelItem(const meteo::GeoPoint& gp, const QString &text,
              QGraphicsItem *parent = 0, MapScene *sc = 0 );
    ~LabelItem();
    TraceItem::PaintState paintState() const { return paint_; }
    void setDistance( double distance );
    void setText(const QString &);
    void setGeoPoint(const GeoPoint& gp);
    GeoPoint gp() const { return geoPoint_; }
    QString text() const { return text_; }
    void setClosable(bool on);
    void needDistance(bool on);
    void prepareChange();
    QRectF boundingRect() const ;
    QRectF labelrect() const ;
protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    bool collidesWithItem(const QGraphicsItem *other, Qt::ItemSelectionMode mode = Qt::IntersectsItemShape) const;

private:
    meteo::GeoPoint geoPoint_;
    MapScene* scene_;
    QString text_;
    double distance_;
    bool dist_;

    TraceItem::PaintState paint_;
    TraceItem* collideWithItem();
};

}
}
#endif
