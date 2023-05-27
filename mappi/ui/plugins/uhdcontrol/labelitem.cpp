#include "labelitem.h"

#include <qpainter.h>
#include <qfontmetrics.h>

#include <cross-commons/debug/tlog.h>

#include <meteo/commons/global/common.h>
#include <meteo/commons/ui/map/layeritems.h>
#include <meteo/commons/ui/map/ramka.h>
#include <meteo/commons/ui/map/view/mapscene.h>

namespace meteo {
namespace spectr {

LabelItem::LabelItem(map::LayerItems* layer, QGraphicsItem* parent, QGraphicsScene* scene)
  : QGraphicsItem(parent)
{
  Q_UNUSED(scene);
  layer_ = layer;
  align_ = Qt::AlignRight | Qt::AlignVCenter;

  padding_ = QMargins(2,1,2,1);
  offset_ = QPoint(0,0);

  colorText_ = Qt::black;
  colorBg_ = QColor(255,255,255,180);
  penBorder_ = QPen(Qt::blue, 1);

  setFlag(QGraphicsItem::ItemSendsScenePositionChanges);
}

LabelItem::~LabelItem()
{
  if ( 0 != layer_ ) {
    layer_->removeItem(this);
  }
}

GeoPoint LabelItem::geoPos() const
{
  return geoPos_;
}

void LabelItem::setGeoPos(const GeoPoint& gp)
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

void LabelItem::setText(const QString& text)
{
  prepareGeometryChange();

  text_ = text;

  QFont f;
  QFontMetrics fm(f);
  boundingRect_ = fm.boundingRect(0,0,0,0, Qt::TextDontClip, text_);

  boundingRect_.setLeft(boundingRect_.left() - padding_.left());
  boundingRect_.setTop(boundingRect_.top() - padding_.top());
  boundingRect_.setRight(boundingRect_.right() + padding_.right());
  boundingRect_.setBottom(boundingRect_.bottom() + padding_.bottom());
}

void LabelItem::setPadding(int left, int top, int right, int bottom)
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

void LabelItem::setOffset(int offsetX, int offsetY)
{
  prepareGeometryChange();

  if ( -1 != offsetX ) { offset_.setX(offsetX); }
  if ( -1 != offsetY ) { offset_.setY(offsetY); }
}

QRectF LabelItem::boundingRect() const
{
  QRectF r = textRect();

  r.setLeft(r.left() - padding_.left());
  r.setTop(r.top() - padding_.top());
  r.setRight(r.right() + padding_.right());
  r.setBottom(r.bottom() + padding_.bottom());

  return r;
}

void LabelItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
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

  if ( doc->isStub() ) {
    painter->save();
    painter->translate(calcScenePoint());
  }

  QRectF r = boundingRect();
  painter->setPen(penBorder_);
  painter->setBrush(colorBg_);
  painter->drawRect(r);

  painter->setPen(colorText_);
  painter->setBrush(Qt::NoBrush);
  painter->drawText(r, Qt::AlignCenter, text_);

  if ( doc->isStub() ) {
    painter->restore();
  }

  painter->setRenderHint(QPainter::Antialiasing, antialiasing);
}

QPoint LabelItem::calcScenePoint() const
{
  map::Document* doc = document();
  if ( 0 != doc ) {
    return doc->coord2screen(geoPos_);
  }
  return QPoint();
}

QRectF LabelItem::textRect() const
{
  QRectF r = boundingRect_;

  QPointF p(0,0);

  map::Document* doc = document();
  if ( 0 != doc && !doc->isStub() ) {
    p = calcScenePoint() - scenePos();
  }

  if ( align_.testFlag(Qt::AlignLeft) ) {
    r.moveRight(p.x() - offset_.x());
  }
  if ( align_.testFlag(Qt::AlignRight) ) {
    r.moveLeft(p.x() + offset_.x());
  }
  if ( align_.testFlag(Qt::AlignHCenter) ) {
    r.moveLeft(p.x() - r.width()/2 + offset_.x());
  }

  if ( align_.testFlag(Qt::AlignTop) ) {
    r.moveBottom(p.y() - offset_.y());
  }
  if ( align_.testFlag(Qt::AlignBottom) ) {
    r.moveTop(p.y() + offset_.y());
  }
  if ( align_.testFlag(Qt::AlignVCenter) ) {
    r.moveTop(p.y() - r.height()/2 - offset_.y());
  }

  return r;
}

QVariant LabelItem::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value)
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

map::Document* LabelItem::document() const
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

} // spectr
} // meteo
