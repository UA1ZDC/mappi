#include <QtWidgets>

#include "traceitem.h"
#include "arrow.h"
//#include "traceproxy.h"
//#include "tracemarkwidget.h"
#include "labelitem.h"

#include <meteo/commons/ui/map/view/mapscene.h>
#include <meteo/commons/ui/map/view/mapview.h>
#include <meteo/commons/ui/map/layeritems.h>
#include <cross-commons/debug/tlog.h>

namespace meteo {
namespace map {

TraceItem::TraceItem(ItemShape ashape, const meteo::GeoPoint& gp,TraceItem* aprev,
                         QGraphicsItem *parent, MapScene* sc)
    : QGraphicsPolygonItem(parent),
      prev_(aprev),
      next_(0),
      geoPoint_(gp),
      orientation_(Forward),
      collideOption_(NotHide),
      markvisible_(true),
      showdistance_(true),
      removable_(true),
      arrowVisible_(true),
      scene_(sc),
      label_(0),
      arrowIn_(0),
      arrowOut_(0)
{
  if ( 0 == scene() ) {
    sc->addItem(this);
  }
  shape_ = ashape;

  switch (shape_) {
    case Round: {
      for( int i = 0; i <= 360; i++ ){
        itemPolygon_ << QPointF( cos(i*(M_PI/180))*12/2.0, sin(i*(M_PI/180))*12/2.0 );
      }
      break;
    }
  }
  setPolygon(itemPolygon_);

  if( 0 != prev() ){
      prev()->setNextItem(this);
  }

  if( NotHide == collideOption_ ){
      paint_ = Painted;
  }else{
      paint_ = NotPainted;
  }
  setAcceptHoverEvents(true);
  setFlag(QGraphicsItem::ItemIsMovable, true);
  label_ = new LabelItem(geoPoint_, 0, this, scene_);
  scene_->document()->addItem(this);
}

TraceItem::~TraceItem()
{
  scene_->document()->removeItem(this);
}

QRectF TraceItem::boundingRect() const
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
  tr.translate(pnt.x(), pnt.y());
  QRectF r( QPointF(0,0), QSizeF(12,12) );
  r.moveCenter(pnt);
  return r;
}

QPainterPath TraceItem::shape() const
{
  QPainterPath p;
  p.addRect( boundingRect() );
  return p;
}

void TraceItem::removeNextItem()
{
  setNextItem(0);
}

void TraceItem::removePreviousItem()
{
  setPreviousItem(0);
}

void TraceItem::setGeoPoint(const GeoPoint& gp)
{
  geoPoint_ = gp;

  if ( 0 != label_ ) { label_->setGeoPoint(gp); }
}

void TraceItem::setMarkVisible(bool on)
{
  markvisible_ = on;
  if( 0 != label_ ){
    label_->setVisible(markvisible_);
  }
}

ArrowItem* TraceItem::createArrow(TraceItem* itemStart, TraceItem* itemEnd)
{
  ArrowItem *arrow = 0;
  if( 0 != itemStart && 0 != itemEnd ){
    if( itemStart->arrowOut()!= 0 && itemEnd->arrowIn() !=0){
      arrow = itemStart->arrowOut();
      arrow->setStartItem(itemStart);
      arrow->setEndItem(itemEnd);
      arrow->updatePosition();
      arrow->show();
    }
    else{
      arrow = new ArrowItem(itemStart, itemEnd, 0, scene_);
      arrow->setColor(Qt::black);
      itemStart->setArrowOut(arrow);
      itemEnd->setArrowIn(arrow);
      arrow->setZValue(-1000.0);
    }
    arrow->updatePosition();
  }
  return arrow;
}

void TraceItem::updateDistance()
{
  if( true == showdistance_ ){
    TraceItem* item = this;
    while( 0 != item ){
      if( 0 != item ){
        double d = item->distance();
        item->updateText(QString::number(d) + QObject::tr(" км."));
      }
      item = item->next();
    }
  }
}

void TraceItem::updateText(const QString& text)
{
  if( 0 != label_ ){
    label_->setText(text);
  }
  prepareGeometryChange();
}

TraceItem* TraceItem::collideWithItem()
{
  TraceItem* item = tail();
  while( item != 0){
    if( true == collidesWithItem(item) && this != item && Painted == item->paintState() ){
      return item;
    }
    item = item->next();
  }
  return 0;
}

ArrowItem* TraceItem::arrowBetweenItem(TraceItem* item)
{
  if( 0 == item ){
    return 0;
  }
  foreach( ArrowItem* a, item->arrows() ){
    if( arrows().contains(a) ){
      return a;
    }
  }
  return 0;
}

void TraceItem::removeArrows()
{
  if( 0 != arrowIn_ ){
    arrowIn_->hide();
  }
  if( 0 != arrowOut_ ){
    arrowOut_->hide();
  }
}

void TraceItem::hideArrows()
{
  if( 0 != arrowIn_ ){
    arrowIn_->hide();
  }
  if( 0 != arrowOut_ ){
    arrowOut_->hide();
  }
  arrowVisible_ = false;
}

void TraceItem::showArrows()
{
  if( 0 != arrowIn_ ){
    arrowIn_->show();
  }
  if( 0 != arrowOut_ ){
    arrowOut_->show();
  }
  arrowVisible_ = true;
}

QPixmap TraceItem::image() const
{
  QPixmap pixmap(250, 250);
  pixmap.fill(Qt::transparent);
  QPainter painter(&pixmap);
  painter.setPen(QPen(Qt::black, 8));
  painter.translate(125, 125);
  painter.drawPolyline(itemPolygon_);
  return pixmap;
}

QPixmap TraceItem::toPixmap() const
{
  QRectF r = boundingRect();
  QPixmap pixmap(r.width(), r.height());
  pixmap.fill(Qt::transparent);
  QPainter painter(&pixmap);
  painter.drawRect(r);
  scene_->render(&painter, QRectF(), sceneBoundingRect());
  painter.end();
  return pixmap;
}

double TraceItem::distance()
{
  double d = 0;
  TraceItem* t = tail();
  TraceItem* item = t;
  while( item != this){
    item = item->next();
    d += item->segment();
  }
  return d;
}

double TraceItem::segment()
{
  if( 0 == prev() ){
    return 0;
  }
  return geoPoint().calcDistance(prev()->geoPoint());
}

int TraceItem::number()
{
  int n = 0;
  TraceItem* item = prev();
  if( 0 == item ){
    return n;
  }
  while( 0 != item ){
    item = item->prev();
    n++;
  }
  return n;
}

QList<ArrowItem*> TraceItem::arrows()
{
  QList<ArrowItem*> arr;
  arr.append(arrowIn_);
  arr.append(arrowOut_);
  return arr;
}

TraceItem*TraceItem::tail()
{
  TraceItem* item = prev();
  TraceItem* tmp;
  if( 0 == item ){
    return this;
  }
  while( 0 != item ){
    tmp = item;
    item = item->prev();
  }
  return tmp;
}

TraceItem*TraceItem::head()
{
  TraceItem* item = next();
  TraceItem* tmp;
  if( 0 == item ){
    return this;
  }
  while( 0 != item ){
    tmp = item;
    item = item->next();
  }
  return tmp;
}

TraceItem *TraceItem::findNearestPreviousPaintedItem()
{
  TraceItem* item = this;
  while( 0 != item ){
    item = item->prev();
    if( 0 != item ){
      if( Painted == item->paintState() ){
        return item;
      }
    }
  }
  return 0;
}

TraceItem *TraceItem::findNearestNextPaintedItem()
{
  TraceItem* item = this;
  while( 0 != item ){
    item = item->next();
    if( 0 != item ){
      if( Painted == item->paintState() ){
        return item;
      }
    }
  }
  return 0;
}

void TraceItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
  Q_UNUSED( widget );
  Q_UNUSED( option );

