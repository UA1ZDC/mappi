#include "sessionviewerwidget.h"
#include "ui_sessionviewerwidget.h"

#include <QGraphicsPixmapItem>
#include <QColor>
#include <QRgb>
#include <QSplitter>

#include <cross-commons/app/paths.h>

#include <meteo/commons/global/global.h>

#include <meteo/commons/ui/map/view/mapscene.h>
#include <meteo/commons/ui/map/view/mapview.h>
#include <meteo/commons/ui/map/view/actions/action.h>
#include <commons/textproto/tprototext.h>

#include <mappi/settings/mappisettings.h>
#include <mappi/proto/satellite.pb.h>
#include <mappi/ui/pos/posgrid.h>
#include <mappi/global/streamheader.h>
#include <mappi/proto/satelliteimage.pb.h>
#include <mappi/proto/sessiondataservice.pb.h>

#include <mappi/ui/satelliteimage/satelliteimage.h>
#include <mappi/ui/satelliteimage/georastr.h>

const QString tmpPath = QObject::tr("/tmp/.mappi");
const QString tmpImage = tmpPath + QObject::tr("/img%1.bmp");

const QString kGeoType = "geo.old";
//const QString kGeoType = "ptkpp";

namespace meteo {
namespace map {

SessionViewerWidget::SessionViewerWidget(QWidget *parent) :
  QWidget(parent),
  ui_(new Ui::SessionViewerWidget)
{
  ui_->setupUi(this);


  meteo::map::proto::Document blank = meteo::global::lastBlankParams(proto::kGeoMap);
  QString loaderName = "geo.old";
  if (blank.has_geoloader()) {
    loaderName = QString::fromStdString(blank.geoloader());
  }
  map_ = new MapWindow( nullptr, blank, loaderName, MnCommon::etcPath() + "/document.menu.conf/sessionmap.menu.conf" );


  //  map_ = new meteo::map::MapWindow(nullptr, initMap(), "geo.old", MnCommon::etcPath() + "/sessionmap.menu.conf");
  //map_ = new meteo::map::MapWindow(nullptr, initMap(), kGeoType, MnCommon::etcPath("meteo") + "/document.menu.conf/sessionmap.menu.conf");

  map_->document()->setMinScale(13);
  map_->addPluginGroup("mappi.map.viewer");
  map_->addPluginGroup("meteo.map.common");
  map_->addPluginGroup("meteo.map.weather");


  ui_->horizontalLayout->addWidget(map_);

  delimLayer_ = new meteo::map::Layer(map_->document(), "Береговая черта (верхний слой)");
  drawDelim();
  delimLayer_->setBase(true);

  //  delete map_->mapscene()->takeAction("scaleaction");
 // delete map_->mapscene()->takeAction("rotateaction");
  delete map_->mapscene()->takeAction("focusaction");
  //  delete map_->mapscene()->takeAction("contextmenueventaction");
  //  delete map_->mapscene()->takeAction("layersaction");
  delete map_->mapscene()->takeAction("savesxfacion");
  delete map_->mapscene()->takeAction("opensxfaction");
  //delete map_->mapscene()->takeAction("printdocaction");
  //  delete map_->mapscene()->takeAction("savebmpacion");
  //delete map_->mapscene()->takeAction("incutaction");
  //delete map_->mapscene()->takeAction("legendaction");
  delete map_->mapscene()->takeAction("hidebuttonsaction");
  delete map_->mapscene()->takeAction("informaction");
  //  delete map_->mapscene()->takeAction("mousecoordaction");

  for ( int i = 0, sz = map_->mapscene()->baseactions().size(); i < sz; ++i ) {
    meteo::map::Action* a = map_->mapscene()->baseactions().at(i);
    if ( a->name() == "coordsaction" ) {
      map_->mapscene()->removeAction(a);
      delete a;
      break;
    }
  }
}

SessionViewerWidget::~SessionViewerWidget()
{
  delete ui_;
  delete map_;
}

meteo::map::proto::Document SessionViewerWidget::initMap()
{
  auto reception = ::mappi::inter::Settings::instance()->reception();

  meteo::map::proto::Document blank;
  blank.set_projection(meteo::kStereo);
  blank.set_geoloader(kGeoType.toStdString());
  blank.mutable_map_center()->set_lat_radian(reception.site().point().lat_radian());
  blank.mutable_map_center()->set_lon_radian(reception.site().point().lon_radian());
  blank.mutable_map_center()->set_height_meters(reception.site().point().height_meters());
  blank.mutable_doc_center()->CopyFrom(blank.map_center());
  blank.set_scale(18);
  blank.set_smooth_iso(true);
  blank.set_cache(false);
  return blank;
}

void SessionViewerWidget::drawDelim()
{
  for ( int i = 0, sz = map_->document()->layers().size(); i < sz; ++i ) {
    meteo::map::Layer* layer = map_->document()->layers().at(i);
    //    var(layer->name());
    if ( "Береговая черта" == layer->name() || "ПОЛИТИЧЕСКАЯ КАРТА" == layer->name()) {
      delimLayer_->copyFrom(layer);
      break;
    }
  }
  //  for ( int i = 0, sz = delimLayer_->objects().size(); i < sz; ++i ) {
  // delimLayer_->objects().at(i)->setClosed(false);
  // }
  foreach(Object* obj, delimLayer_->objects()) {
    obj->setClosed(false);
  }
}

} //map
} //meteo
