#include "layeraxis.h"

#include <quuid.h>
#include <qpainter.h>

#include <cross-commons/debug/tlog.h>

#include <commons/geobasis/generalproj.h>
#include <commons/geom/geom.h>

#include <meteo/commons/ui/map/document.h>
#include <meteo/commons/ui/map/axis/axissimple.h>

namespace meteo {
namespace map {

namespace {
  Layer* createLayer( Document* d )
  {
    return new LayerAxis(d);
  }
  static const bool res = singleton::PtkppFormat::instance()->registerLayerHandler( LayerAxis::Type, createLayer );
}

LayerAxis::LayerAxis(Document* d, const QString& name, int customtype)
  : Layer(d,name,customtype)
{
  setName(name);

  if ( nullptr != document_ ) {
      Projection* proj = document_->projection();
      if(nullptr != proj ){

      const GeoVector& rmk = proj->ramka();

      QPolygonF poly;
      for ( int i = 0, isz = rmk.size(); i < isz; ++i ) {
        poly << QPointF(rmk[i].lat(),rmk[i].lon());
      }
      mapRect_ = poly.boundingRect().toRect();
    }
  }
  x1_ = new AxisSimple();
  x2_ = new AxisSimple();
  y1_ = new AxisSimple();
  y2_ = new AxisSimple();

  x1_->setAxisType(AxisSimple::kBottom);
  x2_->setAxisType(AxisSimple::kTop);
  y1_->setAxisType(AxisSimple::kLeft);
  y2_->setAxisType(AxisSimple::kRight);

  x1_->setLayer(this);
  x2_->setLayer(this);
  y1_->setLayer(this);
  y2_->setLayer(this);

  x1_->setVisible(true);
  x2_->setVisible(false);
  y1_->setVisible(true);
  y2_->setVisible(false);

  setupAxis(x1_);
  setupAxis(x2_);
  setupAxis(y1_);
  setupAxis(y2_);
}

LayerAxis::~LayerAxis()
{
  if ( nullptr != x1_ ) { delete x1_; x1_ = nullptr; }
  if ( nullptr != x2_ ) { delete x2_; x2_ = nullptr; }
  if ( nullptr != y1_ ) { delete y1_; y1_ = nullptr; }
  if ( nullptr != y2_ ) { delete y2_; y2_ = nullptr; }
}

void LayerAxis::setAxisX(AxisSimple* axis)
{
  delete x1_;
  x1_ = axis;
  setupAxis(x1_);
}

void LayerAxis::setAxisY(AxisSimple* axis)
{
  delete y1_;
  y1_ = axis;
  setupAxis(y1_);
}

void LayerAxis::setAxisX2(AxisSimple* axis)
{
  delete x2_;
  x2_ = axis;
  setupAxis(x2_);
}

void LayerAxis::setAxisY2(AxisSimple* axis)
{
  delete y2_;
  y2_ = axis;
  setupAxis(y2_);
}

void LayerAxis::setupAxis(AxisSimple* axis)
{
  if ( nullptr == axis ) {
    return;
  }

  axis->setDrawOnRamka(true);
  axis->setMapSize(mapRect_.size());

  GeoPoint tl(mapRect_.x(),                    mapRect_.y(),                     0, LA_GENERAL);
  GeoPoint bl(mapRect_.x(),                    mapRect_.y() + mapRect_.height(), 0, LA_GENERAL);
  GeoPoint br(mapRect_.x() + mapRect_.width(), mapRect_.y() + mapRect_.height(), 0, LA_GENERAL);

  switch ( axis->axisType() ) {
    case AxisSimple::kLeft:
      axis->setGeoPos(bl);
      axis->setOffset(mapRect_.top());
      axis->setLength(mapRect_.height());
      break;
    case AxisSimple::kRight:
      axis->setGeoPos(br);
      axis->setOffset(mapRect_.top());
      axis->setLength(mapRect_.height());
      break;
    case map::AxisSimple::kCenter:
      not_impl;
      break;
    case AxisSimple::kTop:
      axis->setGeoPos(tl);
      axis->setOffset(mapRect_.left());
      axis->setLength(mapRect_.width());
      break;
    case AxisSimple::kBottom:
      axis->setGeoPos(bl);
      axis->setOffset(mapRect_.left());
      axis->setLength(mapRect_.width());
      break;
    case map::AxisSimple::kMiddle:
      not_impl;
      break;
  }
}

}
}
