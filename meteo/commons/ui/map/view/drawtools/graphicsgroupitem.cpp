#include "graphicsgroupitem.h"
#include <cross-commons/debug/tlog.h>
#include <qgraphicssceneevent.h>

#include <meteo/commons/ui/map/document.h>
#include <meteo/commons/ui/map/view/mapscene.h>


namespace meteo {
namespace map {

GraphicsGroupItem::GraphicsGroupItem(QGraphicsItem *parent)
  : QGraphicsItemGroup(parent)
{
}

GraphicsGroupItem::~GraphicsGroupItem()
{
}

void GraphicsGroupItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
  QPointF oldPos = pos();
  QGraphicsItemGroup::mouseMoveEvent(event);
  QPointF newPos = pos();
  Document* doc = qobject_cast<MapScene*>( scene() )->document();
  if ( nullptr == doc) {
    error_log << QObject::tr("Ошибка. Отсутствует докумет.");
    return;
  }
  Projection* proj = doc->projection();
  if ( nullptr == proj ) {
    error_log << QObject::tr("Ошибка. Проекция не установлена.");
    return;
  }
  QPointF oldcart = doc->screenToCartesian(oldPos);
  QPointF newcart = doc->screenToCartesian(newPos);
  delta_ = newcart - oldcart;
  emit posChanged(delta_);
}

void GraphicsGroupItem::wheelEvent(QGraphicsSceneWheelEvent* event)
{
  Q_UNUSED(event);
}

}
}
