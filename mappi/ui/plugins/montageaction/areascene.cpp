#include "areascene.h"
#include <mappi/settings/mappisettings.h>

#include <qpainter.h>
#include <qgraphicsitem.h>
#include <qstyleoption.h>

#include <meteo/commons/ui/map/document.h>
#include <meteo/commons/ui/map/loader.h>
#include <meteo/commons/ui/map/layer.h>
#include <meteo/commons/ui/map/ramka.h>
#include <meteo/commons/ui/map/geopolygon.h>
#include <meteo/commons/ui/map/geotext.h>
#include <meteo/commons/ui/map/geopixmap.h>
#include <meteo/commons/ui/map/isoline.h>

#include <mappi/schedule/session.h>

namespace meteo {
namespace map{

AreaScene::AreaScene(QGraphicsView* parent):
  QGraphicsScene(parent)
  , document_(new meteo::map::Document())
  , layer_(new meteo::map::SatLayer(document_))
  , grid_(new PosGrid)
{
  const auto reception = ::mappi::inter::Settings::instance()->reception();
  center_.set_lat_radian(reception.site().point().lat_radian());
  center_.set_lon_radian(reception.site().point().lon_radian());
  center_.set_lat_deg(MnMath::rad2deg(reception.site().point().lat_radian()));
  center_.set_lon_deg(MnMath::rad2deg(reception.site().point().lon_radian()));
  center_.set_height_meters(reception.site().point().height_meters());
  if ( false == setupDoc()) {
    error_log << "Не удалось создать документ карты";
  }
}

AreaScene::~AreaScene()
{
  delete document_;
}

void AreaScene::setSession(const ::mappi::schedule::Session &session, const ::mappi::conf::Instrument &instrument, const MnSat::TLEParams& tle)
{
  nadirAngle_ = instrument.has_scan_angle() ? instrument.scan_angle() : 55.37; //TODO так не корректно делать
    
  //debug_log << session.data().aos << session.data().los << instrument.scan_angle() <<  instrument.samples() << instrument.velocity();
  
  grid_->setSatellite(session.data().aos, session.data().los, tle, instrument.scan_angle()*meteo::DEG2RAD, instrument.samples(), instrument.velocity());
  //раскомментировать следующую строчку, чтобы сдвигать центр карты на центр проекции
  document_->setScreenCenter(grid_->getMapCenter());
  
  satelliteAreaCoord_.clear();

  if(nullptr != grid_->satViewPoint() && grid_->satViewPoint()->countBorder(1000, meteo::DEG2RAD * nadirAngle_, &satelliteAreaCoord_)) {
    drawSatelliteArea();
  }

//  createSatelliteInformation(session, tle);
  redraw();
}

void AreaScene::resize(const QSize &size)
{
  setSceneRect(QRect(QPoint(0, 0), size));
  resizeDocument(size);
}

void AreaScene::drawBackground(QPainter *painter, const QRectF &rect)
{
  QGraphicsScene::drawBackground(painter, rect);
  document_->drawDocument(painter);
}

void AreaScene::drawItems(QPainter *painter, int numItems, QGraphicsItem *items[], const QStyleOptionGraphicsItem options[], QWidget *widget)
{
  for(int i = 0; i < numItems; ++i)
  {
    painter->save();
    painter->setMatrix(items[i]->sceneMatrix(), true);
    items[i]->paint(painter, &options[i], widget);
    painter->restore();
  }
}

void AreaScene::drawForeground(QPainter *painter, const QRectF &rect)
{
  QGraphicsScene::drawForeground(painter, rect);
  document_->drawRamka(painter);
}

bool AreaScene::setupDoc()
{
  meteo::map::proto::Document doc;
  doc.mutable_mapsize()->CopyFrom(meteo::qsize2size(QSize(2000, 2000)));
  doc.mutable_docsize()->CopyFrom(meteo::qsize2size(sceneRect().size().toSize()));
  doc.set_geoloader("geo.old");
//  doc.set_cache(true);
  doc.set_scale(18);
  doc.mutable_map_center()->CopyFrom(center_);
  doc.mutable_doc_center()->CopyFrom(center_);
  doc.set_projection(meteo::kStereo);
  if(false == document_->init(doc)) {
    error_log << QObject::tr("Не удалось создать документ с параметрами = %1")
                 .arg(QString::fromStdString(doc.Utf8DebugString()));
    return false;
  }
  document_->setScreenCenter(meteo::pbgeopoint2geopoint(center_));
//  layer_ = new meteo::map::Layer(document_);
  layer_ = new meteo::map::SatLayer(document_);
  layer_->setProjection(grid_);

  meteo::map::Loader::instance()->handleGrid("grid.common", document_);
  QGraphicsScene::setBackgroundBrush(QBrush(meteo::kMAP_ISLAND_COLOR));
  return true;
}

void AreaScene::resizeDocument(const QSize &size)
{
  document_->resizeDocument(size);
  document_->resizeMap(size);
  document_->setScreenCenter(document_->screenCenter());
  redraw();
}

void AreaScene::redraw()
{
  drawSatelliteArea();
  invalidate();
}

void AreaScene::drawSatelliteArea()
{
  if(satelliteAreaCoord_.isEmpty()) return;

  meteo::map::GeoPolygon* area = mapobject_cast<meteo::map::GeoPolygon*>(layer_->objectByUuid(satelliteAreaUuid_));
  if(nullptr == area) {
    area = new meteo::map::GeoPolygon(layer_);
    satelliteAreaUuid_ = area->uuid();
    area->setPen(meteo::qpen2pen(QPen(QBrush(Qt::darkRed), 2.0)));
    area->setClosed(true);
  }
  area->setSkelet(satelliteAreaCoord_);
  area->setVisible(true);
}

}
}

