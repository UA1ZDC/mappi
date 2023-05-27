#include "pixmapdrawobject.h"

#include <qpainter.h>
#include <qcursor.h>

#include <cross-commons/debug/tlog.h>
#include <meteo/commons/ui/map/object.h>
#include <meteo/commons/ui/map/view/drawtools/geoobjectitem.h>
#include <meteo/commons/ui/map/view/mapscene.h>
#include <meteo/commons/ui/graphitems/markeritem.h>

namespace meteo {
namespace map {

PixmapDrawObject::PixmapDrawObject(MapScene* mapScene, const QString& id)
  : DrawObject(mapScene, id),
    geoPixmap_(0),
    nodeItem_(0),
    objItem_(0)
{
  if ( 0 == scene() ) {
    debug_log << QObject::tr("Невозможно инициализировать объект, передан нулевой указатель на объект сцены");
    return;
  }
}

PixmapDrawObject::~PixmapDrawObject()
{
  delete geoPixmap_;
  geoPixmap_ = 0;
}

void PixmapDrawObject::appendNode(const QPoint& point, int beforeIdx)
{
  if ( 0 == scene() ) { return; }

  GeoPoint gp = scene()->screen2coord(point);
  appendNode(gp, beforeIdx);
}

void PixmapDrawObject::appendNode(const GeoPoint& point, int beforeIdx)
{
  Q_UNUSED( beforeIdx );

  if ( nullptr == geoPixmap_) {
    return;
  }
  if ( nullptr == nodeItem_ ) {
    nodeItem_ = createNode();
  }
  nodeItem_->setGeoPos(point);

  calcSceneCoords();
}

void PixmapDrawObject::calcSceneCoords()
{
  if ( 0 == nodeItem_ ) { return; }
  if ( 0 == objItem_ ) { return; }

  nodeItem_->setPos(nodeItem_->calcScenePoint());

  GeoVector v;
  v.append(nodeItem_->geoPos());
  geoPixmap_->setSkelet(v);

  objItem_->setObject(geoPixmap_);

  DrawObject::calcSceneCoords();
}

void PixmapDrawObject::update()
{
}

void PixmapDrawObject::markerChanged(MarkerItem* marker)
{
  if ( 0 == marker ) {
    debug_log << QObject::tr("Получено уведомление от несуществующего маркера");
    return;
  }

  switch ( marker->state() ) {
    case MarkerItem::kNormal:  /*debug_log << "normal";*/ break;
    case MarkerItem::kHover:   /*debug_log << "hover";*/ break;
    case MarkerItem::kPress:   /*debug_log << "press";*/ break;
    case MarkerItem::kRelease: /*debug_log << "release";*/ break;
    case MarkerItem::kClick:   /*debug_log << "click";*/ break;
    case MarkerItem::kDrag:
    case MarkerItem::kDrop:
    case MarkerItem::kGroupDrag: {
      calcSceneCoords();
    } break;
  }
}

void PixmapDrawObject::setObject(const Object* obj)
{
  if ( 0 == scene() ) { return; }

  if ( 0 == obj ) {
    geoPixmap_ = 0;
    return;
  }

  if ( obj->type() != kPixmap ) {
    debug_log << QObject::tr("Ожидается объект типа GeoPolygon");
    geoPixmap_ = 0;
    return;
  }

  if ( 0 != geoPixmap_ ) {
    delete geoPixmap_;
    geoPixmap_ = 0;
  }

  Object* tmp = obj->copy(scene()->document()->projection());
  geoPixmap_ = mapobject_cast<GeoPixmap*>(tmp);

  if ( 0 == geoPixmap_ ) {
    delete tmp;
  }

  if ( 0 != geoPixmap_ ) {
    GeoVector v = geoPixmap_->skelet();
    setObjectSkelet(v);
  }

  if ( 0 != geoPixmap_ ) {
    delete objItem_;
    objItem_ = new GeoObjectItem(geoPixmap_, scene());
    objItem_->setObject(geoPixmap_);
    objItem_->setZValue(objItem_->zValue() - 100);
    objItem_->setFlag(QGraphicsItem::ItemIsMovable);
    objItem_->setHighlight(isHighlighted());
  }
}

void PixmapDrawObject::setObjectSkelet(const QPolygon& skelet)
{
  if ( 0 == scene() ) { return; }

  GeoVector v = scene()->document()->screen2coord(skelet);
  setObjectSkelet(v);
}

void PixmapDrawObject::setObjectSkelet(const GeoVector& skelet)
{
  if ( 0 == scene() ) { return; }
  if ( 0 == geoPixmap_ ) { return; }

  delete nodeItem_;
  nodeItem_ = 0;

  geoPixmap_->setSkelet(GeoVector());

  for ( int i=0,isz=skelet.size(); i<isz; ++i ) {
    appendNode(skelet.at(i));
  }
}

void PixmapDrawObject::setObjectPixmap(const QImage& pix )
{
  if ( 0 == geoPixmap_ ) { return; }

  geoPixmap_->setImage(pix);

  if ( 0 != objItem_ ) {
    objItem_->setPixmap(pix);
    objItem_->setObject(geoPixmap_);
  }
}

GeoVector PixmapDrawObject::objectSkelet() const
{
  if ( 0 == geoPixmap_) { return GeoVector(); }

  return geoPixmap_->skelet();
}

meteo::Property PixmapDrawObject::objectProperty() const
{
  if ( 0 == geoPixmap_) { return meteo::Property(); }

  return geoPixmap_->property();
}

QImage PixmapDrawObject::objectPixmap() const
{
  if ( 0 == geoPixmap_ ) { return QImage(); }

  return geoPixmap_->image();
}

void PixmapDrawObject::mouseMoveEvent(QMouseEvent* e)
{
  static QPointF lastPos;

  if ( 0 == objItem_ ) { return; }
  if ( 0 == nodeItem_ ) { return; }

  if ( e->buttons() == Qt::NoButton && objItem_->isUnderMouse() ) {
    objItem_->setCursor(Qt::SizeAllCursor);
    lastPos = objItem_->scenePos();
  }

  if ( e->buttons() == Qt::NoButton && !objItem_->isUnderMouse() ) {
    objItem_->unsetCursor();
  }

  if ( e->buttons() == Qt::LeftButton && objItem_->isUnderMouse() ) {
    QPointF delta = objItem_->pos() - lastPos;
    nodeItem_->setPos(nodeItem_->calcScenePoint() + delta.toPoint());
    lastPos = objItem_->pos();
  }
}

void PixmapDrawObject::mouseReleaseEvent(QMouseEvent* e)
{
  Q_UNUSED( e );

  if ( 0 == objItem_ ) { return; }

  if ( objItem_->isUnderMouse() ) {
    // обнуляем позицию, после завершения перетаскивания
    objItem_->setPos(0,0);
  }
}

void PixmapDrawObject::removeFromScene()
{
  delete nodeItem_;
  nodeItem_ = 0;

  delete objItem_;
  objItem_ = 0;
}

void PixmapDrawObject::setObjectProperty(const meteo::Property& prop)
{
  if ( 0 == geoPixmap_) { return; }

  geoPixmap_->setProperty(prop);

  if ( 0 != objItem_ ) {
    objItem_->setProperty(prop);
  }
}

MarkerItem* PixmapDrawObject::createNode()
{
  MarkerItem* item = new MarkerItem(4, 0);
  item->setCallback(new DrawObjectClosure(this));
  scene()->addItem(item);
  item->setMovable(true);

  QBrush brush(QColor(255, 0, 0));
  QPen pen(brush, 2);
  item->setStyle(MarkerItem::kNormalStyleRole, pen, brush);
  item->setStyle(MarkerItem::kHoverStyleRole, QPen(QBrush(Qt::black), 2), brush);
  item->setStyle(MarkerItem::kSelectedStyleRole, pen, brush);

  return item;
}

} // map
} // meteo
