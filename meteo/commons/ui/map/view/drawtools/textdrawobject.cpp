#include "textdrawobject.h"

#include <qpainter.h>
#include <qcursor.h>

#include <cross-commons/debug/tlog.h>
#include <meteo/commons/ui/graphitems/markeritem.h>
#include <meteo/commons/ui/map/object.h>
#include <meteo/commons/ui/map/view/drawtools/geoobjectitem.h>
#include <meteo/commons/ui/map/view/mapscene.h>

namespace meteo {
namespace map {

TextDrawObject::TextDrawObject(MapScene* mapScene, const QString& id)
  : DrawObject(mapScene, id),
    geoText_(0),
    nodeItem_(0),
    objItem_(0)
{
  if ( 0 == scene() ) {
    debug_log << QObject::tr("Невозможно инициализировать объект, передан нулевой указатель на объект сцены");
    return;
  }
}

TextDrawObject::~TextDrawObject()
{
  delete geoText_;
  geoText_ = 0;
}

void TextDrawObject::appendNode(const QPoint& point, int beforeIdx)
{
  if ( 0 == scene() ) { return; }

  GeoPoint gp = scene()->screen2coord(point);
  appendNode(gp, beforeIdx);
}

void TextDrawObject::appendNode(const GeoPoint& point, int beforeIdx)
{
  Q_UNUSED( beforeIdx );

  if ( 0 == geoText_ ) { return; }
  if ( 0 != nodeItem_ ) { return; }

  nodeItem_ = createNode();
  nodeItem_->setGeoPos(point);

  calcSceneCoords();
}

void TextDrawObject::calcSceneCoords()
{
  if ( 0 == nodeItem_ ) { return; }
  if ( 0 == objItem_ ) { return; }

  nodeItem_->setPos(nodeItem_->calcScenePoint());

  GeoVector v;
  v.append(nodeItem_->geoPos());
  geoText_->setSkelet(v);
  objItem_->setObject(geoText_);
}

void TextDrawObject::update()
{
}

void TextDrawObject::markerChanged(MarkerItem* marker)
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
      // move Text
      calcSceneCoords();
    } break;
  }
}

void TextDrawObject::setObject(const Object* obj)
{
  if ( 0 == scene() ) { return; }

  if ( 0 == obj ) {
    geoText_ = 0;
    return;
  }

  if ( obj->type() != kText ) {
    debug_log << QObject::tr("Ожидается объект типа GeoText");
    geoText_ = 0;
    return;
  }

  if ( 0 != geoText_ ) {
    delete geoText_;
    geoText_ = 0;
  }

  Object* tmp = obj->copy(scene()->document()->projection());
  geoText_ = mapobject_cast<GeoText*>(tmp);

  if ( 0 == geoText_ ) {
    delete tmp;
  }

  if ( 0 != geoText_ ) {
    GeoVector v = geoText_->skelet();
    setObjectSkelet(v);
  }

  if ( 0 != geoText_ ) {
    delete objItem_;
    objItem_ = new GeoObjectItem(geoText_, scene());
    objItem_->setObject(geoText_);
    objItem_->setZValue(objItem_->zValue() - 100);
    objItem_->setFlag(QGraphicsItem::ItemIsMovable);
    objItem_->setHighlight(isHighlighted());
  }
}

void TextDrawObject::setObjectSkelet(const QPolygon& skelet)
{
  if ( 0 == scene() ) { return; }

  GeoVector v = scene()->document()->screen2coord(skelet);
  setObjectSkelet(v);
}

void TextDrawObject::setObjectSkelet(const GeoVector& skelet)
{
  if ( 0 == scene() ) { return; }
  if ( 0 == geoText_ ) { return; }

  delete nodeItem_;
  nodeItem_ = 0;

  geoText_->setSkelet(GeoVector());

  for ( int i=0,isz=skelet.size(); i<isz; ++i ) {
    appendNode(skelet.at(i));
  }
}

void TextDrawObject::setObjectText(const QString& text)
{
  if ( 0 == geoText_ ) { return; }

  geoText_->setText(text);

  if ( 0 != objItem_ ) {
    objItem_->setText(text);
    objItem_->setObject(geoText_);
  }
}

GeoVector TextDrawObject::objectSkelet() const
{
  if ( 0 == geoText_ ) { return GeoVector(); }

  return geoText_->skelet();
}

meteo::Property TextDrawObject::objectProperty() const
{
  if ( 0 == geoText_ ) { return meteo::Property(); }

  return geoText_->property();
}

QString TextDrawObject::objectText() const
{
  if ( 0 == geoText_ ) { return QString(); }

  return geoText_->text();
}

void TextDrawObject::mouseMoveEvent(QMouseEvent* e)
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

void TextDrawObject::mouseReleaseEvent(QMouseEvent* e)
{
  Q_UNUSED( e );

  if ( 0 == objItem_ ) { return; }

  if ( objItem_->isUnderMouse() ) {
    // обнуляем позицию, после завершения перетаскивания
    objItem_->setPos(0,0);
  }
}

void TextDrawObject::removeFromScene()
{
  delete nodeItem_;
  nodeItem_ = 0;

  delete objItem_;
  objItem_ = 0;
}

void TextDrawObject::setObjectProperty(const meteo::Property& prop)
{
  if ( 0 == geoText_ ) { return; }

  geoText_->setProperty(prop);

  if ( 0 != objItem_ ) {
    objItem_->setProperty(prop);
  }
}

MarkerItem* TextDrawObject::createNode()
{
  MarkerItem* item = new MarkerItem(4, 0);
  item->setCallback(new DrawObjectClosure(this));
  scene()->addItem(item);
  item->setMovable(true);

  QBrush brush(QColor(110, 125, 235));
  QPen pen(brush, 2);
  item->setStyle(MarkerItem::kNormalStyleRole, pen, brush);
  item->setStyle(MarkerItem::kHoverStyleRole, QPen(QBrush(Qt::black), 2), brush);
  item->setStyle(MarkerItem::kSelectedStyleRole, pen, brush);

  return item;
}

} // map
} // meteo
