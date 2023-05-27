#include "groupdrawobject.h"
#include "geoobjectitem.h"

#include <qevent.h>

#include <cross-commons/debug/tlog.h>
#include <meteo/commons/ui/map/geogroup.h>
#include <meteo/commons/ui/map/view/mapview.h>


namespace meteo {
namespace map {

GroupDrawObject::GroupDrawObject(MapScene* mapScene, const QString& id)
  : DrawObject(mapScene, id),
    object_(0),
    objItem_(0),
    groupItem_(0)
{
  if ( 0 == scene() ) {
    debug_log << QObject::tr("Невозможно инициализировать объект, передан нулевой указатель на объект сцены");
    return;
  }
}

GroupDrawObject::~GroupDrawObject()
{
  if ( 0 != groupItem_ ) {
    scene()->destroyItemGroup(groupItem_);
  }
  groupItem_ = 0;

  deleteSkeletItems();

  delete object_;
  object_ = 0;

  delete objItem_;
  objItem_ = 0;
}

void GroupDrawObject::mouseMoveEvent(QMouseEvent* e)
{
  if ( 0 == scene() ) { return; }
  if ( 0 == object_ ) { return; }

  QPoint scenePos = scene()->mapview()->mapToScene(e->pos()).toPoint();

  for ( int i=0,isz=skeletList_.size(); i<isz; ++i ) {
    if ( skeletList_.at(i)->boundingRect().contains(scenePos) ) {
      if ( !skeletList_.at(i)->sectionAt(scenePos, 5).isNull() ) {
        skeletList_[i]->setCursor(Qt::SizeAllCursor);
      }
    }
  }

  if ( isMovable() ) {
    moveSkelets(object_, uuidList_, skeletList_);
    objItem_->setObject(object_);
  }
}

void GroupDrawObject::addObjectSkelet(Object* obj)
{
  if ( 0 == obj ) { return; }
  if ( 0 == scene() ) { return; }

  QPolygon skelet = scene()->document()->coord2screen(obj->skelet());
  skeletList_.append(new SkeletItem(skelet, 0, scene()));
  uuidList_.append(obj->uuid());

  const std::unordered_set<Object*>& tmp = obj->childs();
  foreach(Object* o, tmp){
    addObjectSkelet(o);
  }
}

void GroupDrawObject::updateObjectSceneSkelet(Object* obj, const QStringList& uuidList, const QList<SkeletItem*> skeletList)
{
  if ( 0 == obj ) { return; }
  if ( 0 == scene() ) { return; }

  if ( uuidList.contains(obj->uuid()) ) {
    int i = uuidList.indexOf(obj->uuid());
    QPolygon skelet = scene()->document()->coord2screen(obj->skelet());
    skeletList.at(i)->setSkelet(skelet);
  }

  const std::unordered_set<Object*>& childs = obj->childs();
  foreach(Object* o, childs){
    updateObjectSceneSkelet(o, uuidList, skeletList);
  }
}

void GroupDrawObject::moveSkelets(Object* obj, const QStringList& uuidList, const QList<SkeletItem*> skeletList)
{
  if ( 0 == obj ) { return; }
  if ( 0 == scene() ) { return; }

  if ( uuidList.contains(obj->uuid()) ) {

    int i = uuidList.indexOf(obj->uuid());
    QPoint itemPos = skeletList_[i]->scenePos().toPoint();
    GeoVector skelet = scene()->document()->screen2coord(skeletList_[i]->skelet().translated(itemPos));
    obj->setSkelet(skelet);
  }

  const std::unordered_set<Object*>& childs = obj->childs();
  foreach(Object* o, childs){
    moveSkelets(o, uuidList, skeletList);
  }
}

void GroupDrawObject::deleteSkeletItems()
{
  for ( int i=0,isz=skeletList_.size(); i<isz; ++i ) {
    delete skeletList_[i];
  }
}

void GroupDrawObject::printDebug(Object* obj, int indent)
{
  if ( 0 == obj ) { return; }

  none_log << QString(" ").repeated(indent) << obj->uuid();

  const std::unordered_set<Object*>& childs = obj->childs();
  foreach(Object* o, childs){
    printDebug(o, indent + 3);
  }
}

void GroupDrawObject::calcSceneCoords()
{
  if ( 0 == object_ ) { return; }
  if ( 0 == objItem_ ) { return; }

  updateObjectSceneSkelet(object_, uuidList_, skeletList_);

  objItem_->setObject(object_);

  DrawObject::calcSceneCoords();
}

QLine GroupDrawObject::skeletSectionAt(const QPoint& scenePos, int width) const
{
  for ( int i=0,isz=skeletList_.size(); i<isz; ++i ) {
    if ( skeletList_.at(i)->boundingRect().contains(scenePos) ) {
      QLine tmp = skeletList_.at(i)->sectionAt(scenePos, width);
      if ( !tmp.isNull() ) {
        return tmp;
      }
    }
  }

  return QLine();
}

void GroupDrawObject::setMovable(bool enable)
{
  if ( 0 == scene() ) { return; }

  if ( !enable && 0 == groupItem_ ) {
    return;
  }
  if ( enable && 0 != groupItem_ ) {
    return;
  }

  if ( enable ) {
    groupItem_ = new QGraphicsItemGroup;
    scene()->addItem(groupItem_);
    groupItem_->setFlag(QGraphicsItem::ItemIsMovable);
    for ( int i=0,isz=skeletList_.size(); i<isz; ++i ) {
      groupItem_->addToGroup(skeletList_.at(i));
    }
    groupItem_->setCursor(Qt::SizeAllCursor);
  }
  else {
    scene()->destroyItemGroup(groupItem_);
    groupItem_ = 0;

    for ( int i=0,isz=skeletList_.size(); i<isz; ++i ) {
      skeletList_[i]->updateSkelet();
      skeletList_[i]->setPos(0,0);
    }
  }
}

bool GroupDrawObject::isMovable() const
{
  return 0 != groupItem_;
}

void GroupDrawObject::setObject(const Object* obj)
{
  if ( 0 == scene() ) { return; }

  if ( 0 == obj ) {
    delete object_;
    object_ = 0;
    return;
  }

  if ( obj->type() != kGroup ) {
    debug_log << QObject::tr("Ожидается объект типа GeoGroup");
    delete object_;
    object_ = 0;
    return;
  }

  if ( 0 != object_ ) {
    delete object_;
    object_ = 0;
  }

  Object* tmp = obj->copy(scene()->document()->projection());
  object_ = mapobject_cast<GeoGroup*>(tmp);

  if ( 0 == object_ ) {
    delete tmp;
  }

  if ( 0 != object_ ) {
    uuidList_.clear();
    deleteSkeletItems();

    addObjectSkelet(object_);

    delete objItem_;
    objItem_ = new GeoObjectItem(0, scene());
    objItem_->setZValue(objItem_->zValue() - 100);
    objItem_->setObject(object_);
  }
}

void GroupDrawObject::setObjectValue(double value, const QString& format, const QString& unit)
{
  if ( 0 == object_ ) { return; }

  object_->setValue(value, format, unit);
}

void GroupDrawObject::removeObjectValue()
{
  if ( 0 == object_ ) { return; }

  object_->removeValue();
}



//
// SkeletItem
//

SkeletItem::SkeletItem(const QPolygon& skelet, QGraphicsItem* parent, MapScene* sc)
  : QGraphicsItem(parent),
    scene_(sc)
{
  if ( 0 == scene() ) {
    sc->addItem(this);
  }
  pen_.setWidth(2);
  pen_.setColor(Qt::red);

  setFlag(QGraphicsItem::ItemSendsScenePositionChanges);
  setAcceptHoverEvents(true);

  setSkelet(skelet);
}

QRectF SkeletItem::boundingRect() const
{
  return boundingRect_;
}

void SkeletItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
  Q_UNUSED( option );
  Q_UNUSED( widget );

