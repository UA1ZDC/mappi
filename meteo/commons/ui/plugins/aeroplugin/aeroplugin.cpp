#include "aeroplugin.h"

#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>
#include <meteo/commons/global/log.h>
#include <meteo/commons/ui/mainwindow/mainwindow.h>
#include <meteo/commons/ui/mainwindow/widgethandler.h>
#include <meteo/commons/ui/aero/diagram/createaerodiagwidget.h>
#include <meteo/commons/ui/aero/diagram/aeroaction.h>


namespace meteo {
namespace map {

AeroPlugin::AeroPlugin()
  : MainWindowPlugin("aeroplugin")
{
  static const QString& createAeroDiag = QObject::tr("createaerodiag");
  QAction* action = mainwindow()->findAction(createAeroDiag);
  if ( nullptr == action ){
    warning_log << meteo::msglog::kTargetMenyNotFound.arg(createAeroDiag);
  }
  else {
    action->setDisabled(false);
    QObject::connect(action, &QAction::triggered, this, &AeroPlugin::slotOpenAeroDiag);
  }
}

AeroPlugin::~AeroPlugin()
{
}

void AeroPlugin::slotOpenAeroDiag() {

  meteo::map::proto::Document blank;
  blank.set_doctype( meteo::map::proto::kAeroDiagram );
  blank.set_projection(kGeneral);
  GeoPointPb *gp = blank.mutable_map_center();
  gp->set_lat_radian(-23);
  gp->set_lon_radian(245);
  gp->set_type(kLA_GENERAL);
  blank.mutable_doc_center()->CopyFrom(*gp);
  blank.set_punchorient(meteo::map::proto::kNoOrient);
  blank.set_scale(17);

  map::AeroDocument* doc = new map::AeroDocument(blank);
  doc->init();

  meteo::map::MapWindow* mapwindow = new meteo::map::MapWindow(mainwindow(), doc, MnCommon::etcPath("meteo") + "/tasks.menu.conf");
  meteo::adiag::CreateAeroDiagWidget* ad = new ::meteo::adiag::CreateAeroDiagWidget(mapwindow);
  if(nullptr != mapwindow->mapscene())
  {
    meteo::map::AeroAction* aeroact = new meteo::map::AeroAction(ad, mapwindow->mapscene());
    //mapwindow->mapscene()->addAction(aeroact, map::proto::kBottomLeft, QPoint(0,0));
    mapwindow->mapscene()->addAction(aeroact, kTopRight, QPoint(1,0));
  }
  mapwindow->document()->setBackgroundColor(QColor(255,255,255));
  mapwindow->addToolBarBreak();
  QToolBar* tb = mapwindow->addToolBar(QObject::tr("Свойства диаграммы"));
  tb->addWidget(ad);
  mainwindow()->toMdi(mapwindow);
  ad->show();
  //act->slotShow(true);

}

}
}
