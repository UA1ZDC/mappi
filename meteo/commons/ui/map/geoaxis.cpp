#include "geoaxis.h"

#include <qpainter.h>

#include <commons/geom/geom.h>
#include <meteo/commons/ui/map/document.h>
#include <cross-commons/debug/tlog.h>

#include "object.h"
#include "geotext.h"
#include "puanson.h"

namespace meteo {
namespace map {

static const bool debugDraw = false;
static const int kTickLength = 6;

GeoAxis::GeoAxis(Layer* layer, Orientation orientation)
  : Object(layer),
    orientation_(orientation)
{
  init();
}

GeoAxis::GeoAxis(Object* parent, Orientation orientation)
  : Object(parent),
    orientation_(orientation)
{
  init();
}

GeoAxis::GeoAxis(Projection* proj, Orientation orientation)
  : Object(proj),
    orientation_(orientation)
{
  init();
}

GeoAxis::GeoAxis(Orientation orientation, const meteo::Property& prop)
  : Object(prop),
    orientation_(orientation)
{
  init();
}

GeoAxis::~GeoAxis()
{
  delete labelTickPrototype_;
  labelTickPrototype_ = nullptr;
  for ( int i = 0, sz = beginLabels_.size(); i < sz; ++i ) {
    delete beginLabels_[i].first;
  }
  for ( int i = 0, sz = endLabels_.size(); i < sz; ++i ) {
    delete endLabels_[i].first;
  }
}

Object* GeoAxis::copy(Layer* layer) const
{
  debug_log_if(nullptr == layer) << "layer == 0";

  if ( nullptr == layer ) { return nullptr; }

  GeoAxis* axis = new GeoAxis(layer);
  axis->setProperty(property_);
  axis->setSkelet(skelet());

  foreach(Object* o, objects_){
    o->copy(axis);
  }

  return axis;
}

Object* GeoAxis::copy(Object* parent) const
{
  if ( 0 == parent ) { return 0; }

  GeoAxis* axis = new GeoAxis(parent);
  axis->setProperty(property_);
  axis->setSkelet(skelet());

  foreach(Object* o, objects_){
    o->copy(axis);
  }

  return axis;
}

Object* GeoAxis::copy(Projection* proj) const
{
  if ( 0 == proj ) { return 0; }

  GeoAxis* axis = new GeoAxis(proj);
  axis->setProperty(property_);
  axis->setSkelet(skelet());

  foreach(Object* o, objects_){
    o->copy(axis);
  }

  return axis;
}

QList<QRect> GeoAxis::boundingRect(const QTransform& transform) const
{
  QList<QPoint> points;
  for ( int i=0,isz=cartesianSkelet().size(); i<isz; ++i ) {
    QPolygon screen = transform.map(cartesianSkelet()[i]);
    for ( int j=0,jsz=screen.size(); j<jsz; ++j ) {
      points.append(screen[j]);
    }
  }

  QRect baseRect;

  if ( points.size() >= 2 ) {
    // координаты оси на сцене
    QPoint sceneP1 = points[0];
    QPoint sceneP2 = points[1];
    if ( kVertical == orientation_ ) {
      sceneP2.setX(sceneP1.x());
    }
    else {
      sceneP2.setY(sceneP1.y());
    }

    // расчёты для шкалы
    QPoint tmp( pen().width() + kTickLength, pen().width() + kTickLength) ;

    baseRect.setTopLeft(sceneP1 - tmp);
    baseRect.setBottomRight(sceneP2 + tmp);

    // расчёты для подписей

    // длина оси в масштабе шкалы
    float axisD = rangeUpper_ - rangeLower_;

    // объекты-прототипы
    GeoText* textProto = 0;
    Puanson* puansonProto = 0;

    if ( 0 != labelTickPrototype_  ) {
      if ( labelTickPrototype_->type() == kText ) {
        textProto = static_cast<GeoText*>(labelTickPrototype_);
      }
      if ( labelTickPrototype_->type() == kPuanson ) {
        puansonProto = static_cast<Puanson*>(labelTickPrototype_);
      }
    }

    // для прототипа GeoText
    for ( int i=0,isz=tickValues_.size(); i<isz; ++i ) {
      if ( i >= tickLabels_.size() ) { break; }
      if ( 0 == textProto ) { break; }

      if ( 0 == axisD ) { continue; }

      float val = tickValues_.at(i);

      if ( val < rangeLower_ || val > rangeUpper_ ) {
        continue;
      }

      GeoVector gv;
      gv.append(tickGeoPoint(val));
      textProto->setSkelet(gv);

      textProto->setPos( ( kVertical == orientation_ ) ? kLeftCenter : kTopCenter );
      textProto->setText(tickLabels_[i]);

      QList<QRect> textRects = textProto->boundingRect(transform);
      for ( int i=0,isz=textRects.size(); i<isz; ++i ) {
        textRects[i].translate(0, -kTickLength*1.5);
        baseRect |= textRects.at(i);
      }
    }

    // для прототипа Puanson


    for ( int i=0,isz = tickValues_.size(); i<isz; ++i ) {
      if ( 0 == puansonProto ) { break; }
      if ( 0 == axisD ) { continue; }

      float val = tickValues_.at(i);

      GeoVector gv;
      gv.append(tickGeoPoint(val));
      puansonProto->setSkelet(gv);

      puansonProto->setPos( (kVertical == orientation_ ) ? kLeftCenter : kTopCenter );

      TMeteoData indx_data = meteo_data_.value(val);

      if(indx_data.count()< 1){
        continue;
      }
      puansonProto->setMeteodata(indx_data);
      //indx_data.printData();

      QList<QRect> puansonRects = puansonProto->boundingRect(transform);
      for ( int i=0,isz=puansonRects.size(); i<isz; ++i ) {
        baseRect |= puansonRects.at(i);
      }
    }

    // для надписей (в начале)
    for ( int i=0,isz=beginLabels_.size(); i<isz; ++i ) {
      QPoint offset(0,0);
      if ( kHorizontal == orientation_ ) {
        offset.rx() -= beginLabels_[i].second.x();
        offset.ry() -= beginLabels_[i].second.y();
      }
      else {
        offset.rx() -= beginLabels_[i].second.y();
        offset.ry() -= beginLabels_[i].second.x();
      }

      QList<QRect> rects = beginLabels_[i].first->boundingRect(transform);
      for ( int i=0,isz=rects.size(); i<isz; ++i ) {
        rects[i].translate(offset.x(), offset.y());
        baseRect |= rects.at(i);
      }
    }

    // для надписей (в конце)
    for ( int i=0,isz=endLabels_.size(); i<isz; ++i ) {
      QPoint offset(0,0);
      if ( kHorizontal == orientation_ ) {
        offset.rx() += endLabels_[i].second.x();
        offset.ry() += endLabels_[i].second.y();
      }
      else {
        offset.rx() += endLabels_[i].second.y();
        offset.ry() += endLabels_[i].second.x();
      }

      QList<QRect> rects = endLabels_[i].first->boundingRect(transform);
      for ( int i=0,isz=rects.size(); i<isz; ++i ) {
        rects[i].translate(offset.x(), offset.y());
        baseRect |= rects.at(i);
      }
    }
  }

  QList<QRect> list = QList<QRect>() << baseRect;

  return list;
}

bool GeoAxis::render(QPainter* painter, const QRect& target, const QTransform& transform)
{
  painter->save();

  painter->setPen( qpen() );

  if ( cartesian_points_.size() > 0 && cartesian_points_[0].size() >= 2 ) {
    // координаты оси на сцене
    QPoint sceneP1 = transform.map(cartesian_points_[0][0]);
    QPoint sceneP2 = transform.map(cartesian_points_[0][1]);
    if ( kVertical == orientation_ ) {
      sceneP2.setX(sceneP1.x());
    }
    else {
      sceneP2.setY(sceneP1.y());
    }
    // длина оси в масштабе шкалы
    float axisD = rangeUpper_ - rangeLower_;

    // рисуем ось
    painter->drawLine(QLine(sceneP1,sceneP2));

    // рисуем засечки
    for ( int i=0,isz=tickValues_.size(); i<isz; ++i ) {
      float val = tickValues_.at(i);

      if ( 0 == axisD ) {
        debug_log << QObject::tr("Невозможно расчитать положение засечки, длина шкалы равна 0.");
        continue;
      }

      if ( val < rangeLower_ || val > rangeUpper_ ) {
        continue;
      }

      QPoint p;
      if ( kVertical == orientation_ ) {
        p.setX(kTickLength);
      }
      else {
        p.setY(kTickLength);
      }

      QPointF tickP = transform.map(tickCartesianPoint(val));
      QPointF tickP1 = tickP - p;
      QPointF tickP2 = tickP + p;

      painter->drawLine(QLineF(tickP1,tickP2));
    }

    // объекты-прототипы
    GeoText* textProto = 0;
    Puanson* puansonProto = 0;

    if ( 0 != labelTickPrototype_  ) {
      if ( labelTickPrototype_->type() == kText ) {
        textProto = static_cast<GeoText*>(labelTickPrototype_);
      }
      if ( labelTickPrototype_->type() == kPuanson ) {
        puansonProto = static_cast<Puanson*>(labelTickPrototype_);
      }
    }

    debug_log_if( tickLabels_.size() > 0 && 0 == textProto )
        << QObject::tr("Невозможно отобразить подписи засечек, не задан объект-прототип (text).");

    // рисуем подписи для засечек
    painter->save();

    if ( kHorizontal == orientation_ ) {
      QTransform trans = painter->transform();
      trans.translate(0, -kTickLength*1.5);
      painter->setTransform(trans);
    }

    // для прототипа GeoText
    for ( int i=0,isz=tickValues_.size(); i<isz; ++i ) {
      if ( i >= tickLabels_.size() ) { break; }
      if ( 0 == textProto ) { break; }

      if ( 0 == axisD ) {
        debug_log << QObject::tr("Невозможно расчитать положение надписи для засечки, длина шкалы равна 0.");
        continue;
      }

      float val = tickValues_.at(i);

      if ( val < rangeLower_ || val > rangeUpper_ ) {
        continue;
      }

      textProto->setPos( ( kVertical == orientation_ ) ? kLeftCenter : kTopCenter );
      textProto->setText(tickLabels_[i]);

      GeoVector gv;
      gv.append(tickGeoPoint(val));
      textProto->setSkelet(gv);

      textProto->render(painter, target, transform);
    }

    painter->restore();

    // для прототипа Puanson
    for ( int i=0,isz = tickValues_.size(); i<isz; ++i ) {
      if ( 0 == puansonProto ) { break; }
      if ( 0 == axisD ) {
        debug_log << QObject::tr("Невозможно расчитать положение надписи для засечки, длина шкалы равна 0.");
        continue;
      }

      float val = tickValues_.at(i);
      puansonProto->setPos( ( kVertical == orientation_ ) ? kLeftCenter : kTopCenter );

      TMeteoData indx_data = meteo_data_.value(val);

      if(indx_data.count()< 1){
        continue;
      }

    //  indx_data.printData();
      puansonProto->setMeteodata(indx_data);
      GeoVector gv;
      gv.append(tickGeoPoint(val));
      puansonProto->setSkelet(gv);

      puansonProto->render(painter, target, transform);
    }
  }

  painter->restore();

  // рисуем надписи (в начале)
  for ( int i=0,isz=beginLabels_.size(); i<isz; ++i ) {
    QPoint offset(0,0);
    if ( kHorizontal == orientation_ ) {
      offset.rx() -= beginLabels_[i].second.x();
      offset.ry() -= beginLabels_[i].second.y();
    }
    else {
      offset.rx() -= beginLabels_[i].second.y();
      offset.ry() -= beginLabels_[i].second.x();
    }

    painter->save();

    QTransform trans = painter->transform();
    trans.translate(offset.x(), offset.y());
    painter->setTransform(trans);

    beginLabels_[i].first->render(painter, target, transform);

    painter->restore();
  }

  // рисуем надписи (в конце)
  for ( int i=0,isz=endLabels_.size(); i<isz; ++i ) {
    QPoint offset(0,0);
    if ( kHorizontal == orientation_ ) {
      offset.rx() += endLabels_[i].second.x();
      offset.ry() += endLabels_[i].second.y();
    }
    else {
      offset.rx() += endLabels_[i].second.y();
      offset.ry() += endLabels_[i].second.x();
    }

    painter->save();

    QTransform trans = painter->transform();
    trans.translate(offset.x(), offset.y());
    painter->setTransform(trans);

    endLabels_[i].first->render(painter, target, transform);

    painter->restore();
  }

  foreach(Object* o, objects_){
    painter->save();
    o->render( painter, target, transform );
    painter->restore();
  }


  if ( debugDraw )
  {
    QRect r = boundingRect(transform).first();
    painter->save();

    QPen pen(Qt::blue, 1, Qt::DashDotLine);
    painter->setPen(pen);

    r.setWidth(r.width() - 1);
    r.setHeight(r.height() - 1);
    painter->drawRect(r);

    painter->restore();
  }


  return true;
}

QList<GeoVector> GeoAxis::skeletInRect(const QRect& rect, const QTransform& transform) const
{
  Q_UNUSED( rect );
  Q_UNUSED( transform );

  not_impl;

  return QList<GeoVector>();
}

void GeoAxis::setSkelet(const GeoVector& skelet)
{
  Object::setSkelet(skelet);

  updateLabelsCoord();
}

void GeoAxis::setOrientation(GeoAxis::Orientation orientation)
{
  orientation_ = orientation;
}

void GeoAxis::setRange(float lower, float upper)
{
  rangeLower_ = lower;
  rangeUpper_ = upper;
}

void GeoAxis::setTickValues(const QVector<float>& values)
{
  tickValues_ = values;
}

void GeoAxis::setTickLables(const QVector<QString>& labels)
{
  meteo_data_.clear();
  tickLabels_ = labels;
}

void GeoAxis::setTickLables(const QMap< float , TMeteoData >& meteo_data)
{
  tickLabels_.clear();
  meteo_data_ = meteo_data;
}

void GeoAxis::setTickLabelPrototype(const GeoText& prototype)
{
  if ( 0 == prototype.projection() ) {
    debug_log << QObject::tr("Объект-прототип должен содержать проекцию.");
    return;
  }

  labelTickPrototype_ = prototype.copy(prototype.projection());
}

void GeoAxis::setTickLabelPrototype(const Puanson& prototype)
{
  if ( 0 == prototype.projection() ) {
    debug_log << QObject::tr("Объект-прототип должен содержать проекцию.");
    return;
  }

  labelTickPrototype_ = prototype.copy(prototype.projection());
}

void GeoAxis::addAxisLabel(const Object& object, const QPoint& screenOffset, GeoAxis::LabelPosition position)
{
  if ( 0 == object.projection() ) {
    debug_log << QObject::tr("Объект-подпись должен содержать проекцию.");
    return;
  }

  Object* obj = object.copy(object.projection());

  if ( skelet().size() >= 1 ) {
    GeoPoint axisP = ( kBegin == position ) ? skelet().first() : skelet().last();
    moveToPoint(obj, axisP);
  }

  if ( kBegin == position ) {
    beginLabels_.append(qMakePair(obj, screenOffset));
  }
  else {
    endLabels_.append(qMakePair(obj, screenOffset));
  }
}

bool GeoAxis::underGeoPoint(const GeoPoint& gp) const
{
  Q_UNUSED( gp );

  not_impl;

  return false;
}

int GeoAxis::minimumScreenDistance(const QPoint& pos, QPoint* cross ) const
{
  int dist = 10000000;
  QTransform tr;
  if ( 0 != document() ) {
    tr = document()->transform();
  }
  QList<QRect> list  = boundingRect(tr);
  if ( 0 == list.size() ) {
    return dist;
  }
  for ( int i = 0, sz = list.size(); i < sz; ++i ) {
    dist = meteo::geom::distance( pos, list[i], cross );
  }
  return dist;
}

bool GeoAxis::hasValue() const
{
  return false;
}

float GeoAxis::value() const
{
  return -9999;
}

void GeoAxis::setValue(float val, const QString& format, const QString& unit)
{
  Q_UNUSED( val );
  Q_UNUSED( format );
  Q_UNUSED( unit );
}

GeoPoint GeoAxis::tickGeoPoint(float value) const
{
  GeoPoint tickP(0,0);
  if ( skelet().size() == 1 ) {
    tickP = GeoPoint(0,0,0,skelet().first().type());
  }
  // длина оси в масштабе шкалы
  float axisD = rangeUpper_ - rangeLower_;

  // проверяем наличи необходимых данных для расчёта
  if ( 0 == axisD ) { return tickP; }
  if ( skelet().size() == 0 ) { return tickP; }
  if ( skelet().size() == 1 ) { return tickP; }
  if ( value < rangeLower_ || value > rangeUpper_ ) { return tickP; }

  // координаты оси на плоскости (в проекции документа)
  GeoPoint projP1 = skelet().first();
  GeoPoint projP2 = skelet().last();
  // длина оси на плоскости (в проекции документа)
  float projDx = projP2.lat() - projP1.lat();
  float projDy = projP2.lon() - projP1.lon();
  // смещение засечки на шкале относительно её начала
  float tickOffset = value - rangeLower_;
  // координата засечки будет располагаться относительно начала оси,
  // со смещением, в соответствии с ориетацией
  tickP = projP1;

  if ( kVertical == orientation_ ) {
    tickP.setLon(tickP.lon() + projDy * tickOffset / axisD);
  }
  else {
    tickP.setLat(tickP.lat() + projDx * tickOffset / axisD);
  }

  return tickP;
}

QPoint GeoAxis::tickCartesianPoint(float value) const
{
  // длина оси в масштабе шкалы
  float axisD = rangeUpper_ - rangeLower_;

  // проверяем наличи необходимых данных для расчёта
  if ( 0 == axisD ) { return QPoint(); }
  if ( cartesianSkelet().size() == 0 ) { return QPoint(); }
  if ( cartesianSkelet()[0].size() < 2 ) { return QPoint(); }
  if ( value < rangeLower_ || value > rangeUpper_ ) { return QPoint(); }

  QPoint tickP;
  // координаты оси на плоскост
  QPoint cartP1 = cartesianSkelet()[0].first();
  QPoint cartP2 = cartesianSkelet()[0].last();
  // длина оси на плоскости (в проекции документа)
  float dX = cartP2.x() - cartP1.x();
  float dY = cartP2.y() - cartP1.y();
  // смещение засечки на шкале относительно её начала
  float tickOffset = value - rangeLower_;
  // координата засечки будет располагаться относительно начала оси,
  // со смещением, в соответствии с ориетацией
  tickP = cartP1;

  if ( kVertical == orientation_ ) {
    tickP.setY(tickP.y() + dY * tickOffset / axisD);
  }
  else {
    tickP.setX(tickP.x() + dX * tickOffset / axisD);
  }

  return tickP;
}

void GeoAxis::init()
{
  rangeLower_ = -10;
  rangeUpper_ = 10;

  labelTickPrototype_ = 0;
}

void GeoAxis::moveToPoint(Object* object, const GeoPoint& point) const
{
  if ( object->skelet().size() == 0 ) {
    GeoVector skelet;
    skelet.append(point);
    object->setSkelet(skelet);
  }
  else {
    GeoPoint delta = point - object->skelet().first();
    GeoVector skelet = object->skelet();
    skelet.move(delta);
    object->setSkelet(skelet);
  }
}

void GeoAxis::updateLabelsCoord()
{
  if ( skelet().size() == 0 ) { return; }

  GeoPoint begP = skelet().first();
  GeoPoint endP = skelet().last();

  for ( int i=0,isz=beginLabels_.size(); i<isz; ++i ) {
    moveToPoint(beginLabels_[i].first, begP);
  }
  for ( int i=0,isz=endLabels_.size(); i<isz; ++i ) {
    moveToPoint(endLabels_[i].first, endP);
  }
}

} // map
} // meteo
