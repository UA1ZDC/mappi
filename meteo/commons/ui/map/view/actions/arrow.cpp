#include <QtGui>

#include <math.h>
#include <cross-commons/debug/tlog.h>
#include <meteo/commons/ui/map/view/mapscene.h>
#include <meteo/commons/ui/map/layeritems.h>

#include "arrow.h"

namespace meteo {
namespace map {

ArrowItem::ArrowItem(TraceItem *startItem, TraceItem *endItem,
         QGraphicsItem *parent, MapScene *sc)
    : QGraphicsLineItem(parent),
      textitem_(0),
      scene_(sc),
      angle_(0)
{
  if ( 0 == scene() ) {
    scene_->addItem(this);
  }
  startItem_ = startItem;
  endItem_ = endItem;
  setFlag(QGraphicsItem::ItemIsSelectable, false);
  color_ = Qt::black;
  setPen(QPen(color_, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
  scene_->document()->addItem(this);
}

ArrowItem::~ArrowItem()
{
  scene_->document()->removeItem(this);
}

QRectF ArrowItem::boundingRect() const
{
  return arrowrect();
}

QPainterPath ArrowItem::shape() const
{
    QPainterPath path = QGraphicsLineItem::shape();
    path.addPolygon(arrowHead_);
    return path;
}

void ArrowItem::updatePosition()
{
  QLineF nline(mapFromItem(startItem_, 0, 0), mapFromItem(endItem_, 0, 0));
  setLine(nline);
  prepareGeometryChange();
}

void ArrowItem::setText(const QString& str)
{
  if( true == str.isEmpty() ){
    return;
  }
  text_ = str;
  if( 0 == textitem_ ){
    meteo::GeoPoint gp = scene_->document()->screen2coord(arrowrect().center());
    QPointF pnt1 = scene_->document()->coord2screen(startItem_->geoPoint());
    QPointF pnt2 = scene_->document()->coord2screen(endItem_->geoPoint());
    setLine(QLineF(pnt1, pnt2));
    double angle = ::acos(line().dx() / line().length());
    if (line().dy() >= 0){
      angle = (M_PI * 2) - angle;
    }
    double deg = angle*(180/M_PI);
    if( deg  <= 90 || deg >= 270 ){
      angle_ = -deg;
    }
    else{
      angle_ = 180-deg;
    }
    textitem_ = new TextItem(gp, text_, this, scene_);
    textitem_->setAngle(angle_);
    textitem_->setGeoPoint(gp);
  }
  textitem_->setText(str);
  if( 0 != startItem_ ){
    startItem_->setNextArrowText(str);
  }
  if( 0 != endItem_ ){
    endItem_->setPreviousArrowText(str);
  }
}

void ArrowItem::prepareChange()
{
  prepareGeometryChange();
}

QPointF ArrowItem::centerPoint() const
{
  return arrowrect().center();
}

void ArrowItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
  Q_UNUSED( painter );
  if( 0 == startItem_ || 0 == endItem_ ){
    return;
  }
  LayerItems* layer = scene_->document()->itemsLayer();
  Document* d = scene_->document();
  if ( 0 != layer ) {
    d = layer->document();
  }
  if ( 0 == d ) {
    d = scene_->document();
  }
  QPointF pnt1 = d->coord2screen(startItem_->geoPoint());
  QPointF pnt2 = d->coord2screen(endItem_->geoPoint());

  if ( startItem_->collidesWithItem(endItem_) ) {
    return;
  }
  QPen myPen = pen();
  myPen.setColor(color_);
  qreal arrowSize = 6;
  painter->save();
  painter->setPen(myPen);
  painter->setBrush(color_);

  QLineF centerLine(pnt1, pnt2);
  QPolygonF endPolygon = endItem_->polygon();
  QPointF p1 = endPolygon.first() + pnt2;
  QPointF p2;
  QPointF intersectPoint;
  QLineF polyLine;
  for (int i = 1; i < endPolygon.count(); ++i) {
    p2 = endPolygon.at(i) + pnt2;
    polyLine = QLineF(p1, p2);
    QLineF::IntersectType intersectType =
        polyLine.intersect(centerLine, &intersectPoint);
    if (intersectType == QLineF::BoundedIntersection)
      break;
    p1 = p2;
  }
  setLine(QLineF(intersectPoint, pnt1));
  double angle = ::acos(line().dx() / line().length());
  if (line().dy() >= 0){
    angle = (M_PI * 2) - angle;
  }

  QPointF arrowP1 = line().p1() + QPointF(sin(angle + M_PI / 3) * arrowSize,
                                          cos(angle + M_PI / 3) * arrowSize);
  QPointF arrowP2 = line().p1() + QPointF(sin(angle + M_PI - M_PI / 3) * arrowSize,
                                          cos(angle + M_PI - M_PI / 3) * arrowSize);
  arrowHead_.clear();
  arrowHead_ << line().p1() << arrowP1 << arrowP2;

  if( 0 != textitem_ ){
    double deg = angle*(180/M_PI);
    if( deg  <= 90 || deg >= 270 ){
      angle_ = -deg;
    }
    else{
      angle_ = 180-deg;
    }
  }
  painter->drawLine(line());
  painter->drawPolygon(arrowHead_);
  if ( isSelected() ) {
    painter->setPen(QPen(color_, 1, Qt::DashLine));
    QLineF myLine = line();
    myLine.translate(0, 4.0);
    painter->drawLine(myLine);
    myLine.translate(0,-8.0);
    painter->drawLine(myLine);
  }
  painter->restore();
}

QRectF ArrowItem::arrowrect() const
{
  LayerItems* layer = scene_->document()->itemsLayer();
  Document* d = scene_->document();
  if ( 0 != layer ) {
    d = layer->document();
  }
  if ( 0 == d ) {
    d = scene_->document();
  }
  QPointF pnt1 = d->coord2screen(startItem_->geoPoint());
  QPointF pnt2 = d->coord2screen(endItem_->geoPoint());
  qreal extra = (pen().width() + 12) / 2.0;
  QRectF rect( pnt1, pnt2 );
  return  rect.normalized().adjusted(-extra, -extra, extra, extra);
}

}
}
