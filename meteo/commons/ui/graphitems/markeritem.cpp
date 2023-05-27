#include "markeritem.h"

#include <qapplication.h>

#include <cross-commons/debug/tlog.h>

#include <meteo/commons/ui/map/layeritems.h>
#include <meteo/commons/ui/map/view/mapscene.h>
#include <meteo/commons/ui/map/view/mapview.h>

namespace meteo {

MarkerItem::MarkerItem(int size, int round, QGraphicsItem* parent)
  : QGraphicsObject(parent),
    hovered_(false),
    selected_(false),
    state_(kNormal),
    round_(round)
{
  callback_ = nullptr;
  layer_    = nullptr;

  boundingRect_ = QRectF(0,0,size,size);
  boundingRect_.moveCenter(pos());

  setFlag(QGraphicsItem::ItemSendsScenePositionChanges);
  setAcceptHoverEvents(true);

  initStyles();
}

MarkerItem::~MarkerItem()
{
  delete callback_;

  if ( nullptr != layer_ ) {
    layer_->removeItem(this);
  }
}

QRectF MarkerItem::boundingRect() const
{
  QRectF r = boundingRect_;

  map::Document* doc = document();
  if ( nullptr != doc && !doc->isStub() ) {
    QPointF p = calcScenePoint() - scenePos();
    r.moveCenter(p);
  }

  return r;
}

void MarkerItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
  map::Document* doc = document();
  if ( nullptr == doc ) {
    debug_log << QObject::tr("Невозможно нарисовать объект, т.к. нулевой указатель на Document");
    return;
  }

  StyleRole role = kNormalStyleRole;
  if ( hovered_ ) {
    role = kHoverStyleRole;
  }
  if ( selected_ ){
    role = kSelectedStyleRole;
  }

  if ( doc->isStub() ) {
    painter->save();
    painter->translate(calcScenePoint());
  }

  painter->setPen(pen_[role]);
  painter->setBrush(brush_[role]);
  painter->drawRoundedRect(boundingRect(), round_, round_);

  if ( doc->isStub() ) {
    painter->restore();
  }
}

void MarkerItem::setGeoPos(const GeoPoint& pos)
{
  map::Document* doc = document();
  if ( nullptr == doc ) {
    debug_log << QObject::tr("Невозможно обновить позицию, т.к. нулевой указатель на Document");
    return;
  }

  prepareGeometryChange();

  setPos(doc->coord2screenf(pos));
  geoPos_ = pos;
}

void MarkerItem::setMovable(bool flag)
{
  setFlag(QGraphicsItem::ItemIsMovable, flag);
}

bool MarkerItem::isMovable() const
{
  return flags() & QGraphicsItem::ItemIsMovable;
}

GeoPoint MarkerItem::geoPos() const
{
  return geoPos_;
}

QPoint MarkerItem::calcScenePoint() const
{
  map::Document* doc = document();
  if ( nullptr != doc ) {
    return doc->coord2screen(geoPos_);
  }
  return QPoint();
}

void MarkerItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
  Q_UNUSED( event );

  lastMousePress_ = QDateTime::currentDateTime();
  setState(kPress);

  setCursor(Qt::SizeAllCursor);

  update();

  QGraphicsItem::mousePressEvent(event);
}

void MarkerItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
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

void MarkerItem::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
  Q_UNUSED( event );

  hovered_ = true;
  setState(kHover);

  QGraphicsItem::hoverEnterEvent(event);
}

void MarkerItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
  Q_UNUSED( event );

  hovered_ = false;
  setState(kNormal);

  unsetCursor();

  QGraphicsItem::hoverLeaveEvent(event);
}

void MarkerItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
  Q_UNUSED( event );
  if ( state() == kDrag ) {
    setCursor(Qt::SizeAllCursor);
  }

  QGraphicsItem::mouseMoveEvent(event);
}

QVariant MarkerItem::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value)
{
  if ( ( QGraphicsItem::ItemScenePositionHasChanged == change )
       && ( (kGroupDrag == state_) || (kDrag == state_) ) ) {
    map::Document* doc = document();
    if ( nullptr != doc ) {
      geoPos_ = doc->screen2coord(scenePos());
      emit changePos();
    }
    else {
      debug_log << QObject::tr("Невозможно рассчитать географическую координату: нулевой указатель на Document");
    }
  }

  if ( QGraphicsItem::ItemScenePositionHasChanged == change && (kPress == state_ || kDrag == state_ ) ) {
    setState(kDrag);
  }

  if ( QGraphicsItem::ItemSceneHasChanged == change ) {
  }

  return QGraphicsItem::itemChange(change, value);
}

void MarkerItem::initStyles()
{
  pen_[kNormalStyleRole]   = QPen(QBrush(Qt::black), 2);
  pen_[kHoverStyleRole]    = QPen(QBrush(Qt::black), 2);
  pen_[kSelectedStyleRole] = QPen(QBrush(Qt::black), 2);

  brush_[kNormalStyleRole]   = QBrush(Qt::white);
  brush_[kHoverStyleRole]    = QBrush(Qt::red);
  brush_[kSelectedStyleRole] = QBrush(Qt::red);
}

void MarkerItem::setState(MarkerItem::State newState)
{
  state_ = newState;

  if ( nullptr != callback_ ) {
    callback_->run(this);
  }
}

map::Document* MarkerItem::document() const
{
  if ( nullptr != layer_ ) {
    return layer_->document();
  }

  map::MapScene* mapScene = qobject_cast<map::MapScene*>(scene());
  if ( nullptr != mapScene ) {
    return mapScene->document();
  }

  return nullptr;
}

void MarkerItem::setStyle(MarkerItem::StyleRole role, const QPen& pen, const QBrush& brush)
{
  pen_[role] = pen;
  brush_[role] = brush;
}

void MarkerItem::setCallback(MarkerItemClosure* callback)
{
  delete callback_;
  callback_ = callback;
}

} // meteo
