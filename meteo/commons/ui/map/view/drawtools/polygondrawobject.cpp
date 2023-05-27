#include "polygondrawobject.h"
#include "graphicsgroupitem.h"

#include <qevent.h>

#include <cross-commons/debug/tlog.h>

#include <commons/geobasis/geopoint.h>

#include <meteo/commons/ui/graphitems/markeritem.h>
#include <meteo/commons/ui/map/isoline.h>
#include <meteo/commons/ui/map/view/drawtools/geoobjectitem.h>
#include <meteo/commons/ui/map/view/mapscene.h>
#include <meteo/commons/ui/map/view/mapview.h>
#include <meteo/commons/ui/map/view/menu.h>
#include <meteo/commons/ui/map/ramka.h>


namespace meteo {
namespace map {

PolygonDrawObject::PolygonDrawObject(MapScene* mapScene, const QString& id)
  : DrawObject(mapScene, id)
{
}

PolygonDrawObject::~PolygonDrawObject()
{
  delete object_;
}

void PolygonDrawObject::setObject(const Object* obj)
{
  if ( nullptr == scene() ) {
    return;
  }

  if ( nullptr == obj ) {
    object_ = nullptr;
    return;
  }

  if ( obj->type() != kPolygon && obj->type() !=  kIsoline) {
    debug_log << QObject::tr("Ожидается объект типа GeoPolygon");
    object_ = nullptr;
    return;
  }

  if ( nullptr != object_ ) {
    delete object_;
    object_ = nullptr;
  }

  Object* tmp = obj->copy(scene()->document()->projection());
  object_ = mapobject_cast<GeoPolygon*>(tmp);
  if ( nullptr == object_ ) {
    object_ = mapobject_cast<IsoLine *>(tmp);
  }
  if ( nullptr == object_ ) {
    delete tmp;
  }

  if ( nullptr != object_ ) {
    GeoVector v = object_->skelet();
    setObjectSkelet(v);
  }

  if ( nullptr != object_ ) {
    delete objItem_;
    objItem_ = new GeoObjectItem(scene());
    objItem_->setObject(object_);
    objItem_->setZValue(objItem_->zValue() - 100);
  }
}

void PolygonDrawObject::calcSceneCoords()
{
  for ( int i=0,isz=markers_.size(); i<isz; ++i ) {
    markers_[i]->setPos(markers_[i]->calcScenePoint());
  }
}

void PolygonDrawObject::appendNode(const QPoint& point, int beforeIdx, bool underMouseCheck)
{

  if ( nullptr == scene() ) { return; }
 Q_UNUSED(underMouseCheck);
  if ( true == underMouseCheck ) {
    for ( int i=0,isz=markers_.size(); i<isz; ++i ) {
      if ( markers_.at(i)->isUnderMouse() ) {
        return;
      }
    }
  }
  GeoPoint gp = scene()->screen2coord(point);
  appendNode(gp, beforeIdx);
}

void PolygonDrawObject::appendNode(const GeoPoint& point, int beforeIdx)
{
  if ( nullptr == scene() ) { return; }
  if ( nullptr == object_ ) { return; }

  MarkerItem* marker = new MarkerItem;
  marker->setCallback(new DrawObjectClosure(this));
  marker->setZValue(marker->zValue() + 100);
  scene()->addItem(marker);
  marker->setMovable(true);
  marker->setGeoPos(point);

  beforeIdx = ( -1 == beforeIdx ) ? markers_.size() : beforeIdx;
  markers_.insert(beforeIdx, marker);

  GeoVector tmp = object_->skelet();
  tmp.insert(beforeIdx, marker->geoPos());
  object_->setSkelet(tmp);
}

void PolygonDrawObject::insertNode(const QPoint& point, const QPoint& after)
{
  for ( int i=0,isz=markers_.size(); i<isz; ++i ) {
    if ( markers_.at(i)->calcScenePoint() == after ) {
      appendNode(point, i);
      break;
    }
  }
}

void PolygonDrawObject::insertNode(const GeoPoint& point, const GeoPoint& after)
{
  for ( int i=0,isz=markers_.size(); i<isz; ++i ) {
    if ( markers_.at(i)->geoPos() == after ) {
      appendNode(point, i);
    }
  }
}

void PolygonDrawObject::removeNode(const QPoint& point)
{
  if ( nullptr == object_ ) { return; }

  for ( int i=0,isz=markers_.size(); i<isz; ++i ) {
    if ( markers_.at(i)->calcScenePoint() == point ) {
      delete markers_[i];
      markers_.removeAt(i);

      GeoVector tmp = object_->skelet();
      tmp.remove(i);
      object_->setSkelet(tmp);

      break;
    }
  }
}

void PolygonDrawObject::removeNode(const GeoPoint& point)
{
  if ( nullptr == object_ ) { return; }

  for ( int i=0,isz=markers_.size(); i<isz; ++i ) {
    if ( markers_.at(i)->geoPos() == point ) {
      delete markers_[i];
      markers_.removeAt(i);

      GeoVector tmp = object_->skelet();
      tmp.remove(i);
      object_->setSkelet(tmp);

      break;
    }
  }
}

void PolygonDrawObject::uprost2(GeoVector* gv)
{
  Projection* proj = object_->projection();
  if ( nullptr == proj ) {
    error_log << QObject::tr("Проекция не установлена");
    return ;
  }
  if( gv->size() < 3 ) return;
  for ( int i= 0; i < gv->size()-3; i+=1) {
          QPointF p1, p2, p3;
          GeoPoint gp1 = gv->at(i);
          GeoPoint gp2 = gv->at(i+1);
          GeoPoint gp3 = gv->at(i+2);
         if(!proj->F2X_one(gp1, &p1)) { error_log << QObject::tr("линия не установлена"); return ; }
         if(!proj->F2X_one(gp2, &p2)) { error_log << QObject::tr("линия не установлена"); return ; }
         if(!proj->F2X_one(gp3, &p3)) { error_log << QObject::tr("линия не установлена"); return ; }

         float dx1 = p1.x() - p2.x();
         float dy1 = p1.y() - p2.y();//y1 - y2
         float dx2 = p3.x() - p2.x();//x3 - x2
         float dy2 = p3.y() - p2.y();//y3 - y2
         float a = dx1*dy2 + dy1*dx2;
         float b = dx1*dx2 + dy1*dy2;
         debug_log <<"****"<<i<<  a<<b<<a/b;
    }
}

void PolygonDrawObject::uprost(GeoVector* gv)
{
  Projection* proj = object_->projection();
  if ( nullptr == proj ) {
    error_log << QObject::tr("Проекция не установлена");
    return ;
  }
  if( gv->size() < 3 ) return;
  bool n = false;
  do{
      GeoVector ngv;
      n = false;
      ngv.append(gv->at(0));

      for ( int i= 0; i < gv->size()-3; i+=1) {
          QPointF p1, p2, p3;
          GeoPoint gp1 = gv->at(i);
          GeoPoint gp2 = gv->at(i+1);
          GeoPoint gp3 = gv->at(i+2);
         if(!proj->F2X_one(gp1, &p1)) { error_log << QObject::tr("линия не установлена"); return ; }
         if(!proj->F2X_one(gp2, &p2)) { error_log << QObject::tr("линия не установлена"); return ; }
         if(!proj->F2X_one(gp3, &p3)) { error_log << QObject::tr("линия не установлена"); return ; }
         float dx1 = p1.x() - p2.x();
         float dy1 = p1.y() - p2.y(); //y1 - y2
         float dx2 = p3.x() - p2.x();//x3 - x2
         float dy2 = p3.y() - p2.y();//y3 - y2
         float a = dx1*dy2 - dy1*dx2;
         float b = dx1*dx2 + dy1*dy2;
         float q = atan2(a,b)*(180.0/M_PI);
         if(fabs(q) < 170. ) {
            //  debug_log <<"****"<<i<<  q;
             ngv.append(gp2);
           } else {
             i++;
             n = true;
             ngv.append(gp3);
          //   debug_log <<"----"<<i<<  q;
           }
        }
      ngv.append(gv->at(gv->size()-2));
      ngv.append(gv->at(gv->size()-3));

      (*gv) = ngv;
    }while(n);

}

void PolygonDrawObject::slotChangeMarkerPos( QPointF delta )
{
 if ( QPointF(0,0) == delta) {
   return;
 }
 QPolygon ramka;
 if ( ProjType::MERCAT == scene()->document()->projection()->type() ) {
   ramka = scene()->document()->ramka()->calcRamka(10, 0);
 }
 else  {
   ramka = scene()->document()->ramka()->calcRamka(-10, 0);
 }
 for ( int i=0,isz=markers_.size(); i<isz; ++i ) {
   GeoPoint geoP = markers().at(i)->geoPos();
   QPoint markerCartesianCoord;
   scene()->document()->projection()->F2X_one( geoP, &markerCartesianCoord);
   markerCartesianCoord = markerCartesianCoord + delta.toPoint();
   scene()->document()->projection()->X2F_one(markerCartesianCoord, &geoP);
   QPoint screenP =scene ()->document()->coord2screen(geoP);
   if (!ramka.containsPoint(screenP, Qt::WindingFill)) {
     setMovable(false);
     calcSceneCoords();
     update();
     setMovable(true);
     return;
   }
 }
 for ( int i=0,isz=markers_.size(); i<isz; ++i ) {
   GeoPoint geoP = markers().at(i)->geoPos();
   QPoint markerCartesianCoord;
   scene()->document()->projection()->F2X_one( geoP, &markerCartesianCoord);
   markerCartesianCoord = markerCartesianCoord + delta.toPoint();
   scene()->document()->projection()->X2F_one(markerCartesianCoord, &geoP);
   markers().at(i)->setGeoPos(geoP);
 }
}

void PolygonDrawObject::slotSkeletPos()
{
  deleteMarkers();
  GeoVector skelet = object_->skelet();
  for ( int i=0,isz=skelet.size(); i<isz; ++i ) {
    appendNode(skelet.at(i));
  }
  update();
}

void PolygonDrawObject::update()
{
  if ( nullptr == scene() ) { return; }
  if ( markers_.size() == 0 ) { return; }

  QPainterPath path(markers_.first()->calcScenePoint());
  for ( int i=1,isz=markers_.size(); i<isz; ++i ) {
    path.lineTo(markers_.at(i)->calcScenePoint());
  }

  if ( nullptr == skeletItem_ ) {
    skeletItem_ = scene()->addPath(path);
  }
  else {
    skeletItem_->setPath(path);
  }

  GeoVector v;
  for ( int i=0,isz=markers_.size(); i<isz; ++i ) {
    v.append(markers_.at(i)->geoPos());
  }

//  uprost(&v);

  object_->setSkelet(v);

  if ( nullptr != objItem_ ) {
    objItem_->setProperty(object_->property());
    objItem_->setSkelet(object_->skelet());
  }
}



void PolygonDrawObject::setObjectProperty(const meteo::Property& prop)
{
  if ( nullptr == object_ ) { return; }

  object_->setProperty(prop);
}

void PolygonDrawObject::setObjectValue(double value, const QString& format, const QString& unit)
{
  if ( nullptr == object_ ) { return; }

  object_->setValue(value, format, unit);
}

void PolygonDrawObject::removeObjectValue()
{
  if ( nullptr == object_ ) { return; }

  object_->removeValue();
}

GeoVector PolygonDrawObject::objectSkelet() const
{
  return ( nullptr != object_ ) ? object_->skelet() : GeoVector();
}

meteo::Property PolygonDrawObject::objectProperty() const
{
  return ( nullptr != object_ ) ? object_->property() : meteo::Property();
}

double PolygonDrawObject::objectValue(bool* hasValue) const
{
  if (nullptr != hasValue && nullptr != object_ ) {
    *hasValue = object_->hasValue();
  }

  return ( nullptr != object_ ) ? object_->value() : 0.;
}

QString PolygonDrawObject::objectValueFormat() const
{
  not_impl;

  return QString("4' '.2'0'");
}

QString PolygonDrawObject::objectValueUnit() const
{
  not_impl;

  return QString();
}

void PolygonDrawObject::removeFromScene()
{
  delete skeletItem_;
  skeletItem_ = nullptr;

  delete objItem_;
  objItem_ = nullptr;

  delete object_;
  object_ = nullptr;

  deleteMarkers();

}

void PolygonDrawObject::markerChanged(MarkerItem* marker)
{
  if ( nullptr == marker ) { return; }
  if ( !markers_.contains(marker) ) { return; }

  switch ( marker->state() ) {
    case MarkerItem::kNormal: /*debug_log << "normal";*/ break;
    case MarkerItem::kHover:  /*debug_log << "hover";*/ break;
    case MarkerItem::kPress:  /*debug_log << "press";*/ break;
    case MarkerItem::kRelease: /*debug_log << "release";*/ break;
    case MarkerItem::kClick:  /*debug_log << "click";*/ break;
    case MarkerItem::kDrag:
    case MarkerItem::kDrop:
    case MarkerItem::kGroupDrag: {
      update();
    } break;
  }
}

QList<QAction*> PolygonDrawObject::setupContextMenu(Menu* menu, const QPoint& screenPos)
{
  QList<QAction*> acts;

  QList<QGraphicsItem*> items = scene()->mapview()->items(QRect(screenPos,QSize(10,10)));
  for ( int i=0,isz=items.size(); i<isz; ++i ) {
    MarkerItem* m = qgraphicsitem_cast<MarkerItem*>(items.at(i));
    if ( markers_.contains(m) && nullptr == rmAct_ ) {
      rmAct_ = new QAction(QObject::tr("Удалить узел"), menu);
      rmAct_->setData(QVariant::fromValue(items.at(i)));
      menu->addObjectAction(rmAct_);
      acts << rmAct_;
    }
    else if ( items.at(i) == skeletItem_ ) {
      QPoint scenePoint = scene()->mapview()->mapToScene(screenPos).toPoint();
      QLine line = skeletSectionAt(scenePoint, 20);
      if ( !line.isNull() ) {
        QRect r(scenePoint, line.p2());
        addAct_ = new QAction(QObject::tr("Добавить узел"), menu);
        addAct_->setData(QVariant::fromValue(r));
        menu->addObjectAction(addAct_);
        acts << addAct_;
      }
    }
  }

  return acts;
}

void PolygonDrawObject::processContextMenu(QAction* act)
{
  if ( nullptr == act ) { return; }

  if ( act == addAct_ ) {
    QRect r = act->data().toRect();
    insertNode(r.topLeft(), r.bottomRight());
  }
  else if ( act == rmAct_ ) {
    MarkerItem* m = qgraphicsitem_cast<MarkerItem*>(act->data().value<QGraphicsItem*>());
    if ( nullptr != m ) {
      removeNode(m->calcScenePoint());
      update();
    }
  }

  addAct_ = nullptr;
  rmAct_ = nullptr;
}

QLine PolygonDrawObject::skeletSectionAt(const QPoint& scenePoint, int width) const
{
  if ( width < 1 ) {
    debug_log << QObject::tr("Невозможно определить секцию если width < 1");
    return QLine();
  }

  for ( int i=1,isz=markers_.size(); i<isz; ++i ) {
    QLine line(markers_[i-1]->calcScenePoint(), markers_[i]->calcScenePoint());
    QPolygon p = lineGeometry(line, width);
    if ( p.containsPoint(scenePoint,Qt::WindingFill) ) {
      return line;
    }
  }

  return QLine();
}

QList<MarkerItem*> PolygonDrawObject::skeletMarkersAt(const QPoint& scenePoint) const
{
  QList<QGraphicsItem*> items = scene()->items(scenePoint);

  QList<MarkerItem*> markersAtPoint;
  for ( int i=0,isz=items.size(); i<isz; ++i ) {
    MarkerItem* item = qgraphicsitem_cast<MarkerItem*>(items.at(i));
    if ( markers_.contains(item) ) {
      markersAtPoint << item;
    }
  }

  return markersAtPoint;
}

MarkerItem* PolygonDrawObject::lastMarker() const
{
  if ( false == markers_.isEmpty() ) {
    return markers_.last();
  }
  return nullptr;
}

MarkerItem* PolygonDrawObject::firstMarker() const
{
  if ( markers_.count() < 2 ) {
    return nullptr;
  }
  return markers_.first();
}

int PolygonDrawObject::markersCount() const
{
  return markers_.count();
}

void PolygonDrawObject::setObjectSkelet(const QPolygon& skelet)
{
  if ( nullptr == scene() ) { return; }

  GeoVector v = scene()->document()->screen2coord(skelet);
  setObjectSkelet(v);
}

void PolygonDrawObject::setObjectSkelet(const GeoVector& skelet)
{
  if ( nullptr == object_ ) { return; }

  object_->setSkelet(GeoVector());

  deleteMarkers();

  for ( int i=0,isz=skelet.size(); i<isz; ++i ) {
    appendNode(skelet.at(i));
  }
}

void PolygonDrawObject::setMovable(bool enable)
{
  if (enable && nullptr == grObject_) {
    grObject_ = new GroupObject();
    grObject_->setInvisible(true);
    QObject::connect(grObject_, SIGNAL(skeletChanged()), SLOT(slotSkeletPos()));
    scene()->addItem(grObject_);
    grObject_->addObject(object_);
  }
  else {
    if (nullptr != grObject_) {
      grObject_->removeObject(object_);
      scene()->removeItem(grObject_);
      delete grObject_;
      grObject_ = nullptr;
      deleteMarkers();
      GeoVector skelet = object_->skelet();
      for ( int i=0,isz=skelet.size(); i<isz; ++i ) {
        appendNode(skelet.at(i));
      }
    }
  }
  movable_ = enable;
}

bool PolygonDrawObject::isMovable() const
{
  return movable_;
}

void PolygonDrawObject::mouseMoveEvent(QMouseEvent* e)
{
  if ( nullptr == scene() ) { return; }
  QLine section = skeletSectionAt(scene()->mapview()->mapToScene(e->pos()).toPoint(), 5);
  if ( !section.isNull() ) {
    if ( nullptr != skeletItem_ ) {
      skeletItem_->setCursor(Qt::SizeAllCursor);
    }
    else {
      debug_log << QObject::tr("Невозможно установить вид курсор отсутствует skeletItem_");
      return;
    }
  }
}

void PolygonDrawObject::deleteMarkers()
{
  for ( int i=0,isz=markers_.size(); i<isz; ++i ) {
    delete markers_.at(i);
  }
  markers_.clear();
}


} // map
} // meteo
