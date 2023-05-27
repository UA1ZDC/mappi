#include "faxgeoobject.h"

#include <qpainter.h>

#include <cross-commons/debug/tlog.h>

#include <meteo/commons/ui/map/view/mapscene.h>
#include <meteo/commons/ui/map/geopolygon.h>
#include <meteo/commons/ui/map/geotext.h>
#include <meteo/commons/ui/map/geopixmap.h>
#include <meteo/commons/ui/map/geogroup.h>


namespace meteo {
namespace map {

FaxGeoObject::FaxGeoObject(QGraphicsScene* scene, QGraphicsItem* parent)
  : QGraphicsItem(parent),
    object_(0)
{
  Q_UNUSED(scene);
}

FaxGeoObject::FaxGeoObject(Object* object, QGraphicsScene* scene, QGraphicsItem* parent)
  : QGraphicsItem(parent),
    object_(0)
{
  Q_UNUSED(scene);
  setObject(object);
}

FaxGeoObject::~FaxGeoObject()
{
}

QRectF FaxGeoObject::boundingRect() const
{
  return boundingRect_;
}

void FaxGeoObject::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
  if ( 0 == object_ ) { return; }

  MapScene* s = qobject_cast<MapScene*>(scene());
  if ( 0 == s ) { return; }

  QRect r = boundingRect().toRect();

  object_->render(painter, r, s->document()->transform());
}

void FaxGeoObject::setObject(Object* obj)
{
  object_ = obj;
  calcBoundingRect();
}

void FaxGeoObject::setSkelet(const GeoVector& skelet)
{
  if ( 0 == object_ ) { return; }

  object_->setSkelet(skelet);

  calcBoundingRect();
}

void FaxGeoObject::setProperty(const meteo::Property& prop)
{
  if ( 0 == object_ ) { return; }

  object_->setProperty(prop);

  calcBoundingRect();
}

void FaxGeoObject::setValue(double value, const QString& format, const QString& unit)
{
  if ( 0 == object_ ) { return; }

  object_->setValue(value, format, unit);

  calcBoundingRect();
}

void FaxGeoObject::removeValue()
{
  if ( 0 == object_ ) { return; }

  // TODO: реализовать удаление значения

  calcBoundingRect();
}

void FaxGeoObject::setText(const QString& text)
{
  GeoText* tmp = mapobject_cast<GeoText*>(object_);
  if ( 0 != tmp ) {
    tmp->setText(text);
  }

  calcBoundingRect();
}

void FaxGeoObject::setPixmap(const QPixmap& pix )
{
  GeoPixmap* tmp = mapobject_cast<GeoPixmap*>(object_);
  if ( 0 != tmp ) {
    tmp->setImage(pix.toImage());
  }

  calcBoundingRect();
}

void FaxGeoObject::setCoord(int a, const GeoPoint& coor)
{
  if( object_ == 0){
    return;
  }

  if( a > 3 || a < 1 ){
    return;
  }

  GeoPixmap* gp = mapobject_cast <GeoPixmap*> (object_);
  if( gp == 0){
    return;
  }
  QPair< QPolygon, GeoVector > coords = gp->coordBunch();

  coords.second[a-1] = coor;
  gp->bindScreenToCoord(coords.first, coords.second);

  calcBoundingRect();
}

void FaxGeoObject::setPixmapPoints(int num, QPoint pnt)
{
  if( object_ == 0){
    return;
  }
  if( num > 3 || num < 1 ){
    return;
  }
  GeoPixmap* gpix = mapobject_cast <GeoPixmap*> (object_);
  if( gpix == 0){
    return;
  }
  QPair< QPolygon, GeoVector > coords = gpix->coordBunch();
  coords.first[num-1] = pnt;
  gpix->bindScreenToCoord(coords.first, coords.second);
  calcBoundingRect();
}

void FaxGeoObject::calcBoundingRect()
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
