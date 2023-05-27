#include "combolabelitem.h"

#include <qpixmap.h>
#include <qpainter.h>
#include <qfontmetrics.h>

#include <cross-commons/debug/tlog.h>

#include <meteo/commons/global/common.h>
#include <meteo/commons/ui/map/layeritems.h>
#include <meteo/commons/ui/map/ramka.h>
#include <meteo/commons/ui/map/view/mapscene.h>

namespace meteo {
namespace spectr {

ComboLabelItem::ComboLabelItem(map::LayerItems* layer, QGraphicsItem* parent, QGraphicsScene* scene)
  : QGraphicsItem(parent)
{
  Q_UNUSED(scene);
  layer_ = layer;
  align_ = Qt::AlignTop | Qt::AlignHCenter;

  padding_ = QMargins(5,1,2,0);
  offset_ = QPoint(0,10);

  colorText_ = Qt::black;
  colorBg_ = QColor(255,255,255,180);
  penBorder_ = QPen(Qt::blue, 1);

  setFlag(QGraphicsItem::ItemSendsScenePositionChanges);
}

ComboLabelItem::~ComboLabelItem()
{
  if ( 0 != layer_ ) {
    layer_->removeItem(this);
  }
}

GeoPoint ComboLabelItem::geoPos() const
{
  return geoPos_;
}

void ComboLabelItem::setGeoPos(const GeoPoint& gp)
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

void ComboLabelItem::setTitle(const QString& text)
{
  if ( title_ == text ) { return; }

  prepareGeometryChange();

  title_ = text;

  calcBoundingRect();
}


void ComboLabelItem::addText(const QString& label, const QString& value)
{
  prepareGeometryChange();

  labels_ << label;
  values_ << value;

  calcBoundingRect();
}

void ComboLabelItem::clearTexts()
{
  prepareGeometryChange();

  labels_.clear();
  values_.clear();

  calcBoundingRect();
}

void ComboLabelItem::setPadding(int left, int top, int right, int bottom)
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

void ComboLabelItem::setOffset(int offsetX, int offsetY)
{
  prepareGeometryChange();

  if ( -1 != offsetX ) { offset_.setX(offsetX); }
  if ( -1 != offsetY ) { offset_.setY(offsetY); }
}

QRectF ComboLabelItem::boundingRect() const
{
  QRectF rect = boundingRect_;

  double l = rect.left();
  double t = rect.top();
  double r = rect.right();
  double b = rect.bottom();

  l -= penBorder_.width();
  t -= penBorder_.width();
  r += penBorder_.width();
  b += penBorder_.width();

  l -= padding_.left();
  t -= padding_.top();
  r += padding_.right();
  b += padding_.bottom();

  rect.setLeft(l);
  rect.setTop(t);
  rect.setRight(r);
  rect.setBottom(b);

  QPointF p(0,0);

  map::Document* doc = document();
  if ( 0 != doc && !doc->isStub() ) {
    p = calcScenePoint() - scenePos();
  }

  if ( align_.testFlag(Qt::AlignLeft) ) {
    rect.moveRight(p.x() - offset_.x());
  }
  if ( align_.testFlag(Qt::AlignRight) ) {
    rect.moveLeft(p.x() + offset_.x());
  }
  if ( align_.testFlag(Qt::AlignHCenter) ) {
    rect.moveLeft(p.x() - rect.width()/2 + offset_.x());
  }

  if ( align_.testFlag(Qt::AlignTop) ) {
    rect.moveBottom(p.y() - offset_.y());
  }
  if ( align_.testFlag(Qt::AlignBottom) ) {
    rect.moveTop(p.y() + offset_.y());
  }
  if ( align_.testFlag(Qt::AlignVCenter) ) {
    rect.moveTop(p.y() - rect.height()/2 - offset_.y());
  }

  return rect;
}

void ComboLabelItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
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
  double b = rect.bottom();

  double off = penBorder_.width() / 2;
  l += off;
  t += off;
  r -= off;
  b -= off;

  painter->setPen(penBorder_);

  painter->drawLine(l,t,r,t);
  painter->drawLine(l,b,r,b);
  painter->drawLine(l,t,l,b);
  painter->drawLine(r,t,r,b);


  l = rect.left() + penBorder_.width();
  t = rect.top() + penBorder_.width();
  r = rect.right() - penBorder_.width();
  b = rect.bottom() - penBorder_.width();

  rect.setLeft(l);
  rect.setTop(t);
  rect.setRight(r);
  rect.setBottom(b);

  painter->fillRect(rect, colorBg_);

  painter->setBrush(colorBg_);

  l = rect.left() + padding_.left();
  t = rect.top() + padding_.top();
  r = rect.right() - padding_.right();
  b = rect.bottom() - padding_.bottom();

  rect.setLeft(l);
  rect.setTop(t);
  rect.setRight(r);
  rect.setBottom(b);

  QRectF titleRect = cacheTitleRect_;
  titleRect.moveTopLeft(rect.topLeft());
  titleRect.setWidth(rect.width());
  painter->setPen(colorText_);
  painter->drawText(titleRect, Qt::AlignCenter, title_);

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

QPoint ComboLabelItem::calcScenePoint() const
{
  map::Document* doc = document();
  if ( 0 != doc ) {
    return doc->coord2screen(geoPos_);
  }
  return QPoint();
}

void ComboLabelItem::setAlignment(Qt::Alignment align)
{
  prepareGeometryChange();

  align_ = align;

  calcBoundingRect();
}

QVariant ComboLabelItem::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value)
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

map::Document* ComboLabelItem::document() const
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

void ComboLabelItem:: calcBoundingRect()
{
  cacheLabelRects_.clear();
  cacheValueRects_.clear();

  QPixmap pix(1,1);
  QPainter painter(&pix);
  painter.setRenderHint(QPainter::TextAntialiasing, true);

  cacheTitleRect_ = painter.boundingRect(QRect(), Qt::AlignLeft|Qt::AlignVCenter|Qt::TextDontClip, title_);

  QRectF left;
  left.moveTopLeft(cacheTitleRect_.bottomLeft());
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

  boundingRect_ = cacheTitleRect_;
  boundingRect_ = boundingRect_.united(left);
  boundingRect_ = boundingRect_.united(right);

  boundingRect_.setLeft(boundingRect_.left() - padding_.left());
  boundingRect_.setTop(boundingRect_.top() - padding_.top());
  boundingRect_.setRight(boundingRect_.right() + padding_.right());
  boundingRect_.setBottom(boundingRect_.bottom() + padding_.bottom());

  // округляем размеры bounding rect до четных величин, чтобы избежать артефактов при отрисовки
  int w = qRound(boundingRect_.width());
  int h = qRound(boundingRect_.height());
  boundingRect_.setWidth(w + w % 2);
  boundingRect_.setHeight(h + h % 2);
}

} // gspectr
} // meteo
