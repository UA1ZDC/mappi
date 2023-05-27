
#include "ramkaspectr.h"
#include "layerspectr.h"
#include "gridspectr.h"
#include "axisspectr.h"

#include <qpainter.h>

#include <commons/geobasis/projection.h>

#include <meteo/commons/ui/map/document.h>
#include <meteo/commons/ui/map/geopolygon.h>
#include <meteo/commons/ui/map/layergrid.h>
#include <meteo/commons/ui/map/view/mapscene.h>



namespace meteo {

RamkaSpectr::RamkaSpectr(map::Document* doc)
  : map::Ramka(doc, 0)
{
  grid_ = new GridSpectr(document_);
}

RamkaSpectr::~RamkaSpectr()
{
  delete grid_;
}

bool RamkaSpectr::drawOnDocument(QPainter* painter)
{
  Projection* proj = document_->projection();
  const GeoVector& rmk = proj->ramka();
  QPolygon mappoly = document_->coord2screen(rmk);
  if ( 3 > mappoly.size() ) {
    return true;
  }

  // определяем какие оси должны быть отрисованы
  QList<const AxisSpectr*> axes;
  foreach ( map::Layer* layer, document_->layers()) {
    map::LayerSpectr* l = maplayer_cast<map::LayerSpectr*>(layer);

    if ( 0 == l || !l->visible() ) { continue; }

    const AxisSpectr* x = l->xAxis();
    for ( int i=0,isz=axes.size(); i<isz; ++i ) {
      if ( axes.at(i)->isEqual(*x) ) {
        x = 0;
        break;
      }
    }
    if ( 0 != x ) { axes << x; }

    const AxisSpectr* y = l->yAxis();
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
  foreach ( const AxisSpectr* a, axes ) {
    if ( AxisSpectr::kLeft == a->position() ) {
      int sz = cacheAxisSize_.value(a, 85);
      QPoint beg = r.bottomLeft();
      QPoint end = r.topLeft();
      beg.rx() -= lOffset;
      end.rx() -= lOffset;
      a->paint(painter, document_, beg, end);
      lOffset += sz;
    }
    else if ( AxisSpectr::kRight == a->position() ) {
      int sz = cacheAxisSize_.value(a, 85);
      QPoint beg = r.bottomRight();
      QPoint end = r.topRight();
      beg.rx() += rOffset;
      end.rx() += rOffset;
      a->paint(painter, document_, beg, end);
      rOffset += sz;
    }
    else if ( AxisSpectr::kBottom == a->position() ) {
      int sz = cacheAxisSize_.value(a, 30);
      QPoint beg = r.bottomLeft();
      QPoint end = r.bottomRight();
      beg.ry() += bOffset;
      end.ry() += bOffset;
      a->paint(painter, document_, beg, end);
      bOffset += sz;
    }
    else if ( AxisSpectr::kTop == a->position() ) {
      int sz = cacheAxisSize_.value(a, 30);
      QPoint beg = r.topLeft();
      QPoint end = r.topRight();
      beg.ry() -= tOffset;
      end.ry() -= tOffset;
      a->paint(painter, document_, beg, end);
      tOffset += sz;
    }
  }

  if ( nullptr != scene_ ) { scene_->muteEvents(); }
  updateGrid();
  grid_->setMapScene(scene_);
  if ( nullptr != scene_ ) { scene_->turnEvents(); }

  painter->restore();

  return true;
}

QPolygon RamkaSpectr::calcRamka(int /*mapindent*/, int /*docindent*/) const
{
  return QPolygon(calcRamkaRect(), true);
}

QList<const AxisSpectr*> RamkaSpectr::visibleAxes(int axisPosition) const
{
  if ( 0 == grid_ ) { return QList<const AxisSpectr*>(); }

  return grid_->visibleAxes(axisPosition);
}

QRect RamkaSpectr::calcRamkaRect() const
{
  // определяем какие оси должны быть отрисованы
  QList<const AxisSpectr*> axes = visibleAxes();

  foreach ( const AxisSpectr* a, axes ) {
    if ( !cacheAxisSize_.contains(a) ) {
      int sz = ( AxisSpectr::kLeft == a->position() || AxisSpectr::kRight == a->position() ) ? 85 : 30;
      cacheAxisSize_.insert(a, sz);
    }
  }

  int offset[4];
  offset[AxisSpectr::kLeft]  = 0;
  offset[AxisSpectr::kRight] = 0;
  offset[AxisSpectr::kTop]   = 0;
  offset[AxisSpectr::kBottom] = 0;
  foreach ( const AxisSpectr* a, axes ) {
    offset[a->position()] += cacheAxisSize_[a];
  }
  QRect r = calcRamkaRect(offset[AxisSpectr::kLeft],offset[AxisSpectr::kTop],offset[AxisSpectr::kRight],offset[AxisSpectr::kBottom]);
  QRect mainRect = r;

  offset[AxisSpectr::kLeft]  = 0;
  offset[AxisSpectr::kRight] = 0;
  offset[AxisSpectr::kTop]   = 0;
  offset[AxisSpectr::kBottom] = 0;

  QRect rect;
  QPoint beg;
  QPoint end;
  foreach ( const AxisSpectr* a, axes ) {
    if ( AxisSpectr::kLeft == a->position() ) {
      beg = r.bottomLeft();
      end = r.topLeft();
      beg.rx() -= offset[a->position()];
      end.rx() -= offset[a->position()];
    }
    else if ( AxisSpectr::kRight == a->position() ) {
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
  foreach ( const AxisSpectr* a, axes ) {
    QPoint beg;
    QPoint end;
    if ( AxisSpectr::kTop == a->position() ) {
      beg = r.topLeft();
      end = r.topRight();
      beg.ry() -= offset[a->position()];
      end.ry() -= offset[a->position()];
    }
    else if ( AxisSpectr::kBottom == a->position() ) {
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

  offset[AxisSpectr::kLeft]  = qAbs(mainRect.left() - r.left());
  offset[AxisSpectr::kRight] = qAbs(mainRect.right() - r.right());
  offset[AxisSpectr::kTop]   = qAbs(mainRect.top() - r.top());
  offset[AxisSpectr::kBottom]= qAbs(mainRect.bottom() - r.bottom());

  return calcRamkaRect(offset[AxisSpectr::kLeft],offset[AxisSpectr::kTop],offset[AxisSpectr::kRight],offset[AxisSpectr::kBottom]);;
}

QRect RamkaSpectr::calcRamkaRect(int left, int top, int right, int bottom) const
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

void RamkaSpectr::updateGrid()
{
  if ( 0 == grid_ ) { return; }

  grid_->calcGrid();
}

} // meteo
