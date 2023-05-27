#include "balloonitem.h"

#include <qpixmap.h>
#include <qpainter.h>
#include <qfontmetrics.h>

#include <cross-commons/debug/tlog.h>

#include <meteo/commons/global/common.h>
#include <meteo/commons/ui/map/layeritems.h>
#include <meteo/commons/ui/map/ramka.h>
#include <meteo/commons/ui/map/view/mapscene.h>

namespace meteo {
namespace map {

BalloonItem::BalloonItem(map::LayerItems* layer, QGraphicsItem* parent, QGraphicsScene* scene)
  : QGraphicsItem(parent)
{
  Q_UNUSED(scene);
  layer_ = layer;

  padding_ = QMargins(5,1,2,0);

  colorText_ = Qt::black;
  colorBg_ = QColor(255,255,255,180);
  penBorder_ = QPen(Qt::blue, 1);

  setFlag(QGraphicsItem::ItemSendsScenePositionChanges);
}

BalloonItem::~BalloonItem()
{
  if ( 0 != layer_ ) {
    layer_->removeItem(this);
  }
}

GeoPoint BalloonItem::geoPos() const
{
  return geoPos_;
}

void BalloonItem::setGeoPos(const GeoPoint& gp)
{
  map::Document* doc = document();
  if ( 0 == doc ) {
    debug_log << QObject::tr("Невозможно обновить позицию, т.к. нулевой указатель на Document");
    return;
  }

  prepareGeometryChange();

  setPos(mapFromScene(doc->coord2screenf(gp)));
  geoPos_ = gp;
}

void BalloonItem::addText(const QString& label, const QString& value)
{
  prepareGeometryChange();

  labels_ << label;
  values_ << value;

  calcBoundingRect();
}

void BalloonItem::clearTexts()
{
  prepareGeometryChange();

  labels_.clear();
  values_.clear();

  calcBoundingRect();
}

void BalloonItem::setPadding(int left, int top, int right, int bottom)
{
  prepareGeometryChange();

  if ( -1 != left ) {
    padding_.setLeft(left);
  }
  if ( -1 != top ) {
    padding_.setTop(top);
  }
  if ( -1 != right ) {
    padding_.setRight(right);
  }
  if ( -1 != bottom ) {
    padding_.setBottom(bottom);
  }
}

void BalloonItem::setOffset(int x, int y)
{
  prepareGeometryChange();

  xCenter_ = qBound(0, x, 100) / 100.0;
  yCenter_ = qBound(0, y, 100) / 100.0;
}

QRectF BalloonItem::boundingRect() const
{
  QRectF rect = boundingRect_;

  double w = rect.width();
  double h = rect.height();

  w += penBorder_.width() * 2;
  h += penBorder_.width() * 2;

  w += padding_.left() + padding_.right();
  h += padding_.top() + padding_.bottom();

  rect.setSize(QSizeF(w,h));

  map::Document* doc = document();
  if ( 0 != doc && !doc->isStub() ) {
    QPointF p = calcScenePoint() - scenePos();
    rect.translate(p);
  }

  rect.translate(-QPointF(rect.width() * xCenter_, rect.height() * yCenter_));

  return rect;
}

void BalloonItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
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
  painter->setRenderHint(QPainter::TextAntialiasing, true);

  if ( doc->isStub() ) {
    painter->save();
    painter->translate(calcScenePoint());
  }

  QRectF rect = boundingRect();

  // debug
//  QRectF brect = rect;
//  brect.setTopLeft(brect.topLeft() - QPoint(1,1));
//  brect.setBottomRight(brect.bottomRight() + QPoint(1,1));
//  painter->save();
//  painter->setBrush(Qt::NoBrush);
//  painter->setPen(Qt::black);
//  painter->fillRect(brect, Qt::black);
//  painter->restore();
  // end debug

  double l = rect.left();
  double t = rect.top();
  double r = rect.right();
  double b = rect.bottom() - 10;

  double off = penBorder_.width() / 2.0;
  l += off;
  t += off;
  r -= off;
  b -= off;

  double lc = (r - l) * xCenter_;
  double rc = (r - l) * (1.0 - xCenter_);

  painter->setPen(penBorder_);

  QPolygonF border({ QPointF(l,t), QPointF(r,t),
                     QPointF(r,b),
                     QPointF(r - rc + 5,b), QPointF(r - rc,b + 10),
                     QPointF(l + lc,b + 10), QPointF(l + lc - 5,b),
                     QPointF(l,b),
                     QPointF(l,t)
                   });
  QPainterPath path;
  path.addPolygon(border);
  painter->fillPath(path, colorBg_);
  painter->drawPath(path);

