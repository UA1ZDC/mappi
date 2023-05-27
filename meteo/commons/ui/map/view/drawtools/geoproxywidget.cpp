#include "geoproxywidget.h"
#include <qgraphicssceneevent.h>
#include <cross-commons/debug/tlog.h>
#include <meteo/commons/ui/map/document.h>
#include <meteo/commons/ui/map/layer.h>
#include <meteo/commons/ui/map/view/mapscene.h>

namespace meteo {
namespace map {

GeoProxyWidget::GeoProxyWidget(QGraphicsItem* parent)
  : QGraphicsProxyWidget(parent)
{
}

void GeoProxyWidget::setGeoPos(const GeoPoint &pos)
{
  map::Document* document = doc();
  if ( nullptr == document ) {
    error_log << QObject::tr("Невозможно обновить позицию, т.к. нулевой указатель на Document");
    return;
  }
  prepareGeometryChange();
  geoPos_ = pos;
  setPos(calcScenePoint());
}

void GeoProxyWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *o, QWidget *w)
{
  setPos(calcScenePoint());
  QGraphicsProxyWidget::paint(painter,o,w);
}

Document* GeoProxyWidget::doc() const
{
  map::MapScene* mapScene = qobject_cast<map::MapScene*>(scene());
  if ( nullptr != mapScene ) {
    return mapScene->document();
  }
  return nullptr;
}

QPoint GeoProxyWidget::calcScenePoint() const
{
  map::Document* document = doc();
  if ( 0 != document ) {
    return document->coord2screen(geoPos_);
  }
  return QPoint();
}

void GeoProxyWidget::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
  grabed_ = true;
  QGraphicsProxyWidget::mousePressEvent(event);
}

void GeoProxyWidget::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
  grabed_ = false;
  if  ( false == wasMoved_ ) {
    QGraphicsProxyWidget::mouseReleaseEvent(event);
  }
  wasMoved_ = false;
}

void GeoProxyWidget::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
  Document* document = doc();
  if ( ( true == grabed_ )
        && ( false == locked_ )
       && ( nullptr != document ) ) {
    QPoint lastCart = document->screenToCartesian( event->lastScreenPos() );
    QPoint cart = document->screenToCartesian( event->screenPos() );
    QPoint diffCart = cart - lastCart;
    QPoint nowCart;
    document->projection()->F2X_one(geoPos_, &nowCart);
    nowCart += diffCart;
    document->projection()->X2F_one(nowCart, &geoPos_);
    setGeoPos(geoPos_);
    wasMoved_ = true;
  }
  QGraphicsProxyWidget::mouseMoveEvent(event);
}

void GeoProxyWidget::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
  emit enter();
  QGraphicsProxyWidget::hoverEnterEvent(event);
}

void GeoProxyWidget::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
  emit leave();
  QGraphicsProxyWidget::hoverLeaveEvent(event);
}

}
}
