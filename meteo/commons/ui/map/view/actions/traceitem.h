#ifndef TraceItem_H
#define TraceItem_H

#include <QList>
#include <QAction>
#include <QGraphicsPixmapItem>
#include <commons/geobasis/geopoint.h>

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

namespace meteo {
namespace map {

//class TraceProxy;
//class DiagramMarkWidget;
class MapScene;
class LabelItem;
class ArrowItem;

class TraceItem : public QGraphicsPolygonItem
{
public:
    enum                  { Type = UserType + 15 };
    enum ItemShape        { Round };
    enum PaintState       { Painted = 0, NotPainted = 1, Insure = 2, Outside = 3 };
    enum ArrowOrientation { Forward, Backward };
    enum CollideItemOption{ HideMark, HideNodeAndMark, NotHide };

    TraceItem(ItemShape ashape, const meteo::GeoPoint& gp, TraceItem* aprev = 0,
                QGraphicsItem *parent = 0, MapScene* sc = 0 );
    ~TraceItem();

    QRectF boundingRect() const ;
    QPainterPath shape() const ;

    QPixmap image() const;
    QPixmap toPixmap() const;
    int number();
    TraceItem* prev()                       { return prev_; }
    TraceItem* next()                       { return next_; }
    LabelItem* label()                      { return label_; }
    QList<ArrowItem*> arrows();
    ArrowItem* arrowIn()                    { return arrowIn_; }
    ArrowItem* arrowOut()                   { return arrowOut_; }
    int type()                        const { return Type; }
    ItemShape itemShape()             const { return shape_; }
    GeoPoint geoPoint()               const { return geoPoint_; }
    QPolygonF polygon()               const { return itemPolygon_; }
    PaintState paintState()           const { return paint_; }
    ArrowOrientation orientation()    const { return orientation_; }
    CollideItemOption collideOption() const { return collideOption_; }
    bool markVisible()                const { return markvisible_; }
    bool itemRemovable()              const { return removable_; }
    QString previousArrowText()       const { return parrowtext_;}
    QString nextArrowText()           const { return narrowtext_;}

    TraceItem* tail();
    TraceItem* head();
    TraceItem* findNearestPreviousPaintedItem();
    TraceItem* findNearestNextPaintedItem();
    TraceItem* collideWithItem();
    ArrowItem* arrowBetweenItem(TraceItem* item);

    void removeArrows();
    void hideArrows();
    void showArrows();
    void removeNextItem();
    void removePreviousItem();
    void setPreviousItem(TraceItem* aprev)          { prev_ = aprev; }
    void setNextItem(TraceItem* anext)              { next_ = anext; }
    void setGeoPoint(const meteo::GeoPoint& gp);
    void setOrientation(ArrowOrientation o)         { orientation_ = o; }
    void setCollideOption(CollideItemOption opt)    { collideOption_ = opt; }
    void setShowDistance( bool on )                 { showdistance_ = on; }
    void setItemRemovable( bool on )                { removable_ = on; }
    void setPreviousArrowText(const QString& ptext) { parrowtext_ = ptext;}
    void setNextArrowText(const QString& ntext)     { narrowtext_ = ntext;}
    void setMarkVisible( bool on );
    void setArrowIn(ArrowItem* in)                  { arrowIn_ = in; }
    void setArrowOut(ArrowItem* out)                { arrowOut_ = out; }
    ArrowItem* createArrow(TraceItem* itemStart, TraceItem* itemEnd);
    void updateDistance();
    void updateText(const QString& text);
    bool collidesWithItem(const QGraphicsItem *other, Qt::ItemSelectionMode mode = Qt::IntersectsItemShape) const;

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event);

private:
    ItemShape shape_;
    QPolygonF itemPolygon_;
    TraceItem* prev_;
    TraceItem* next_;
    meteo::GeoPoint geoPoint_;
    PaintState paint_;
    ArrowOrientation orientation_;
    CollideItemOption collideOption_;
    bool markvisible_;
    bool showdistance_;
    bool removable_;
    bool arrowVisible_;
    QString parrowtext_;
    QString narrowtext_;
    MapScene* scene_;
    LabelItem* label_;
    ArrowItem* arrowIn_;
    ArrowItem* arrowOut_;

    double distance();
    double segment();
    void updatePaintState();
};

}
}
#endif
