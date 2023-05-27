#include "coordproxywidget.h"

#include <qgraphicssceneevent.h>
#include <qpainter.h>

#include "../mapscene.h"

namespace meteo{
namespace map{

BaloonProxyWidget::BaloonProxyWidget(QGraphicsItem* parent, Qt::WindowFlags wFlags) :
  QGraphicsProxyWidget( parent, wFlags ),
  state_(Fixed),
  press_(false),
  on_hover(false)
{
  setAcceptHoverEvents(true);
  setFlag(ItemIsSelectable);
  setFlag(ItemIsMovable);
  setWindowFlags(Qt::BypassGraphicsProxyWidget);
 // setAttribute(Qt::WA_DeleteOnClose);
  //connect(this, SIGNAL(lock(bool)), SLOT(slotLock(bool)));
}

BaloonProxyWidget::~BaloonProxyWidget(){
  MapScene* ms = qobject_cast< MapScene* >(scene()) ;
  if(ms){
   ms->removeVisibleItem(this);
  }
  if(widget()){
    widget()->deleteLater();
  }
}

BaloonProxyWidget::State BaloonProxyWidget::state() const
{
  return state_;
}

void BaloonProxyWidget::setState(BaloonProxyWidget::State astate)
{
  state_ = astate;
  prepareGeometryChange();
}

GeoPoint BaloonProxyWidget::gp() const
{
  return gp_;
}

void BaloonProxyWidget::setGeoPoint(GeoPoint gp)
{
  gp_ = gp;
}

QPoint BaloonProxyWidget::clickPos() const
{
  return click_;
}

void BaloonProxyWidget::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
  QGraphicsProxyWidget::mouseMoveEvent(event);
}

void BaloonProxyWidget::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
{
    CoordWidget* w = qobject_cast<CoordWidget*>(widget());
    if( 0 != w ){
      w->setBtnVisible(false);
     // debug_log<<this<<state();
      switch(state() ){
        case Fixed:
          w->setLockVisible(true);
          w->setCloseVisible(true);
          break;
        case Moving:
          break;
        case Pointer:
          w->setCloseVisible(true);
          break;
      }
    }
  }
  setZValue(zValue()+1);
  on_hover=true;
  QGraphicsProxyWidget::hoverEnterEvent(event);
}

void BaloonProxyWidget::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
  setZValue(zValue()-1);
  CoordWidget* w = qobject_cast<CoordWidget*>(widget());
  if( 0 != w ){
    w->setBtnVisible(false);
  }
  on_hover=false;
  QGraphicsProxyWidget::hoverLeaveEvent(event);
}

bool BaloonProxyWidget::onHover ( ) const {
 if(!isVisible()) return false;
 return on_hover;
}

void BaloonProxyWidget::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
  click_ = event->pos().toPoint();
  press_ = true;
  QGraphicsProxyWidget::mousePressEvent(event);
}

void BaloonProxyWidget::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
  press_ = false;
  QGraphicsProxyWidget::mouseReleaseEvent(event);
}


void BaloonProxyWidget::adjustPos(const QPointF &pos){
  if ( false == leftOrient_ ) {
    setPos(pos - QPoint(13, boundingRect().height()));
  }
  else {
    setPos(pos - QPoint(-13, boundingRect().height()));
  }
}


void BaloonProxyWidget::paint(QPainter* p, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
  QPen pen(QColor(56,123,248));
  pen.setWidth(1);
  QBrush brush(Qt::white);
  brush.setStyle(Qt::SolidPattern);
  p->setPen(pen);
  p->setBrush(brush);
  int smesh  = 11;
  if(BaloonProxyWidget::Moving == state()||BaloonProxyWidget::Pointer == state() ) {
    QPolygonF poly;
    if ( false == leftOrient_ ) {
      poly.append(boundingRect().topLeft());
      poly.append(boundingRect().topRight());
      poly.append(boundingRect().bottomRight() - QPoint(0, smesh));
      poly.append(boundingRect().bottomLeft() - QPoint(-20, smesh));
      poly.append(boundingRect().bottomLeft() - QPoint(-13, 0));
      poly.append(boundingRect().bottomLeft() -  QPoint(-7, smesh));
      poly.append(boundingRect().bottomLeft() -  QPoint(0, smesh));
    }
    else {
      poly.append(boundingRect().topLeft());
      poly.append(boundingRect().topRight());
      poly.append(boundingRect().bottomRight() - QPoint(0, smesh));
      poly.append(boundingRect().bottomRight() - QPoint(7, smesh));
      poly.append(boundingRect().bottomRight() - QPoint(13, 0));
      poly.append(boundingRect().bottomRight() -  QPoint(20, smesh));
      poly.append(boundingRect().bottomLeft() -  QPoint(0, smesh));
    }
//    debug_log<<" 1 w"<<width()<<" h"<< height()<<rect();
    p->drawPolygon(poly);
  } else {
    p->drawRect(0,0, boundingRect().width(), boundingRect().height()-smesh );
  }
  // QDialog::paintEvent(e);
  QGraphicsProxyWidget::paint(p,option,widget);
  
}

}
}