  LayerItems* layer = scene_->document()->itemsLayer();
  if ( 0 == layer ) {
    return;
  }
  Document* document = layer->document();
  if ( 0 == document ) {
    return;
  }
  switch( collideOption_ ){
    case HideNodeAndMark : {
      PaintState st = paintState();
      updatePaintState();
      if( st != paintState() ){
        if( NotPainted == paintState() ){
          setAcceptHoverEvents(false);
          removeArrows();
          if ( true == arrowVisible_ ) {
            if( Forward == orientation() ){
              createArrow(findNearestPreviousPaintedItem(), findNearestNextPaintedItem());
            }
            if( Backward == orientation() ){
              createArrow(findNearestNextPaintedItem(), findNearestPreviousPaintedItem());
            }
          }
        }
        if( Painted == paintState() ){
          setAcceptHoverEvents(true);
          removeArrows();
          TraceItem* pItem = findNearestPreviousPaintedItem();
          TraceItem* nItem = findNearestNextPaintedItem();
          if ( true == arrowVisible_ ) {
            if( Forward == orientation() ){
              ArrowItem* pa = createArrow(pItem, this);
              if( 0 != pa ){
                pa->setText(previousArrowText());
              }
              ArrowItem* na = createArrow(this, nItem);
              if( 0 != na ){
                na->setText(nextArrowText());
              }
            }
            if( Backward == orientation() ){
              createArrow(this, pItem);
              createArrow(nItem, this);
            }
          }
          if( 0 != pItem ){
            ArrowItem* a = pItem->arrowBetweenItem(nItem);
            if( 0 != a ){
              scene_->removeItem(a);
              a->hide();
            }
          }
        }
      }
      if( NotPainted == paintState() ){
        return;
      }
      painter->save();
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
      tr.translate(pnt.x(), pnt.y());

      painter->setTransform(tr, true);

      QGraphicsPolygonItem::paint(painter, option, widget);
      painter->restore();
      break;
    }
    case HideMark :
    case NotHide : {
      painter->save();
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
      tr.translate(pnt.x(), pnt.y());

      painter->setTransform(tr, true);

      QGraphicsPolygonItem::paint(painter, option, widget);
      painter->restore();
      //painter->drawRect(boundingRect());
    }
  }
}

void TraceItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
  GeoPoint gp = scene_->document()->screen2coord(event->pos());
  setGeoPoint(gp);
  setPos(0,0);
  if( 0 != label_ ){
    label_->setGeoPoint(gp);
    label_->prepareChange();
  }
  if(0!= arrowIn_){
    arrowIn_->prepareChange();
  }
  if(0!= arrowOut_){
    arrowOut_->prepareChange();
  }
  if( 0 != arrowIn_ && 0 != arrowIn_->textitem() ){
    arrowIn_->textitem()->setAngle(arrowIn_->angle());
    GeoPoint gpIn = scene_->document()->screen2coord(arrowIn_->centerPoint());
    arrowIn_->textitem()->setGeoPoint(gpIn);
  }
  if( 0 != arrowOut_ && 0 != arrowOut_->textitem() ){
    arrowOut_->textitem()->setAngle(arrowOut_->angle());
    GeoPoint gpOut = scene_->document()->screen2coord(arrowOut_->centerPoint());
    arrowOut_->textitem()->setGeoPoint(gpOut);
  }
  updateDistance();
}

void TraceItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
  QGraphicsPolygonItem::mouseReleaseEvent(event);
}

void TraceItem::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
  if( 0 != label_ ){
    label_->setData(0, true);
    setZValue(zValue()+1);
    if( false == markvisible_ ){
      label_->setVisible(true);
    }
  }
  QGraphicsPolygonItem::hoverEnterEvent(event);
}

void TraceItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
  if( 0 != label_ ){
    label_->setData(0, false);
    setZValue(zValue()-1);
    if( false == markvisible_ ){
      label_->setVisible(false);
    }
  }
  QGraphicsPolygonItem::hoverLeaveEvent(event);
}

void TraceItem::updatePaintState()
{
    TraceItem* item = collideWithItem();
    if( 0 != item ){
        paint_ = NotPainted;
    }else{
        paint_ = Painted;
    }
}

bool TraceItem::collidesWithItem(const QGraphicsItem* other, Qt::ItemSelectionMode /*mode*/) const
{
  // стандартный алгоритм очень медленный
  const TraceItem* other_Diagram = qgraphicsitem_cast<const TraceItem*>(other);
  if( 0 ==  other_Diagram ){
    return false;
  }
  QPointF other_Diagram_center = other_Diagram->scenePos() + other_Diagram->boundingRect().center();
  QPointF this_Diagram_center = scenePos() + boundingRect().center();
  QPointF delta = other_Diagram_center - this_Diagram_center;
  if( abs(delta.x()) < boundingRect().width()/2 + other_Diagram->boundingRect().width()/2  &&
      abs(delta.y()) < boundingRect().width()/2 + other_Diagram->boundingRect().width()/2 ) {
    return true;
  }
  return false;
}

}
}
