#include "markerflagitem.h"
//#include "drawobject.h"

#include <qapplication.h>

#include <cross-commons/debug/tlog.h>

#include <meteo/commons/ui/map/view/mapview.h>
#include <meteo/commons/ui/map/view/mapscene.h>
#include <meteo/commons/ui/map/geopixmap.h>



namespace meteo {
namespace map {

MarkerFlagItem::MarkerFlagItem(int size, int number)
  : QGraphicsItem(),
    selected_(false),
    state_(kNormal),
    size_(size),
    number_(number),
    faxgeob(0)
{
  setFlag(QGraphicsItem::ItemSendsScenePositionChanges);
//  setAcceptHoverEvents(true);

  boundingRect_ = QRect(0,0,size,size);

  initStyles();
}

MarkerFlagItem::~MarkerFlagItem()
{
}

QRectF MarkerFlagItem::boundingRect() const
{
  QRectF r = boundingRect_;  r.setRight(r.right()+20);
  r.setTop(r.top()-25);
  r.setLeft(r.left()-15);
  return r;
}


void MarkerFlagItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
  StyleRole role = kNormalStyleRole;
  if ( selected_ ){
    role = kSelectedStyleRole;
  }

  painter->setPen(pen_[role]);
  painter->setBrush(brush_[role]);
  painter->drawArc(-10,-10, 20, 20, 0, 16*360);
  painter->drawText(boundingRect_.x()+15,boundingRect_.y(), QString::number(number_));

  painter->drawLine( QPoint( 0, -10 ), QPoint( 0, +10 ) );
  painter->drawLine( QPoint( -10, 0 ), QPoint( +10, 0 ) );
}

void MarkerFlagItem::setScenePoint(const QPoint& pos, bool recalcGeoPoint)
{
  setPos(pos);
  if ( recalcGeoPoint ) {
    updateGeoPoint();
  }
}

void MarkerFlagItem::setScenePointForRelease(const QPoint &pos, bool recalcGeoPoint)
{
  setPos(pos - boundingRect().center().toPoint());

  if ( recalcGeoPoint ) {
    updateGeoPoint();
  }
}

void MarkerFlagItem::setGeoPoint(const GeoPoint& point, bool recalcScenePoint)
{
  geoPoint_ = point;
  if ( recalcScenePoint ) {
    updateScenePoint();
  }
}

void MarkerFlagItem::setMovable(bool flag)
{
  setFlag(QGraphicsItem::ItemIsMovable, flag);
}

bool MarkerFlagItem::isMovable() const
{
  return flags() & QGraphicsItem::ItemIsMovable;
}

QPoint MarkerFlagItem::scenePoint() const
{
  return (scenePos() + boundingRect().center()).toPoint();
}

void MarkerFlagItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
  Q_UNUSED( event );
  lastMousePress_ = QDateTime::currentDateTime();
  setState(kPress);

  setCursor(Qt::SizeAllCursor);

  update();

  QGraphicsItem::mousePressEvent(event);
}

void MarkerFlagItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
  Q_UNUSED( event );

  if ( state() == kDrag ) {
    setState(kDrop);
  }

  setState(kRelease);

  int d = lastMousePress_.msecsTo(QDateTime::currentDateTime());
  if ( d < QApplication::doubleClickInterval() / 2 ) {
    setState(kClick);
  }

  unsetCursor();

  update();

  QGraphicsItem::mouseReleaseEvent(event);
}

void MarkerFlagItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
  Q_UNUSED( event );
  if ( state() == kDrag ) {
    setCursor(Qt::SizeAllCursor);
  }

  QGraphicsItem::mouseMoveEvent(event);

  if( faxgeob == 0 ){
    return;
  }
  faxgeob->setCoord(number_, geoPoint_);
}

QVariant MarkerFlagItem::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value)
{
  if ( QGraphicsItem::ItemScenePositionHasChanged == change
       && (kPress == state_ || kDrag == state_ )
       )
  {
    setState(kDrag);
    updateGeoPoint();
  }

  return QGraphicsItem::itemChange(change, value);
}

void MarkerFlagItem::initStyles()
{
  pen_[kNormalStyleRole]   = QPen(QBrush(Qt::red), 2);
  pen_[kSelectedStyleRole] = QPen(QBrush(Qt::red), 2);

  brush_[kNormalStyleRole]   = QBrush(Qt::transparent);
  brush_[kSelectedStyleRole] = QBrush(Qt::red);
}

void MarkerFlagItem::setState(MarkerFlagItem::State newState)
{
  state_ = newState;
}

void MarkerFlagItem::updateGeoPoint()
{
  MapScene* mapScene = qobject_cast<MapScene*>(scene());
  if ( 0 == mapScene ) {
    //debug_log << QObject::tr("Невозможно расчитать географическую координату узла, т.к. нет объекта MapScene");
    return;
  }
  setGeoPoint(mapScene->document()->screen2coord(pos()), false);
}

void MarkerFlagItem::updateScenePoint()
{
  MapScene* mapScene = qobject_cast<MapScene*>(scene());
  if ( 0 == mapScene ) {
    //debug_log << QObject::tr("Невозможно расчитать координату узла на сцене, т.к. нет объекта MapScene");
    return;
  }

  setScenePoint(mapScene->document()->coord2screen(geoPoint_), false);
}

void MarkerFlagItem::updateScenePointForRelease()
{
  MapScene* mapScene = qobject_cast<MapScene*>(scene());
  if ( 0 == mapScene ) {
    //debug_log << QObject::tr("Невозможно расчитать координату узла на сцене, т.к. нет объекта MapScene");
    return;
  }

  setScenePointForRelease(mapScene->document()->coord2screen(geoPoint_), false);

}

void MarkerFlagItem::setStyle(MarkerFlagItem::StyleRole role, const QPen& pen, const QBrush& brush)
{
  pen_[role] = pen;
  brush_[role] = brush;
}

void MarkerFlagItem::setColor(const QColor &color)
{
  pen_[kNormalStyleRole].setColor(color);
  brush_[kNormalStyleRole].setColor(color);
  prepareGeometryChange();
}

void MarkerFlagItem::clearFaxgeob()
{
  faxgeob = 0;
}

FaxGeoObject* MarkerFlagItem::geoobject()
{
  return  faxgeob;
}

}
}
