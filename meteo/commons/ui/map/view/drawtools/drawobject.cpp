#include "drawobject.h"

#include <qline.h>

#include <commons/mathtools/mnmath.h>
#include <meteo/commons/ui/map/object.h>

namespace meteo {
namespace map {


DrawObject::DrawObject(MapScene* scene, const QString& id)
  : scene_(scene),
    id_(id),
    highlighted_(false)
{
}

DrawObject::~DrawObject()
{
}

void DrawObject::setHighlightEnable(bool enable)
{
  if ( highlighted_ == enable ) { return; }

  highlighted_ = enable;
}

void DrawObject::appendNode(const QPoint& point, int beforeIdx)
{
  Q_UNUSED( point );
  Q_UNUSED( beforeIdx );
}

void DrawObject::appendNode(const GeoPoint& point, int beforeIdx)
{
  Q_UNUSED( point );
  Q_UNUSED( beforeIdx );
}

void DrawObject::insertNode(const QPoint& point, const QPoint& after)
{
  Q_UNUSED( point );
  Q_UNUSED( after );
}

void DrawObject::insertNode(const GeoPoint& point, const GeoPoint& after)
{
  Q_UNUSED( point );
  Q_UNUSED( after );
}

void DrawObject::removeNode(const QPoint& point)
{
  Q_UNUSED( point );
}

void DrawObject::removeNode(const GeoPoint& point)
{
  Q_UNUSED( point );
}

void DrawObject::markerChanged(MarkerItem* marker)
{
  Q_UNUSED( marker );
}

void DrawObject::calcSceneCoords()
{}

void DrawObject::setObject(const Object* object)
{
  Q_UNUSED( object );
}

void DrawObject::setObjectSkelet(const QPolygon& skelet)
{
  Q_UNUSED( skelet );
}

void DrawObject::setObjectSkelet(const GeoVector& skelet)
{
  Q_UNUSED( skelet );
}

void DrawObject::setObjectProperty(const meteo::Property& prop)
{
  Q_UNUSED( prop );
}

void DrawObject::setObjectValue(double value, const QString& format, const QString& unit)
{
  Q_UNUSED( value );
  Q_UNUSED( format );
  Q_UNUSED( unit );
}

void DrawObject::setObjectText(const QString& text)
{
  Q_UNUSED( text );
}

void DrawObject::setObjectPixmap(const QImage& pix)
{
  Q_UNUSED( pix );
}

double DrawObject::objectValue(bool* hasValue) const
{
  Q_UNUSED( hasValue );

  return 0;
}

void DrawObject::setMovable(bool enable)
{
  Q_UNUSED( enable );
}

QLine DrawObject::skeletSectionAt(const QPoint& scenePoint, int width) const
{
  Q_UNUSED( scenePoint );
  Q_UNUSED( width );

  return QLine();
}

QList<QAction*> DrawObject::setupContextMenu(Menu* menu, const QPoint& screenPos)
{
  Q_UNUSED( menu );
  Q_UNUSED( screenPos );

  return QList<QAction*>();
}

void DrawObject::processContextMenu(QAction* act)
{
  Q_UNUSED( act );
}

void DrawObject::mouseMoveEvent(QMouseEvent* e)
{
  Q_UNUSED( e );
}

void DrawObject::mouseReleaseEvent(QMouseEvent* e)
{
  Q_UNUSED( e );
}

QPolygon DrawObject::lineGeometry(const QLine& source, double width) const
{
  int x1 = source.x1();
  int y1 = source.y1();
  int x2 = source.x2();
  int y2 = source.y2();

  double angle = atan2(double(y2-y1),double(x2-x1)) + M_PI_2;
  int sx = MnMath::ftoi_norm(width*sin(angle));
  int cx = MnMath::ftoi_norm(width*cos(angle));

  QPolygon poly;
  poly.append(QPoint(x1+cx,y1+sx));
  poly.append(QPoint(x2+cx,y2+sx));
  poly.append(QPoint(x2-cx,y2-sx));
  poly.append(QPoint(x1-cx,y1-sx));
  return poly;
}

void DrawObjectClosure::run(MarkerItem* item)
{
  if ( 0 == object_ ) { return; }

  object_->markerChanged(item);
}

} // map
} // meteo
