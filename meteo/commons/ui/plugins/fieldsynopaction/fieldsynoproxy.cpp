#include "fieldsynoproxy.h"

namespace meteo {
namespace map {


FieldSynoProxy::FieldSynoProxy(QGraphicsItem* parent, Qt::WindowFlags wFlags):
  QGraphicsProxyWidget( parent, wFlags ),
  state_(Fixed),
  press_(false)
{
  setAcceptHoverEvents(true);
  setFlag(ItemIsSelectable);
  setFlag(ItemIsMovable);
  setWindowFlags(Qt::BypassGraphicsProxyWidget);

}

FieldSynoProxy::State FieldSynoProxy::state() const
{
  return state_;
}

void FieldSynoProxy::setState(FieldSynoProxy::State state)
{
  state_ = state;
}

GeoPoint FieldSynoProxy::gp() const
{
  return gp_;
}

void FieldSynoProxy::setGeoPoint(GeoPoint gp)
{
  gp_ = gp;
}

QPoint FieldSynoProxy::clickPos() const
{
  return click_;
}

void FieldSynoProxy::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
  QGraphicsProxyWidget::mouseMoveEvent(event);
}

void FieldSynoProxy::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
  click_ = event->pos().toPoint();
  press_ = true;
  QGraphicsProxyWidget::mousePressEvent(event);

}

void FieldSynoProxy::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
  press_ = false;
  QGraphicsProxyWidget::mouseReleaseEvent(event);
}

void FieldSynoProxy::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
  if( Fixed == state() ){
      FieldSynopWidget* w = qobject_cast<FieldSynopWidget*>(widget());
      if( 0 != w ){
//        if( FieldsWidget::LockButton == w->typeWidget() ){
//          w->setLockVisible(true);
//        }
        if(  FieldSynopWidget::CloseButton == w->typeWidget()){
          w->setCloseVisible(true);
        }
        if(  FieldSynopWidget::DoubleButton == w->typeWidget()){
          w->setBtnVisible(true);
        }
      }
    }
    setZValue(zValue()+1);
    QGraphicsProxyWidget::hoverEnterEvent(event);
}

void FieldSynoProxy::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
  setZValue(zValue()-1);
  if( Fixed == state() ){
    FieldSynopWidget* w = qobject_cast<FieldSynopWidget*>(widget());
    if( 0 != w ){
      w->setBtnVisible(false);
    }
  }
  QGraphicsProxyWidget::hoverLeaveEvent(event);
}




}
}
