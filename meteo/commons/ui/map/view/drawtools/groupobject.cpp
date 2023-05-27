#include "groupobject.h"

#include <cross-commons/debug/tlog.h>
#include <meteo/commons/ui/map/object.h>
#include <meteo/commons/ui/map/view/mapscene.h>
#include <meteo/commons/ui/map/document.h>
#include <meteo/commons/ui/map/geogroup.h>

#include <qpainter.h>
#include <qstyleoption.h>
#include <qgraphicssceneevent.h>
#include <qmap.h>


namespace meteo {
namespace map {

GroupObject::GroupObject( QGraphicsItem* parent) :
  QGraphicsObject(parent)
{

}

GroupObject::GroupObject(GeoGroup *geoGroup, QGraphicsItem* parent) :
  QGraphicsObject(parent),
  geoGroup_(geoGroup)
{
  objects_.insert(geoGroup_->objects().begin(),geoGroup_->objects().end());
}

GroupObject::~GroupObject()
{
  if (path_ != nullptr) {
    path_->scene()->removeItem(path_);
    delete path_;
    path_ = nullptr;
  }
}

QRectF GroupObject::boundingRect() const
{
  QRectF boundingRect = QRectF();
  if ( nullptr == path_) {
    return boundingRect;
  }
  path_->setPath(QPainterPath());

  if ( 0 == objects_.size()) {
    return boundingRect;
  }
  Document* docum = doc();
  if ( nullptr == docum ) {
    return boundingRect;
  }
  foreach (Object* o, objects_) {
    if ( nullptr == o ) {
      continue;
    }
    if ( true == o->wasdrawed() ) {
      QTransform trans = docum->transform();
      QList<QRect> list = o->boundingRect(trans);
      if (list.size() == 0) {
        continue;
      }
      boundingRect = list.first();
      break;
    }
  }
  foreach (Object* o, objects_) {
    if ( ( nullptr == o )
         || ( false == o->visible() )
         || ( false == o->wasdrawed() ) ) {
      continue;
    }
    foreach (QRect rect, o->boundingRect(docum->transform())) {
      if ( rect.left() < boundingRect.left() ) {
        boundingRect.setLeft( rect.left() );
      }
      if ( rect.top() < boundingRect.top() ) {
        boundingRect.setTop(rect.top());
      }
      if ( rect.right() > boundingRect.right() ) {
        boundingRect.setRight(rect.right());
      }
      if( rect.bottom() > boundingRect.bottom() ) {
        boundingRect.setBottom(rect.bottom());
      }
    }
    if ( o->type() == kPolygon ||  o->type() == kIsoline) {
      QPolygon poly = docum->coord2screen(o->skelet());
      if ( poly.size() > 0 ) {
        QPainterPath path(poly.first());
        for ( int i=1,isz=poly.size(); i<isz; ++i ) {
          path.lineTo(poly.at(i));
        }

        QPainterPath pp = path_->path();
        pp.addPath(path);
        path_->setPath(pp);
      }
    }
    if ( o->type() == kText || o->type() == kPixmap ) {
      QPolygon poly = docum->coord2screen(o->skelet());
      QPainterPath path;
      if ( poly.size() > 0 ) {
        path.addEllipse(poly.first(), 3, 3);
      }
      QList<QRect> brect = o->boundingRect( docum->transform() );
      if ( 0 != brect.size() ) {
        path.addRect( brect.first() );
      }
      if ( false == path.isEmpty() ) {
        QPainterPath pp = path_->path();
        pp.addPath(path);
        path_->setPath(pp);
      }
    }
  }
  if (invisible_) {
    path_->setPath(QPainterPath());
  }
  boundingRect_ = boundingRect;
  return boundingRect;
}

void GroupObject::paint(QPainter* painter, const QStyleOptionGraphicsItem* o, QWidget* w)
{
  Q_UNUSED(o);
  Q_UNUSED(w);
  if (invisible_) {
    return;
  }
  painter->save();
  QBrush brush(Qt::black);
  QPen pen(brush, 2);
  painter->setBrush(brush);
  painter->setPen(pen);
  painter->drawLine(boundingRect_.topLeft(), boundingRect_.topRight());
  painter->drawLine(boundingRect_.topRight(), boundingRect_.bottomRight());
  painter->drawLine(boundingRect_.bottomLeft(), boundingRect_.topLeft());
  painter->drawLine(boundingRect_.bottomLeft(), boundingRect_.bottomRight());
  painter->restore();
//  QGraphicsObject::paint(painter, o, w);
}

void GroupObject::addObject(const Object* object)
{
  if ( 0 != objects_.count(const_cast<Object*>(object)) ) {
    return;
  }
  if ( nullptr == path_ ) {
    if ( scene() != nullptr ) {
      setItem( scene()->addPath(QPainterPath() ) );
    }
    else {
      error_log << QObject::tr("Ошибка. Поместите группу на сцену");
    }
  }
  if ( object->type() == kGroup ) {
    Object* obj = const_cast<Object*>(object);
    GeoGroup* group = static_cast<GeoGroup*>(obj);
    std::unordered_set<Object*> list = group->objects();
    listGeoGroup_.insert(group, list);
    foreach (Object* o, list) {
      addObject(o);
    }
    group->ungroup();
  }
  else {
    objects_.insert(const_cast<Object*>(object));
  }
}

void GroupObject::removeObject(const Object* object)
{
  objects_.erase(const_cast<Object*>(object));
}

void GroupObject::setItem(QGraphicsPathItem *path)
{
  path_ = path;
  QPen pen(Qt::darkMagenta);
  pen.setWidth(5);
  pen.setStyle(Qt::PenStyle::DotLine);
  path_->setPen(pen);

}

void GroupObject::finish()
{
  if ( false == listGeoGroup_.isEmpty()) {
    foreach (GeoGroup* geoGroup, listGeoGroup_.keys()) {
      delete geoGroup;
      geoGroup = nullptr;
    }
  }
  if ( objects_.size() <= 1 ) {
    return;
  }
  if ( nullptr == geoGroup_ ) {
    geoGroup_ = new GeoGroup((*objects_.begin())->layer());
  }

  foreach (Object* o, objects_) {
    geoGroup_->add(o);
  }
}

void GroupObject::abort()
{
  if ( true == listGeoGroup_.isEmpty() ) {
    return;
  }
  foreach (GeoGroup* geoGroup, listGeoGroup_.keys()) {
    std::unordered_set<Object*> list = listGeoGroup_.value(geoGroup);
    foreach (Object* o, list) {
      geoGroup->add(o);
    }
  }
}

void GroupObject::deleteObjects()
{
  if ( false == listGeoGroup_.isEmpty()) {
    foreach (GeoGroup* geoGroup, listGeoGroup_.keys()) {
      delete geoGroup;
      geoGroup = nullptr;
    }
  }
  foreach (Object* o, objects_) {
    delete o;
    o = nullptr;
  }
}

void GroupObject::setInvisible(bool invisible)
{
  invisible_ = invisible;
  if (scene() == nullptr) {
    return;
  }
  if (invisible) {
    scene()->removeItem(path_);
  }
  else {
    scene()->addItem(path_);
  }
}

void GroupObject::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
  Q_UNUSED(event);
  state_ = kMove;
//  QGraphicsObject::mousePressEvent(event);
}

void GroupObject::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
  if ( kMove == state_) {
    Document* docum = doc();
    Projection* proj = projection();
    if ( nullptr != proj && nullptr != doc()) {
      GeoPoint gpold = docum->screen2coord(event->lastPos());
      GeoPoint gpnew = docum->screen2coord(event->pos());;
      QPointF oldcart;
      QPointF newcart;
      proj->F2X_one( gpold, &oldcart );
      proj->F2X_one( gpnew, &newcart );
      QPointF delta = newcart - oldcart;
      QPointF cartCoord;
      foreach (Object* o, objects_) {
        GeoVector skelet = o->skelet();
        for (int i = 0; i < skelet.size(); ++i) {
          GeoPoint gp = skelet.at(i);
          proj->F2X_one( gp, &cartCoord);
          cartCoord = cartCoord + delta;
          proj->X2F_one(cartCoord, &gp);
          skelet.replace(i, gp);
        }
        o->setSkelet(skelet);
        scene()->update();
      }
    }
    emit skeletChanged();
  }
  //  QGraphicsObject::mouseMoveEvent(event);
}

void GroupObject::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
  Q_UNUSED(event);
  state_ = kNone;
//  QGraphicsObject::mouseReleaseEvent(event);
}

MapScene* GroupObject::mapScene() const
{
  return qobject_cast<MapScene*>(scene());
}

Document* GroupObject::doc() const
{
  MapScene* mapSc = mapScene();
  if ( nullptr != mapSc ) {
    return mapSc->document();
  }
  return nullptr;
}

Projection* GroupObject::projection() const
{
  Document* document = doc();
  if ( nullptr != document ) {
    return document->projection();
  }
  return nullptr;
}

}
}
