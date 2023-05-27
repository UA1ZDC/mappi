#include "geoobjectitem.h"

#include <qpainter.h>

#include <cross-commons/debug/tlog.h>
#include <meteo/commons/ui/map/view/mapscene.h>
#include <meteo/commons/ui/map/geopolygon.h>
#include <meteo/commons/ui/map/geotext.h>
#include <meteo/commons/ui/map/geopixmap.h>
#include <meteo/commons/ui/map/geogroup.h>


namespace meteo {
namespace map {

GeoObjectItem::GeoObjectItem(QGraphicsScene* sc, QGraphicsItem* parent)
  : QGraphicsItem(parent),
    object_(0),
    highlighted_(false)
{
  if ( 0 == scene() ) {
    sc->addItem(this);
  }
}

GeoObjectItem::GeoObjectItem(Object* object, QGraphicsScene* sc, QGraphicsItem* parent)
  : QGraphicsItem(parent),
    object_(0),
    highlighted_(false)
{
  if ( 0 == scene() ) {
    sc->addItem(this);
  }
  setObject(object);
}

GeoObjectItem::~GeoObjectItem()
{
}

QRectF GeoObjectItem::boundingRect() const
{
  return boundingRect_;
}

void GeoObjectItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
  if ( 0 == object_ ) { return; }

  MapScene* s = qobject_cast<MapScene*>(scene());
  if ( 0 == s ) { return; }

  QRect r = boundingRect().toRect();

  object_->render(painter, r, s->document()->transform());

  if ( highlighted_ ) {
    painter->setPen(QPen(QBrush(Qt::red), 1));
    painter->setBrush(QBrush(Qt::NoBrush));
    painter->drawRect(r);
  }
}

void GeoObjectItem::setObject(Object* obj)
{
  object_ = obj;
  calcBoundingRect();
}

void GeoObjectItem::setSkelet(const GeoVector& skelet)
{
  if ( 0 == object_ ) { return; }

  object_->setSkelet(skelet);

  calcBoundingRect();
}

void GeoObjectItem::setProperty(const meteo::Property& prop)
{
  if ( 0 == object_ ) { return; }

  object_->setProperty(prop);

  calcBoundingRect();
}

void GeoObjectItem::setValue(double value, const QString& format, const QString& unit)
{
  if ( 0 == object_ ) { return; }

  object_->setValue(value, format, unit);

  calcBoundingRect();
}

void GeoObjectItem::removeValue()
{
  if ( 0 == object_ ) { return; }

  // TODO: реализовать удаление значения

  calcBoundingRect();
}

void GeoObjectItem::setText(const QString& text)
{
  GeoText* tmp = mapobject_cast<GeoText*>(object_);
  if ( 0 != tmp ) {
    tmp->setText(text);
  }

  calcBoundingRect();
}

void GeoObjectItem::setPixmap(const QImage& pix )
{
  GeoPixmap* tmp = mapobject_cast<GeoPixmap*>(object_);
  if ( 0 != tmp ) {
    tmp->setImage(pix);
  }

  calcBoundingRect();
}

void GeoObjectItem::calcBoundingRect()
{
  prepareGeometryChange();

  boundingRect_ = QRect();

  if ( 0 == object_ ) { return; }

  MapScene* s = qobject_cast<MapScene*>(scene());
  if ( 0 == s ) {
    debug_log << QObject::tr("Невозможно расчитать bounding rect, объект не добавлен на сцену.");
    return;
  }

  QList<QRect> tmp = object_->boundingRect(s->document()->transform());
  for ( int i=0,isz=tmp.size(); i<isz; ++i ) {
    boundingRect_ |= tmp.at(i);
  }
}

} // map
} // meteo
