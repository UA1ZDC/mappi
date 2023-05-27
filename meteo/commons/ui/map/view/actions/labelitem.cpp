#include <QtGui>

#include "labelitem.h"
#include <meteo/commons/ui/map/view/mapscene.h>
#include <meteo/commons/ui/map/layeritems.h>

namespace meteo {
namespace map {

LabelItem::LabelItem(const GeoPoint& gp, const QString& text,
                     QGraphicsItem* parent, MapScene* sc):
  QGraphicsItem(parent),
  geoPoint_(gp),
  scene_(sc),
  text_(text),
  distance_(0),
  dist_(true),
  paint_(TraceItem::NotPainted)
{
  if ( 0 == scene() ) {
    scene_->addItem(this);
  }
  scene_->document()->addItem(this);
}

LabelItem::~LabelItem()
{
  scene_->document()->removeItem(this);
}

QRectF LabelItem::boundingRect() const
{
  QRectF r = labelrect();
  r.setBottom( r.bottom() + 10 );
  return r;
}

void LabelItem::paint(QPainter* p, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
  paint_ = TraceItem::NotPainted;
  TraceItem* pitem = qgraphicsitem_cast<TraceItem*>(parentItem());
  if( 0 == pitem ){
    return;
  }
  if ( 0 == scene_ || 0 == scene_->document() ) {
    return;
  }
  LayerItems* layer = scene_->document()->itemsLayer();
  if ( 0 == layer ) {
    return;
  }

  TraceItem::CollideItemOption opt = pitem->collideOption();

  switch(opt){
    case TraceItem::HideMark:
    case TraceItem::HideNodeAndMark: {
      if( TraceItem::NotPainted == pitem->paintState() ){
        if( true != data(0).toBool() ){
          return;
        }
      }
      TraceItem* collideItem = collideWithItem();
      if( 0 != collideItem ){
        if( collideItem->number() < pitem->number() ){
          if( true != data(0).toBool() ){
            return;
          }
        }
      }
      QRectF prect = labelrect();
      p->save();
      p->setPen( QPen( kMAP_RAMKAPEN_DEFAULT ) );
      p->setBrush( QBrush(Qt::white) );
      QPolygonF poly;
      poly.append( prect.topLeft() );
      poly.append( prect.topRight() );
      poly.append( prect.bottomRight() );
      poly.append( QPointF( prect.center().x()+7, prect.bottom() ) );
      poly.append( QPointF( prect.center().x(), prect.bottom() + 10 ) );
      poly.append( QPointF( prect.center().x()-7, prect.bottom() ) );
      poly.append( prect.bottomLeft() );
      poly.append( prect.topLeft() );
      p->drawPolygon(poly);
      QRectF r = p->boundingRect( QRect(), Qt::AlignCenter, text_ );
      r = QRectF( prect.topLeft(), QSizeF(prect.size().width(), r.height() ) );
      p->setPen( QPen( Qt::black ) );
      p->drawText( prect, Qt::AlignCenter, text_ );
      p->restore();
      paint_ = TraceItem::Painted;
      break;
    }
    case TraceItem::NotHide:{
      QRectF prect = labelrect();
      p->save();
      p->setPen( QPen( kMAP_RAMKAPEN_DEFAULT ) );
      p->setBrush( QBrush(Qt::white) );
      QPolygonF poly;
      poly.append( prect.topLeft() );
      poly.append( prect.topRight() );
      poly.append( prect.bottomRight() );
      poly.append( QPointF( prect.center().x()+7, prect.bottom() ) );
      poly.append( QPointF( prect.center().x(), prect.bottom() + 10 ) );
      poly.append( QPointF( prect.center().x()-7, prect.bottom() ) );
      poly.append( prect.bottomLeft() );
      poly.append( prect.topLeft() );
      p->drawPolygon(poly);
      QRectF r = p->boundingRect( QRect(), Qt::AlignCenter, text_ );
      r = QRectF( prect.topLeft(), QSizeF(prect.size().width(), r.height() ) );
      p->setPen( QPen( Qt::black ) );
      p->drawText( prect, Qt::AlignCenter, text_ );
      p->restore();
      paint_ = TraceItem::Painted;
    }
  }

}

bool LabelItem::collidesWithItem(const QGraphicsItem* other, Qt::ItemSelectionMode /*mode*/) const
{
  const LabelItem* other_label = qgraphicsitem_cast<const LabelItem*>(other);
  if( 0 ==  other_label ){
    return false;
  }
  QRectF inter = labelrect().intersected(other_label->labelrect());
  if( false == inter.isEmpty() ){
    return true;
  }
  return false;
}

TraceItem*LabelItem::collideWithItem()
{
  TraceItem* pitem = qgraphicsitem_cast<TraceItem*>(parentItem());
  if( 0 == pitem ){
    return 0;
  }
  TraceItem* item = pitem->tail();
  while( item != 0){
    LabelItem* label = qgraphicsitem_cast<LabelItem*>(item->label());
    if( true == collidesWithItem(label)  && this != label && TraceItem::Painted == label->paintState() ){
      return item;
    }
    item = item->next();
  }
  return 0;
}

void LabelItem::setDistance(double distance)
{
  distance_ = distance;
  if( true == dist_ ){
    setText(QString("%1 км").arg(distance_));
  }
}

void LabelItem::setText(const QString& text)
{
  text_ = text;
  prepareGeometryChange();
}

void LabelItem::setGeoPoint(const GeoPoint& gp)
{
  geoPoint_ = gp;
}

void LabelItem::prepareChange()
{
  prepareGeometryChange();
}

QRectF LabelItem::labelrect() const
{
  LayerItems* layer = scene_->document()->itemsLayer();
  Document* d = scene_->document();
  if ( 0 != layer ) {
    d = layer->document();
  }
  if ( 0 == d ) {
    d = scene_->document();
  }
  QFont f;
  QFontMetrics fm(f);
  QSize sz(fm.width(text_)+6, fm.height()+6);

  int parent_r = 0;
  if( 0 != parentItem() ){
    parent_r = parentItem()->boundingRect().height();
  }
  QTransform tr;
  QPointF pnt = d->coord2screen(geoPoint_);
  tr.translate(pnt.x()-sz.width()/2, pnt.y()-sz.height()-parent_r/2-10);

  QRectF rect = QRectF( QPointF(0,0), sz );
  return tr.mapRect( rect );
}

}
}
