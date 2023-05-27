#include "uhdcontrolplugin.h"
#include "uhdcontrol.h"
#include "layerspectr.h"
#include "ramkaspectr.h"
#include "axisspectr.h"
#include "funcs.h"
#include "uhdwindow.h"

#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>

#include <qapplication.h>
#include <QDockWidget>
#include <QAction>


#include <cross-commons/app/paths.h>
#include <mappi/ui/plugins/uhdcontrol/coordaction/spectrcoordaction.h>
#include <mappi/ui/plugins/uhdcontrol/valueaction/spectrvalueaction.h>
#include <commons/geobasis/generalproj.h>
#include <commons/geobasis/projection.h>
#include <meteo/commons/ui/map/view/mapscene.h>
#include <meteo/commons/ui/mainwindow/mainwindow.h>
#include <meteo/commons/ui/mainwindow/mdisubwindow.h>
#include <meteo/commons/ui/map/view/mapwindow.h>
#include <meteo/commons/proto/map_document.pb.h>
#include <meteo/commons/ui/map/document.h>


const QString kRootPath = MnCommon::projectPath();

UhdControlPlugin::UhdControlPlugin()
  : meteo::app::MainWindowPlugin("uhdcontrol"),
  mapWindow_(NULL)
{
  if ( 0 == mainwindow() ) {
    return;
  }

  // QAction* a = 0;
  // a = mainwindow()->actionFromFile(MnCommon::etcPath() + "/application.menu.conf", "uhdcontrol");
  static const QString& actionName = QObject::tr("uhdcontrol");
  QAction* a = mainwindow()->findAction(actionName);
  connect( a, SIGNAL( triggered() ), this, SLOT( slotCreateWindow() ) );
}

UhdControlPlugin::~UhdControlPlugin()
{
  delete mapWindow_;
  mapWindow_ = 0;
}

UhdWindow *UhdControlPlugin::slotCreateWindow()
{
  qApp->setOverrideCursor(Qt::WaitCursor);
  if (NULL == mapWindow_) {
    mapWindow_ = getMapWindow();
    QDockWidget* docWidget = new QDockWidget();
    docWidget->setMinimumWidth(325);
    mapWindow_->addDockWidget(Qt::LeftDockWidgetArea,docWidget);
    connect(mapWindow_, SIGNAL(destroyed(QObject*)), SLOT(slotWidgetDeleted()));
    if ( 0 != window_ ) {
      window_->toMdi(mapWindow_);
    }
    UhdControl* widget= new UhdControl(mapWindow_);
    widget->init();
    meteo::map::SpectrValueAction* grCoordAct2 = new meteo::map::SpectrValueAction(mapWindow_->mapscene());
    mapWindow_->mapscene()->addAction(grCoordAct2, meteo::kBottomLeft, QPoint(0,0));
    docWidget->setWidget(widget);
    docWidget->setFeatures(QDockWidget::NoDockWidgetFeatures);
  }
  mapWindow_->show();
  mapWindow_->setFocus();
  qApp->restoreOverrideCursor();
  return mapWindow_;
}

UhdWindow* UhdControlPlugin::getMapWindow()
{
  meteo::map::proto::Document blank;
  blank.set_doctype( meteo::map::proto::kMeteogram );
  blank.set_projection(meteo::kGeneral);
  meteo::GeoPointPb* gp = blank.mutable_map_center();
  gp->set_lat_radian(117.5);
  gp->set_lon_radian(-50);
  gp->set_type( meteo::kLA_GENERAL );
  gp = blank.mutable_doc_center();
  gp->set_lat_radian(117.5);
  gp->set_lon_radian(-50);
  gp->set_type( meteo::kLA_GENERAL );
  blank.set_scale(9);
  blank.set_smooth_iso(true);
  UhdWindow* mapWindow = new UhdWindow(0, blank );
  meteo::map::Document* document = mapWindow->document();
  meteo::GeneralProj* proj = meteo::projection_cast<meteo::GeneralProj*>( document->projection());
  if ( 0 != proj ) {
    proj->setFuncTransform(meteo::graphLinearF2X_one, meteo::graphLinearX2F_one);
    proj->setFuncTransform(meteo::graphLinearF2X_onef, meteo::graphLinearX2F_onef);
    proj->setRamka(0,-100,500,0);
    proj->setStart(meteo::GeoPoint(0,0,0,meteo::LA_GENERAL));
    proj->setEnd(meteo::GeoPoint(500,-100,0,meteo::LA_GENERAL));
//    proj->setXfactor(15*1);
    proj->setYfactor(1.0/15.0);
  }
  meteo::RamkaSpectr* ramka = new meteo::RamkaSpectr(document);
  ramka->setMapScene(mapWindow->mapscene());
  document->setRamka(ramka);
  document->setBackgroundColor(QColor(255,255,255));
  document->setScreenCenter(120,-50);
  mapWindow->setWindowTitle("Тест аппаратуры приёма");
  return mapWindow;
}

void UhdControlPlugin::slotWidgetDeleted()
{
  mapWindow_ = NULL;
}


