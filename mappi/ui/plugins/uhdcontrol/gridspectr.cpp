#include "gridspectr.h"
#include "layerspectr.h"
#include "axisspectr.h"

#include <meteo/commons/ui/map/document.h>
#include <meteo/commons/ui/map/geopolygon.h>
#include <meteo/commons/ui/map/layergrid.h>
#include <meteo/commons/global/global.h>

namespace meteo {

GridSpectr::GridSpectr(map::Document* doc, QObject *parent)
  : QObject(parent),
    document_(doc)
{
  if ( 0 == document_ ) { return; }

  if ( 0 == document_->gridLayer() ) {
    map::LayerGrid* layer = new map::LayerGrid(document_);
    layer->setName(tr("Координатная сетка"));
    layerUuid_ = layer->uuid();
  }

  foreach ( map::Layer* layer, document_->layers() ) {
    map::LayerSpectr* l = maplayer_cast<map::LayerSpectr*>(layer);
    if ( 0 != l ) {
      layerAxes_[l->xAxis()] = l->uuid();
      layerAxes_[l->yAxis()] = l->uuid();
    }
  }
  calcGrid();

  if ( !document_->isStub() ) {
    document_->turnEvents();
    if ( document_->eventHandler() != 0 ) {
      document_->eventHandler()->installEventFilter(this);
    }
  }
}

QList<const AxisSpectr*> GridSpectr::visibleAxes(int axisPosition) const
{
  QList<const AxisSpectr*> axes;
  foreach (map::Layer* layer, document_->layers()) {
    map::LayerSpectr* l = maplayer_cast<map::LayerSpectr*>(layer);

    if ( 0 == l || !l->visible() ) { continue; }

    const AxisSpectr* x = l->xAxis();
    x->updateDocScale(document_->scale());
    if ( nullptr != scene_) {
      x->setScale(scene_->scale());
      x->calcTicks();
    }
    for ( int i=0,isz=axes.size(); i<isz; ++i ) {
      if ( !x->property().visible() || axes.at(i)->isEqual(*x) ) {
        x = 0;
        break;
      }
    }
    if ( 0 != x ) { axes << x; }

    const AxisSpectr* y = l->yAxis();
    y->updateDocScale(document_->scale());
    if ( nullptr != scene_) {
      y->setScale(scene_->scale());
      y->calcTicks();
    }
    for ( int i=0,isz=axes.size(); i<isz; ++i ) {
      if ( !y->property().visible() || axes.at(i)->isEqual(*y) ) {
        y = 0;
        break;
      }
    }
    if ( 0 != y ) { axes << y; }
  }

  for ( int i=0,isz=axes.size(); i<isz && -1 != axisPosition; ++i ) {
    if ( axes.at(i)->position() != axisPosition ) {
      axes.removeAt(i);
      --i;
      --isz;
    }
  }

  return axes;
}

void GridSpectr::calcGrid()
{
  foreach ( const AxisSpectr* a, gridLines_.keys() ) {
    foreach ( int lvl, gridLines_[a].keys() ) {
      qDeleteAll(gridLines_[a][lvl]);
    }
  }
  gridLines_.clear();

  map::LayerGrid* layer = document_->gridLayer();
  if ( 0 == layer ) {
    return;
  }

  if ( layer->uuid() != layerUuid_ ) {
    return;
  }

  meteo::Property prop;
  prop.mutable_pen()->set_color(qRgba(210,210,210,255));
  prop.mutable_pen()->set_style(meteo::kDotLine);
  prop.mutable_pen()->set_width(1);
  prop.set_visible(false);

  GeoPoint tlMap = document_->projection()->start();
  GeoPoint brMap = document_->projection()->end();

  GeoPoint beg(tlMap.lat(), tlMap.lon(), 0, LA_GENERAL);
  GeoPoint end(tlMap.lat(), brMap.lon(), 0, LA_GENERAL);
  GeoVector skelet;
  skelet << beg << end;

  QList<const AxisSpectr*> hAxes = visibleAxes(AxisSpectr::kBottom) + visibleAxes(AxisSpectr::kTop);
  foreach ( const AxisSpectr* a, hAxes ) {
    if ( !gridLines_.contains(a) ) {
      QList<meteo::Generalization> generals = a->dataLevels();
      foreach ( const meteo::Generalization& g, generals ) {
        QList<map::Object*> list;
        QVector<double> ticks = a->ticks(g.highLimit());
        foreach ( double tick, ticks ) {
          skelet[0].setLat(tick);
          skelet[1].setLat(tick);
          map::GeoPolygon* line = new map::GeoPolygon(layer);
          line->setSkelet(skelet);
          line->setProperty(prop);
          line->setGeneral(g);
          list << line;
        }
        gridLines_[a].insert(g.highLimit(), list);
      }
    }
  }

  beg = GeoPoint(tlMap.lat(), brMap.lon(), 0, LA_GENERAL);
  end = GeoPoint(brMap.lat(), brMap.lon(), 0, LA_GENERAL);
  skelet.clear();
  skelet << beg << end;
  QList<const AxisSpectr*> vAxes = visibleAxes(AxisSpectr::kRight) + visibleAxes(AxisSpectr::kLeft);
  foreach ( const AxisSpectr* a, vAxes ) {
    if ( !gridLines_.contains(a) ) {
      QList<meteo::Generalization> generals = a->dataLevels();
      foreach ( const meteo::Generalization& g, generals ) {
        QList<map::Object*> list;
        QVector<double> ticks = a->ticks(g.highLimit());
        foreach ( double tick, ticks ) {
          skelet[0].setLon(tick);
          skelet[1].setLon(tick);
          map::GeoPolygon* line = new map::GeoPolygon(layer);

          line->setSkelet(skelet);
          line->setProperty(prop);
          line->setGeneral(g);
          list << line;
        }
        gridLines_[a].insert(g.highLimit(), list);
      }
    }
  }

  QList<const AxisSpectr*> allAxes;
  foreach (map::Layer* layer, document_->layers()) {
    map::LayerSpectr* l = maplayer_cast<map::LayerSpectr*>(layer);
    if ( 0 == l ) { continue; }
    allAxes << l->xAxis() << l->yAxis();
  }

  foreach ( const AxisSpectr* a, gridLines_.keys() ) {
    if ( !allAxes.contains(a) ) {
      gridLines_.remove(a);
      continue;
    }

    foreach ( const meteo::Generalization& g, a->dataLevels() ) {
      QList<map::Object*> lines = gridLines_.value(a).value(g.highLimit());
      for ( int i=0,isz=lines.size(); i<isz; ++i ) {
        lines.at(i)->setVisible(false);
      }
    }
  }

  updateGrid();
}

void GridSpectr::updateGrid()
{
  if ( 0 == document_ ) { return; }

  QList<const AxisSpectr*> leftAxes = visibleAxes(AxisSpectr::kLeft);
  QList<const AxisSpectr*> rightAxes = visibleAxes(AxisSpectr::kRight);
  QList<const AxisSpectr*> topAxes = visibleAxes(AxisSpectr::kTop);
  QList<const AxisSpectr*> bottomAxes = visibleAxes(AxisSpectr::kBottom);

  QList<const AxisSpectr*> gridAxes;

  if ( leftAxes.size() != 0 ) {
    gridAxes << leftAxes.first();
  }
  else if ( rightAxes.size() != 0 ) {
    gridAxes << rightAxes.first();
  }

  if ( bottomAxes.size() != 0 ) {
    gridAxes << bottomAxes.first();
  }
  else if ( topAxes.size() != 0 ) {
    gridAxes << topAxes.first();
  }

  foreach ( const AxisSpectr* a, gridAxes ) {
    QList<map::Object*> lines = gridLines_[a][a->level()];
    QBitArray mask = a->lastDrawed();

    if ( lines.size() != mask.size() ) {
      debug_log << QObject::tr("Ошибка: количество тиков не соответствует количеству линий сетки.");
      var(lines.size()) << var(mask.size()) << var(a->level());
      continue;
    }

    for ( int i=0,isz=lines.size(); i<isz; ++i ) {
      lines.at(i)->setVisible(mask.testBit(i));
    }
  }

}

bool GridSpectr::eventFilter(QObject* obj, QEvent* event)
{
  Q_UNUSED( obj );

  if ( 0 == document_ ) {
    return false;
  }
  if ( map::LayerEvent::LayerChanged != event->type() ) {
    return false;
  }

  map::LayerEvent* ev = static_cast<map::LayerEvent*>(event);
  if ( false == document_->hasLayer(ev->layer()) && map::LayerEvent::Deleted != ev->changeType() ) {
//    debug_log << QObject::tr("Ошибка. Слой %1 не найден. Событие %2").arg(ev->layer()).arg(ev->changeType());
    return false;
  }

  switch ( ev->changeType() ) {
    case map::LayerEvent::Moved:
//      trc << "MOVED";
      break;
    case map::LayerEvent::Visibility:
//      trc << "VISIBILITY";
      break;
    case map::LayerEvent::Added:
    {
//      trc << "ADDED";
      map::LayerSpectr* l = maplayer_cast<map::LayerSpectr*>(document_->layerByUuid(ev->layer()));
      if ( 0 != l ) {
        layerAxes_[l->xAxis()] = l->uuid();
        layerAxes_[l->yAxis()] = l->uuid();
      }
    } break;
    case map::LayerEvent::Deleted:
//      trc << "DELETED";
      if ( ev->layer() == layerUuid_ ) {
        layerUuid_.clear();
        gridLines_.clear();
      }

      foreach ( const AxisSpectr* a, layerAxes_.keys(ev->layer()) ) {
        foreach ( int lvl, gridLines_[a].keys() ) {
          qDeleteAll(gridLines_[a][lvl]);
        }
        gridLines_.remove(a);

        layerAxes_.remove(a);
      }
      break;
    case map::LayerEvent::ObjectChanged:
//      trc << "OBJECT CHANGED";
      break;
    case map::LayerEvent::Activity:
//      trc << "ACTIVITY";
      break;
    default:
      break;
  }

  return false;
}

} // meteo
