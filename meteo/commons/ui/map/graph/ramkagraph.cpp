#include "ramkagraph.h"

#include <qpainter.h>

#include <commons/geobasis/projection.h>

#include <meteo/commons/ui/map/document.h>
#include <meteo/commons/ui/map/geopolygon.h>
#include <meteo/commons/ui/map/layergrid.h>
#include <meteo/commons/ui/map/view/mapscene.h>

#include "axisgraph.h"
#include "layergraph.h"
#include "gridgraph.h"

namespace meteo {

RamkaGraph::RamkaGraph(map::Document* doc)
  : map::Ramka(doc, 0)
{
  grid_ = new GridGraph(document_);
}

RamkaGraph::~RamkaGraph()
{
  delete grid_;
}

bool RamkaGraph::drawOnDocument(QPainter* painter)
{
  Projection* proj = document_->projection();
  const GeoVector& rmk = proj->ramka();
  QPolygon mappoly = document_->coord2screen(rmk);
  if ( 3 > mappoly.size() ) {
    return true;
  }

  // определяем какие оси должны быть отрисованы
  QList<const AxisGraph*> axes;
  foreach ( map::Layer* layer, document_->layers()) {
    map::LayerGraph* l = maplayer_cast<map::LayerGraph*>(layer);

    if ( 0 == l || !l->visible() ) { continue; }

    const AxisGraph* x = l->xAxis();
    for ( int i=0,isz=axes.size(); i<isz; ++i ) {
      if ( axes.at(i)->isEqual(*x) ) {
        x = 0;
        break;
      }
    }
    if ( 0 != x ) { axes << x; }

    const AxisGraph* y = l->yAxis();
    for ( int i=0,isz=axes.size(); i<isz; ++i ) {
      if ( axes.at(i)->isEqual(*y) ) {
        y = 0;
        break;
      }
    }
    if ( 0 != y ) { axes << y; }
  }

  QRect r = calcRamkaRect();

  QPen pen;
  pen.setWidth(1);
  pen.setColor( Qt::gray );

  painter->save();
  painter->setPen(pen);
  QRect axesRect = r;
  r.setRight(r.right() + 1);
  r.setBottom(r.bottom() + 1);
  painter->drawRect(axesRect);

  pen.setWidth(1);
  pen.setColor( Qt::black );
  painter->setPen(pen);

  int lOffset = 0;
  int rOffset = 0;
  int tOffset = 0;
  int bOffset = 0;
  foreach ( const AxisGraph* a, axes ) {
    if ( AxisGraph::kLeft == a->position() ) {
      int sz = cacheAxisSize_.value(a, 85);
      QPoint beg = r.bottomLeft();
      QPoint end = r.topLeft();
      beg.rx() -= lOffset;
      end.rx() -= lOffset;
      a->paint(painter, document_, beg, end);
      lOffset += sz;
    }
    else if ( AxisGraph::kRight == a->position() ) {
      int sz = cacheAxisSize_.value(a, 85);
      QPoint beg = r.bottomRight();
      QPoint end = r.topRight();
      beg.rx() += rOffset;
      end.rx() += rOffset;
      a->paint(painter, document_, beg, end);
      rOffset += sz;
    }
    else if ( AxisGraph::kBottom == a->position() ) {
      int sz = cacheAxisSize_.value(a, 30);
      QPoint beg = r.bottomLeft();
      QPoint end = r.bottomRight();
      beg.ry() += bOffset;
      end.ry() += bOffset;
      a->paint(painter, document_, beg, end);
      bOffset += sz;
    }
    else if ( AxisGraph::kTop == a->position() ) {
      int sz = cacheAxisSize_.value(a, 30);
      QPoint beg = r.topLeft();
      QPoint end = r.topRight();
      beg.ry() -= tOffset;
      end.ry() -= tOffset;
      a->paint(painter, document_, beg, end);
      tOffset += sz;
    }
  }

  painter->restore();

  updateGrid();

  return true;
}

QPolygon RamkaGraph::calcRamka(int /*mapindent*/, int /*docindent*/) const
{
  return QPolygon(calcRamkaRect(), true);
}

QList<const AxisGraph*> RamkaGraph::visibleAxes(int axisPosition) const
{
  if ( 0 == grid_ ) { return QList<const AxisGraph*>(); }

  return grid_->visibleAxes(axisPosition);
}

QRect RamkaGraph::calcRamkaRect() const
{
  // определяем какие оси должны быть отрисованы
  QList<const AxisGraph*> axes = visibleAxes();

  foreach ( const AxisGraph* a, axes ) {
    if ( !cacheAxisSize_.contains(a) ) {
      int sz = ( AxisGraph::kLeft == a->position() || AxisGraph::kRight == a->position() ) ? 85 : 30;
      cacheAxisSize_.insert(a, sz);
    }
  }

  int offset[4];
  offset[AxisGraph::kLeft]  = 0;
  offset[AxisGraph::kRight] = 0;
  offset[AxisGraph::kTop]   = 0;
  offset[AxisGraph::kBottom] = 0;
  foreach ( const AxisGraph* a, axes ) {
    offset[a->position()] += cacheAxisSize_[a];
  }

  QRect r = calcRamkaRect(offset[AxisGraph::kLeft],offset[AxisGraph::kTop],offset[AxisGraph::kRight],offset[AxisGraph::kBottom]);

  QRect mainRect = r;

  offset[AxisGraph::kLeft]  = 0;
  offset[AxisGraph::kRight] = 0;
  offset[AxisGraph::kTop]   = 0;
  offset[AxisGraph::kBottom] = 0;

  QRect rect;
  QPoint beg;
  QPoint end;
  foreach ( const AxisGraph* a, axes ) {
    if ( AxisGraph::kLeft == a->position() ) {
      beg = r.bottomLeft();
      end = r.topLeft();
      beg.rx() -= offset[a->position()];
      end.rx() -= offset[a->position()];
    }
    else if ( AxisGraph::kRight == a->position() ) {
      beg = r.bottomRight();
      end = r.topRight();
      beg.rx() += offset[a->position()];
      end.rx() += offset[a->position()];
    }
    else {
      continue;
    }
    rect = a->geometry(document_, beg, end);
    mainRect = mainRect.united(rect);

    offset[a->position()] += rect.size().width();
    cacheAxisSize_[a] = rect.size().width();
  }
  foreach ( const AxisGraph* a, axes ) {
    QPoint beg;
    QPoint end;
    if ( AxisGraph::kTop == a->position() ) {
      beg = r.topLeft();
      end = r.topRight();
      beg.ry() -= offset[a->position()];
      end.ry() -= offset[a->position()];
    }
    else if ( AxisGraph::kBottom == a->position() ) {
      beg = r.bottomLeft();
      end = r.bottomRight();
      beg.ry() += offset[a->position()];
      end.ry() += offset[a->position()];
    }
    else {
      continue;
    }
    QRect rect = a->geometry(document_, beg, end);
    mainRect = mainRect.united(rect);

    offset[a->position()] += rect.size().height();
    cacheAxisSize_[a] = rect.size().height();
  }

  offset[AxisGraph::kLeft]  = qAbs(mainRect.left() - r.left());
  offset[AxisGraph::kRight] = qAbs(mainRect.right() - r.right());
  offset[AxisGraph::kTop]   = qAbs(mainRect.top() - r.top());
  offset[AxisGraph::kBottom]= qAbs(mainRect.bottom() - r.bottom());

  return calcRamkaRect(offset[AxisGraph::kLeft],offset[AxisGraph::kTop],offset[AxisGraph::kRight],offset[AxisGraph::kBottom]);;
}

QRect RamkaGraph::calcRamkaRect(int left, int top, int right, int bottom) const
{
  if ( 0 == document_ ) { return QRect(); }

  Projection* proj = document_->projection();
  const GeoVector& rmk = proj->ramka();
  QRect mapRect = document_->coord2screen(rmk).boundingRect();

  QRect docRect = document_->documentRect();
  docRect.setTopLeft(docRect.topLeft() + QPoint(left,top));
  docRect.setBottomRight(docRect.bottomRight() - QPoint(right,bottom));

  return docRect.intersected(mapRect);
}

void RamkaGraph::updateGrid()
{
  if ( 0 == grid_ ) { return; }

  grid_->updateGrid();
}

} // meteo
