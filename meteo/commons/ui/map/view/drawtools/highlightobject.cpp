#include "highlightobject.h"

#include <qgraphicsitem.h>

#include <cross-commons/debug/tlog.h>
#include <meteo/commons/ui/map/object.h>
#include <meteo/commons/ui/map/view/mapscene.h>


namespace meteo {
namespace map {

HighlightObject::HighlightObject(const Object* object, MapScene* scene)
  : object_(object),
    scene_(scene),
    sceneItem_(0)
{
  if ( 0 == object ) { return; }
  assert_log( 0 != object );

  if ( 0 != scene_ && (object_->type() == kPolygon || object_->type() == kIsoline )) {
    sceneItem_ = scene_->addPath(QPainterPath());
    QPen pen(Qt::red);
    pen.setWidth(4);
    sceneItem_->setPen(pen);
  }
  if ( 0 != scene_ && object_->type() == kText ) {
    sceneItem_ = scene_->addPath(QPainterPath());
    QPen pen(Qt::red);
    pen.setWidth(2);
    sceneItem_->setPen(pen);
    sceneItem_->setBrush(QBrush(Qt::NoBrush));
  }
  if ( 0 != scene_ && object_->type() == kPixmap ) {
    sceneItem_ = scene_->addPath(QPainterPath());
    QPen pen(Qt::red);
    pen.setWidth(2);
    sceneItem_->setPen(pen);
    sceneItem_->setBrush(QBrush(Qt::NoBrush));
  }

  const std::unordered_set<Object*>& tmp = object_->childs();
  foreach(Object* o, tmp){
    childs_.append(new HighlightObject(o, scene_));
  }

  calcSceneCoords();
}

HighlightObject::~HighlightObject()
{
  for ( int i=0,isz=childs_.size(); i<isz; ++i ) {
    delete childs_[i];
  }
  childs_.clear();
}

void HighlightObject::calcSceneCoords()
{
  if ( 0 == object_ ) {
    debug_log << QObject::tr("Невозможно пересчитать координаты, т.к. отсутствует географический объект");
    return;
  }

  if ( object_->type() == kPolygon ||  object_->type() == kIsoline) {
    QPolygon poly = scene_->document()->coord2screen(object_->skelet());
    if ( poly.size() > 0 ) {
      QPainterPath path(poly.first());
      for ( int i=1,isz=poly.size(); i<isz; ++i ) {
        path.lineTo(poly.at(i));
      }
      sceneItem_->setPath(path);
    }
  }
  if ( object_->type() == kText || object_->type() == kPixmap ) {
    QPolygon poly = scene_->document()->coord2screen(object_->skelet());
    QPainterPath path;
    if ( poly.size() > 0 ) {
      path.addEllipse(poly.first(), 3, 3);
    }
    QList<QRect> brect = object_->boundingRect( scene_->document()->transform() );
    if ( 0 != brect.size() ) {
      path.addRect( brect.first() );
    }
    if ( false == path.isEmpty() ) {
      sceneItem_->setPath(path);
    }
  }

  for ( int i=0,isz=childs_.size(); i<isz; ++i ) {
    childs_[i]->calcSceneCoords();
  }
}

bool HighlightObject::isUnderMouse() const
{
  if ( 0 != sceneItem_ && sceneItem_->isUnderMouse() ) {
    return true;
  }

  for ( int i=0,isz=childs_.size(); i<isz; ++i ) {
    if ( childs_[i]->isUnderMouse() ) {
      return true;
    }
  }

  return false;
}

bool HighlightObject::isPointOnObject(const QPointF& scenePoint, int width) const
{
  if ( width < 1 ) {
    debug_log << QObject::tr("Невозможно определить секцию если width < 1");
    return false;
  }

  if ( 0 != object_ ) {
    if ( width > object_->minimumScreenDistance(scenePoint.toPoint()) ) {
      return true;
    }
  }

  for ( int i=0,isz=childs_.size(); i<isz; ++i ) {
    if ( childs_[i]->isPointOnObject(scenePoint, width) ) {
      return true;
    }
  }

  return false;
}

bool HighlightObject::contains(const Object* obj) const
{
  if ( 0 == object_ ) { return false; }

  if ( object_ == obj ) {
    return true;
  }

  for ( int i=0,isz=childs_.size(); i<isz; ++i ) {
    if ( childs_.at(i)->contains(obj) ) {
      return true;
    }
  }

  return false;
}

void HighlightObject::removeFromScene()
{
  delete sceneItem_;
  sceneItem_ = 0;

  for ( int i=0,isz=childs_.size(); i<isz; ++i ) {
    childs_[i]->removeFromScene();
    delete childs_[i];
  }
  childs_.clear();
}

QPolygon HighlightObject::lineGeometry(const QLine& source, double width) const
{
  int x1 = source.x1();
  int y1 = source.y1();
  int x2 = source.x2();
  int y2 = source.y2();

  double angle = atan2(double(y2-y1),double(x2-x1)) + M_PI/2;
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