  painter->setPen(pen_);
  for ( int i=0,isz=sections_.size(); i<isz; ++i ) {
    painter->drawLine(sections_[i]);
  }
}

void SkeletItem::setSkelet(const QPolygon& skelet)
{
  prepareGeometryChange();

  skelet_ = skelet;

  sections_.clear();
  for ( int i=1,isz=skelet_.size(); i<isz; ++i ) {
    sections_.append(QLine(skelet_[i-1], skelet_[i]));
  }

  boundingRect_ = QRectF();
  for ( int i=0,isz=sections_.size(); i<isz; ++i ) {
    boundingRect_ |= QRect(sections_[i].p1(), sections_[i].p2());
  }

  if ( skelet_.size() == 1 ) {
    QPoint topLeft = skelet_.first() - QPoint(3,3);
    boundingRect_ = QRectF(topLeft, QSize(6,6));
  }
}

QLine SkeletItem::sectionAt(const QPoint& scenePos, int width) const
{
  for ( int i=0,isz=sections_.size(); i<isz; ++i ) {
    QPolygon tmp = lineGeometry(sections_[i], width);
    if ( tmp.containsPoint(scenePos, Qt::WindingFill) ) {
      return sections_[i];
    }
  }

  return QLine();
}

void SkeletItem::updateSkelet()
{
  skelet_.translate(scenePos().toPoint());
  setSkelet(skelet_);
}

QVariant SkeletItem::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value)
{
  if ( QGraphicsItem::ItemScenePositionHasChanged == change ) {
//    updateSkelet();
  }

  return QGraphicsItem::itemChange(change, value);
}

QPolygon SkeletItem::lineGeometry(const QLine& source, double width) const
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

} // map
} // meteo