  l = rect.left() + penBorder_.width();
  t = rect.top() + penBorder_.width();
  r = rect.right() - penBorder_.width();
  b = rect.bottom() - penBorder_.width() - 10;

  rect.setLeft(l);
  rect.setTop(t);
  rect.setRight(r);
  rect.setBottom(b);

  painter->setBrush(colorBg_);

  l = rect.left() + padding_.left();
  t = rect.top() + padding_.top();
  r = rect.right() - padding_.right();
  b = rect.bottom() - padding_.bottom();

  rect.setLeft(l);
  rect.setTop(t);
  rect.setRight(r);
  rect.setBottom(b);

  QRectF titleRect;
  titleRect.moveTopLeft(rect.topLeft());
  titleRect.setWidth(rect.width());
  painter->setPen(colorText_);

  // debug
//  QRectF trect = titleRect;
//  trect.setTopLeft(trect.topLeft() - QPoint(1,1));
//  painter->save();
//  painter->setBrush(Qt::NoBrush);
//  painter->setPen(Qt::white);
//  painter->drawRect(trect);
//  painter->restore();
  // end debug

  double maxWidth = 0;
  double topOffset = 0;
  QList<QRectF> list = cacheLabelRects_;
  for ( int i=0,isz=list.size(); i<isz; ++i ) {
    QRectF r = list[i];

    r.moveTopLeft(titleRect.bottomLeft());
    r.moveTop(r.top() + topOffset);
    topOffset += r.height();
    maxWidth = qMax(maxWidth, r.width());

    painter->drawText(r, Qt::AlignLeft|Qt::AlignVCenter, labels_[i]);

    // debug
//    QRectF lrect = r;
//    lrect.setTopLeft(lrect.topLeft() - QPoint(1,1));
//    painter->save();
//    painter->setBrush(Qt::NoBrush);
//    painter->setPen(Qt::green);
//    painter->drawRect(lrect);
//    painter->restore();
    // end debug
  }

  topOffset = 0;
  list = cacheValueRects_;
  for ( int i=0,isz=list.size(); i<isz; ++i ) {
    QRectF r = list[i];
    r.moveTopLeft(titleRect.bottomLeft());
    r.moveTop(r.top() + topOffset);
    r.moveLeft(r.left() + maxWidth);
    topOffset += r.height();

    painter->drawText(r, Qt::AlignLeft|Qt::AlignVCenter, values_[i]);

    // debug
//    QRectF rrect = r;
//    rrect.setTopLeft(rrect.topLeft() - QPoint(1,1));
//    painter->save();
//    painter->setBrush(Qt::NoBrush);
//    painter->setPen(Qt::yellow);
//    painter->drawRect(rrect);
//    painter->restore();
    // end debug
  }

  if ( doc->isStub() ) {
    painter->restore();
  }

  painter->setRenderHint(QPainter::Antialiasing, antialiasing);
}

QPointF BalloonItem::calcScenePoint() const
{
  map::Document* doc = document();
  if ( 0 != doc ) {
    return doc->coord2screenf(geoPos_);
  }
  return QPointF();
}

QVariant BalloonItem::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value)
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

map::Document* BalloonItem::document() const
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

void BalloonItem:: calcBoundingRect()
{
  cacheLabelRects_.clear();
  cacheValueRects_.clear();

  QPixmap pix(1,1);
  QPainter painter(&pix);
  painter.setRenderHint(QPainter::TextAntialiasing, true);

  QRectF left;
  foreach ( const QString& text, labels_ ) {
    QRectF r = painter.boundingRect(QRect(), Qt::AlignLeft|Qt::AlignVCenter|Qt::TextDontClip, text);
    r.moveTopLeft(left.bottomLeft());
    left = left.united(r);

    cacheLabelRects_ << r;
  }

  QRectF right;
  right.moveTopLeft(left.topRight());
  foreach ( const QString& text, values_ ) {
    QRectF r = painter.boundingRect(QRect(), Qt::AlignLeft|Qt::AlignVCenter|Qt::TextDontClip, text);
    r.moveTopLeft(right.bottomLeft());
    right = right.united(r);

    cacheValueRects_ << r;
  }

  boundingRect_ = left;
  boundingRect_ = boundingRect_.united(right);

  boundingRect_.setLeft(boundingRect_.left() - padding_.left());
  boundingRect_.setTop(boundingRect_.top() - padding_.top());
  boundingRect_.setRight(boundingRect_.right() + padding_.right());
  boundingRect_.setBottom(boundingRect_.bottom() + padding_.bottom() + 10);
}

} // map
} // meteo
