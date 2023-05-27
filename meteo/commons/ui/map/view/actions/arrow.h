#ifndef ARROW_H
#define ARROW_H

#include <QGraphicsLineItem>

#include "traceitem.h"
#include "textitem.h"

class QGraphicsPolygonItem;
class QGraphicsLineItem;
class QGraphicsScene;
class QRectF;
class QGraphicsSceneMouseEvent;
class QPainterPath;

namespace meteo {
namespace map {

class MapScene;

class ArrowItem : public QGraphicsLineItem
{
public:
    enum { Type = UserType + 4 };

    ArrowItem(TraceItem *startItem, TraceItem *endItem,
          QGraphicsItem *parent = 0, MapScene *sc = 0);
    ~ArrowItem();

    QPainterPath shape()  const;
    QRectF boundingRect() const;
    int type()            const { return Type; }

    TraceItem *startItem() const { return startItem_; }
    TraceItem *endItem()   const { return endItem_; }
    QString text()         const { return text_; }
    TextItem* textitem()         { return textitem_; }

    void updatePosition();
    void setColor(const QColor &acolor) { color_ = acolor; }
    void setText(const QString& str);
    void prepareChange();

    void setStartItem(TraceItem* item) { startItem_ = item; }
    void setEndItem(TraceItem* item) { endItem_ = item; }
    double angle() { return angle_; }
    QPointF centerPoint() const;


protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

private:
    TraceItem *startItem_;
    TraceItem *endItem_;
    QColor color_;
    QPolygonF arrowHead_;
    QString text_;
    //QGraphicsTextItem* textitem_;
    TextItem* textitem_;
    MapScene* scene_;
    double angle_;

    QRectF arrowrect() const ;
};

}
}

#endif
