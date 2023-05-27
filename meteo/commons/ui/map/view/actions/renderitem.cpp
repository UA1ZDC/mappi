#include <QtGui>

#include "renderitem.h"
#include <meteo/commons/ui/map/view/mapscene.h>
#include <meteo/commons/ui/map/layeritems.h>

namespace meteo {
namespace map {

RenderItem::RenderItem(const GeoPoint& gp, QGraphicsItem* parent, MapScene* sc):
  QGraphicsItem(parent),
  geoPoint_(gp),
  scene_(sc),
  proxy_(0),
  btn_(0)
{
  proxy_ = new QGraphicsProxyWidget(this);
  btn_ = new QToolButton;
  btn_->setVisible(false);
  btn_->setMaximumSize(QSize(16,16));
  btn_->setMinimumSize(QSize(16,16));
  btn_->setIcon(QIcon(":/meteo/icons/map/close.png"));
  btn_->setIconSize(QSize(8,8));
  proxy_->setWidget(btn_);
  scene_->document()->addItem(this);
  if ( 0 == scene() ) {
    scene_->addItem(this);
  }
  setAcceptHoverEvents(true);
}

RenderItem::~RenderItem()
{
  scene_->document()->removeItem(this);
}

QRectF RenderItem::boundingRect() const
{
  QRectF r = labelrect();
  r.setBottom( r.bottom() + 10 );
  return r;
}

void RenderItem::paint(QPainter* p, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
  QPen pen(QColor(56,123,248));
  pen.setWidth(1);
  QBrush brush(Qt::white);
  brush.setStyle(Qt::SolidPattern);
  p->setPen(pen);
  p->setBrush(brush);
  int smesh  = 11;
    QPolygonF poly;
    QRectF r = boundingRect();
    QRectF lrect = labelrect();

    poly.append(r.topLeft());
    poly.append(r.topRight());
    poly.append(r.bottomRight() - QPoint(0, smesh));
    poly.append(r.bottomLeft() - QPoint(-20, smesh));
    poly.append(r.bottomLeft() - QPoint(-13, 0));
    poly.append(r.bottomLeft() -  QPoint(-7, smesh));
    poly.append(r.bottomLeft() -  QPoint(0, smesh));

    p->drawPolygon(poly);
    //r = QRectF(labelrect().topLeft()+QPointF(1,1), labelrect().size()-QSize(2,2));

    QRectF adjustRect( lrect.topLeft()+QPointF(1,1), pixmap().size() );

    p->drawPixmap(adjustRect.toRect(), pixmap());
    proxy_->setPos(r.topLeft() + QPoint(lrect.width()-(btn_->width()+2),
                                       (lrect.height()/2-btn_->height()/2)));
}

void RenderItem::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
  btn_->setVisible(true);
  QGraphicsItem::hoverEnterEvent(event);
}

void RenderItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
  btn_->setVisible(false);
  QGraphicsItem::hoverLeaveEvent(event);
}

QToolButton* RenderItem::buttonClose()
{
  return btn_;
}

void RenderItem::setGeoPoint(const GeoPoint& gp)
{
  geoPoint_ = gp;
}

void RenderItem::prepareChange()
{
  prepareGeometryChange();
}

QRectF RenderItem::labelrect() const
{
  LayerItems* layer = scene_->document()->itemsLayer();
  Document* d = scene_->document();
  if ( 0 != layer ) {
    d = layer->document();
  }
  if ( 0 == d ) {
    d = scene_->document();
  }
  QTransform tr;
  QPointF pnt = d->coord2screen(geoPoint_);
  tr.translate(pnt.x()-13, pnt.y()-itemSize().height()-10);
  QRectF rect = QRectF( QPointF(0,0), itemSize() );
  return tr.mapRect( rect );
}

}
}
