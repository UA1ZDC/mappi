#include <QtGui>

#include "textitem.h"
#include <cross-commons/debug/tlog.h>
#include <meteo/commons/ui/map/view/mapscene.h>
#include <meteo/commons/ui/map/layeritems.h>

namespace meteo {
namespace map {

TextItem::TextItem(const meteo::GeoPoint& gp, const QString& text, QGraphicsItem* parent, MapScene* sc) :
   QGraphicsItem(parent),
  geoPoint_(gp),
  scene_(sc),
  angle_(0),
  text_(text)
{
  if ( 0 == scene() ) {
    scene_->addItem(this);
  }
  scene_->document()->addItem(this);
}

TextItem::~TextItem()
{
  scene_->document()->removeItem(this);
}

void TextItem::setText(const QString text)
{
  text_ = text;

  QFont f;
  QFontMetrics fm(f);
  QSize sz(fm.width(text_)+6, fm.height()+6);
  textRect_ = QRectF(QPointF(0,0), sz);

  prepareGeometryChange();
}

void TextItem::setAngle(double angle)
{
  angle_ = angle;
  prepareGeometryChange();
}

QRectF TextItem::boundingRect() const
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
  tr.rotate(angle_);
  QRectF r = tr.mapRect(textRect_);

  return r;
}

QPainterPath TextItem::shape() const
{
  QPainterPath p;
  p.addRect( boundingRect() );
  return p;
}

void TextItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
  Q_UNUSED(option)
  Q_UNUSED(widget)

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
  tr.rotate(angle_);

  painter->setTransform(tr, true);
  painter->drawText(textRect_, text_);

  painter->restore();
}

}
}
