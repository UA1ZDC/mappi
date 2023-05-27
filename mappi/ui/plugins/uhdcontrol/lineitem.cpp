#include "lineitem.h"

#include <qpainter.h>

#include <cross-commons/debug/tlog.h>
#include <meteo/commons/ui/map/layeritems.h>
#include <meteo/commons/ui/map/view/mapscene.h>

namespace meteo {
namespace spectr {

LineItem::LineItem(map::LayerItems* layer, QGraphicsItem* parent, QGraphicsScene* scene)
  : QGraphicsItem(parent)
{
  Q_UNUSED(scene);
  layer_ = layer;

  setFlag(QGraphicsItem::ItemSendsScenePositionChanges);
}

LineItem::~LineItem()
{
  if ( 0 != layer_ ) {
    layer_->removeItem(this);
  }
}

QRectF LineItem::boundingRect() const
{
  map::Document* doc = document();
  if ( 0 == doc ) {
    debug_log << QObject::tr("Невозможно рассчитать boundingRect, т.к. нулевой указатель на Document");
    return QRectF();
  }

  QRect mapRect = doc->mapRect();
  QPoint p = doc->coord2screen(geoPos_);

  QRectF r(-2,-2,4,4);
  r.setTop(mapRect.top() - p.y());
  r.setBottom(mapRect.bottom() + p.y());

  if ( !doc->isStub() ) { r.moveCenter(p - scenePos()); }

  return r;
}

void LineItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
  Q_UNUSED( option );
  Q_UNUSED( widget );

  map::Document* doc = document();
  if ( 0 == doc ) {
    debug_log << QObject::tr("Невозможно нарисовать объект, т.к. нулевой указатель на Document");
    return;
  }

  bool antialiasing = painter->testRenderHint(QPainter::Antialiasing);
  painter->setRenderHint(QPainter::Antialiasing, false);

  if ( doc->isStub() ) {
    painter->save();
    painter->translate(calcScenePoint());
  }

  QPen pen;
  pen.setWidth(1);
  pen.setColor(QColor(120,120,120));
  painter->setPen(pen);

  QRectF r = boundingRect();
  QPointF beg = r.center();
  QPointF end = beg;

  beg.ry() = r.top();
  end.ry() = r.bottom();

  painter->drawLine(beg, end);

  if ( doc->isStub() ) {
    painter->restore();
  }

  painter->setRenderHint(QPainter::Antialiasing, antialiasing);
}

void LineItem::setGeoPos(const GeoPoint& pos)
{
  map::Document* doc = document();
  if ( 0 == doc ) {
    debug_log << QObject::tr("Невозможно обновить позицию, т.к. нулевой указатель на Document");
    return;
  }

  prepareGeometryChange();

  setPos(mapFromScene(doc->coord2screenf(pos)));
  geoPos_ = pos;
}

QPoint LineItem::calcScenePoint() const
{
  map::Document* doc = document();
  if ( 0 != doc ) {
    return doc->coord2screen(geoPos_);
  }
  return QPoint();
}

QVariant LineItem::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value)
{
  if ( QGraphicsItem::ItemScenePositionHasChanged == change ) {
    map::Document* doc = document();
    if ( 0 != doc ) {
      geoPos_ = doc->screen2coord(scenePos());
    }
    else {
      debug_log << QObject::tr("Невозможно рассчитать географическую координату: нулевой указатель на Document");
    }
  }

  return QGraphicsItem::itemChange(change, value);
}

map::Document* LineItem::document() const
{
  if ( 0 != layer_ ) {
    return layer_->document();
  }

  map::MapScene* mapScene = qobject_cast<map::MapScene*>(scene());
  if ( 0 != mapScene ) {
    return mapScene->document();
  }

  return 0;
}

} // spectr
} // meteo
