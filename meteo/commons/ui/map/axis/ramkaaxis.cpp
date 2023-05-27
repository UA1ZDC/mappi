#include "ramkaaxis.h"

#include <qpainter.h>

#include <commons/geobasis/generalproj.h>
#include <cross-commons/debug/tlog.h>

#include <meteo/commons/ui/map/layer.h>
#include <meteo/commons/ui/map/document.h>
#include <meteo/commons/ui/map/axis/layeraxis.h>
#include <meteo/commons/ui/map/axis/axissimple.h>
#include <meteo/commons/ui/map/axis/axispuanson.h>
#include <meteo/commons/ui/map/profile/layerprofile.h>

namespace meteo {
namespace map {

static const int drawAxisRect = false;

RamkaAxis::RamkaAxis(Document* doc)
  : Ramka(doc,0)
{
}

RamkaAxis::~RamkaAxis()
{
}

bool RamkaAxis::drawOnDocument(QPainter* painter)
{
  QRect inner = calcInnerRect();
  if ( inner.isNull() ) {
    return false;
  }

  QPen pen;
  pen.setWidth(1);
  pen.setColor(Qt::gray);

  painter->save();
  painter->setPen(pen);

  QRect border = inner;
  // увеличиваем размер прямоугольника слева и сверху,
  // чтобы отрисовать рамку по внешней границе
  border.setX(border.x() - 1);
  border.setY(border.y() - 1);
  painter->drawRect(border);

  int t = border.y();
  int b = border.y() + border.height();
  int l = border.x();
  int r = border.x() + border.width();

//  var(inner) << var(l) << var(t) << var(r) << var(b);

  GeneralProj* proj = projection_cast<meteo::GeneralProj*>(document_->projection());
  if ( nullptr == proj ) {
    error_log << QObject::tr("Нулевой указатель на проекцию.");
    painter->restore();
    return false;
  }

  int offset[4];
  offset[map::AxisSimple::kLeft]   = 0;
  offset[map::AxisSimple::kRight]  = 0;
  offset[map::AxisSimple::kTop]    = 0;
  offset[map::AxisSimple::kBottom] = 0;

  const float begX = proj->startFi();
  const float endX = proj->endFi();
  const float begY = proj->startLa();
  const float endY = proj->endLa();

  const QList<map::AxisSimple*> axes = visibleAxes();
  foreach ( map::AxisSimple* a, axes ) {
    QPoint beg;
    QPoint end;
    float dataX = 0;
    float dataY = 0;

    const QRect rect = a->boundingRect(document_);

    switch ( a->axisType() ) {
      case map::AxisSimple::kLeft:
        beg = QPoint(l,b);
        end = QPoint(l,t);
        dataX = inner.x();

        beg.rx() -= offset[a->axisType()];
        end.rx() -= offset[a->axisType()];
        break;
      case map::AxisSimple::kRight:
        beg = QPoint(r,b);
        end = QPoint(r,t);
        dataX = inner.x() + inner.width();

        beg.rx() += offset[a->axisType()];
        end.rx() += offset[a->axisType()];
        break;
      case map::AxisSimple::kCenter:
        not_impl;
        break;
      case map::AxisSimple::kBottom:
        beg = QPoint(l,b);
        end = QPoint(r,b);
        dataY = inner.y() + inner.height();

        beg.ry() += offset[a->axisType()];
        end.ry() += offset[a->axisType()];
        break;
      case map::AxisSimple::kTop:
        beg = QPoint(l,t);
        end = QPoint(r,t);
        dataY = inner.y();

        beg.ry() -= offset[a->axisType()];
        end.ry() -= offset[a->axisType()];
        break;
      case map::AxisSimple::kMiddle:
        not_impl;
        break;
    }

    if ( a->isHorizontal() ) {
      offset[a->axisType()] += rect.height();
    }
    else {
      offset[a->axisType()] += rect.width();
    }

    GeoPoint gpData = document_->screen2coord(QPointF(dataX,dataY));
    float x = qBound(begX,static_cast<float>(gpData.lat()),endX);
    float y = qBound(begY,static_cast<float>(gpData.lon()),endY);
    gpData.setLat(x);
    gpData.setLon(y);

    a->updateData(gpData, document_);
    a->paint(painter, beg, end, document_);

    if ( drawAxisRect )
    {
      QRect geom = a->boundingRect(document_);
      // уменьшаем размер прямоугольника справа и снизу,
      // чтобы отрисовать рамку по внутренней границе
      geom.setWidth(geom.width() - 1);
      geom.setHeight(geom.height() - 1);

      QPen pen;
      pen.setWidth(1);
      pen.setColor(Qt::blue);
      pen.setStyle(Qt::DotLine);

      painter->save();
      painter->translate(beg.x(),beg.y());
      painter->setPen(pen);
      painter->drawRect(geom);
      painter->restore();
    }
  }

  painter->restore();

  return true;
}

QPolygon RamkaAxis::calcRamka(int mapindent, int docindent) const
{
  Q_UNUSED( mapindent );

  QRect r = calcInnerRect();
//  r.setX(r.left() - mapindent);
//  r.setY(r.top() - mapindent);
//  r.setWidth(r.width() + mapindent);
//  r.setHeight(r.height() + mapindent);

  r.setX(r.left() - docindent);
  r.setY(r.top() - docindent);
  r.setWidth(r.width() + docindent);
  r.setHeight(r.height() + docindent);

  return QPolygon(r, true);
}

QList<AxisSimple*> RamkaAxis::visibleAxes(int axisPosition) const
{
  QList<map::AxisSimple*> axes;
  for ( map::Layer* layer : document_->layers()) {
    map::LayerAxis* l = nullptr;
    if ( LayerAxis::Type == layer->type() || LayerProfile::Type == layer->type() ) {
      l = static_cast<LayerAxis*>(layer);
    }

    if ( nullptr == l || !l->visible() ) { continue; }

    map::AxisSimple* x = l->xAxis();
    for ( int i = 0, isz = axes.size(); i<isz; ++i ) {
      if ( axes.at(i)->isEqual(*x) ) {
        x = nullptr;
        break;
      }
    }
    if ( 0 != x && x->isVisible() ) { axes << x; }

    x = l->xAxis2();
    for ( int i = 0, isz = axes.size(); i<isz; ++i ) {
      if ( axes.at(i)->isEqual(*x) ) {
        x = nullptr;
        break;
      }
    }
    if ( 0 != x && x->isVisible() ) { axes << x; }

    map::AxisSimple* y = l->yAxis();
    for ( int i = 0, isz = axes.size(); i < isz; ++i ) {
      if ( axes.at(i)->isEqual(*y) ) {
        y = nullptr;
        break;
      }
    }
    if ( 0 != y && y->isVisible() ) { axes << y; }

    y = l->yAxis2();
    for ( int i = 0, isz = axes.size(); i < isz; ++i ) {
      if ( axes.at(i)->isEqual(*y) ) {
        y = nullptr;
        break;
      }
    }
    if ( 0 != y && y->isVisible() ) { axes << y; }
  }

  QMutableListIterator<map::AxisSimple*> it(axes);
  while ( -1 != axisPosition && it.hasNext() ) {
    if ( it.next()->axisType() != axisPosition ) {
      it.remove();
    }
  }

  return axes;
}

QRect RamkaAxis::calcOuterRect(int left, int top, int right, int bottom) const
{
  if ( nullptr == document_ ) { return QRect(); }

  Projection* proj = document_->projection();
  if ( nullptr == proj ) { return QRect(); }

  const GeoVector& rmk = proj->ramka();
  QRect mapRect = document_->coord2screen(rmk).boundingRect();

  QRect docRect = document_->documentRect();
  docRect.setTopLeft(docRect.topLeft() + QPoint(left,top));
  docRect.setBottomRight(docRect.bottomRight() - QPoint(right,bottom));

  return docRect.intersected(mapRect);
}

QRect RamkaAxis::calcInnerRect() const
{
  QRect outer = calcOuterRect(0,0,0,0);
  if ( outer.isNull() ) {
    return QRect();
  }

  QPoint tl(outer.x(),                 outer.y());
  QPoint tr(outer.x() + outer.width(), outer.y());
  QPoint bl(outer.x(),                 outer.y() + outer.height());
  QPoint br(outer.x() + outer.width(), outer.y() + outer.height());

  int offset[4];
  // устанавливаем минимальный отступ в 1 px, чтобы был виден контур рамки,
  // если на стороне ни будет отрисована ни одна шкала
  offset[map::AxisSimple::kLeft]   = 1;
  offset[map::AxisSimple::kRight]  = 1;
  offset[map::AxisSimple::kTop]    = 1;
  offset[map::AxisSimple::kBottom] = 1;

  QRect rect;
  QPoint beg;
  QPoint end;
  QList<map::AxisSimple*> axes = visibleAxes();

  // hack
  foreach ( map::AxisSimple* a, axes ) {
    if ( a->type() != map::AxisPuanson::Type ) {
      continue;
    }
    GeoPoint gpData = document_->screen2coord(QPointF(0,0));
    a->updateData(gpData, document_);
  }

  foreach ( const map::AxisSimple* a, axes ) {
    if ( map::AxisSimple::kLeft == a->axisType() ) {
      beg = bl;
      end = tl;
      beg.rx() -= offset[a->axisType()];
      end.rx() -= offset[a->axisType()];
    }
    else if ( map::AxisSimple::kRight == a->axisType() ) {
      beg = br;
      end = tr;
      beg.rx() += offset[a->axisType()];
      end.rx() += offset[a->axisType()];
    }
    else {
      continue;
    }
    rect = a->boundingRect(document_);

    offset[a->axisType()] += rect.width();
  }
  foreach ( const map::AxisSimple* a, axes ) {
    if ( map::AxisSimple::kTop == a->axisType() ) {
      beg = outer.topLeft();
      end = outer.topRight();
      beg.ry() -= offset[a->axisType()];
      end.ry() -= offset[a->axisType()];
    }
    else if ( map::AxisSimple::kBottom == a->axisType() ) {
      beg = outer.bottomLeft();
      end = outer.bottomRight();
      beg.ry() += offset[a->axisType()];
      end.ry() += offset[a->axisType()];
    }
    else {
      continue;
    }
    rect = a->boundingRect(document_);

    offset[a->axisType()] += rect.height();
  }

  QRect inner = calcOuterRect(
        offset[map::AxisSimple::kLeft],
        offset[map::AxisSimple::kTop],
        offset[map::AxisSimple::kRight],
        offset[map::AxisSimple::kBottom]);

  return inner;
}

}
}
