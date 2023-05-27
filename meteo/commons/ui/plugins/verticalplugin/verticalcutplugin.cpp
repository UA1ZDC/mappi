#include "verticalcutplugin.h"

#include <cross-commons/app/paths.h>
#include <cross-commons/debug/tlog.h>
#include <meteo/commons/global/log.h>
#include <meteo/commons/ui/mainwindow/mainwindow.h>
#include <meteo/commons/ui/mainwindow/widgethandler.h>
#include <meteo/commons/ui/aero/verticalcut/createcutwidget.h>
#include <meteo/commons/ui/map/ramka.h>
#include <meteo/commons/ui/mainwindow/mdisubwindow.h>
#include <meteo/commons/ui/plugins/axisaction/axisaction.h>
#include <meteo/commons/ui/plugins/axisaction/pointaction.h>

#include <qobject.h>

#include "createcutaction.h"

namespace meteo {
namespace map {

VerticalCutPlugin::VerticalCutPlugin()
  : MainWindowPlugin("verticalcutplugin")
{
  static const QString& createcut = QObject::tr("createcut");
  QAction* action = mainwindow()->findAction(createcut);
  if ( nullptr == action ){
    warning_log.msgBox() << meteo::msglog::kTargetMenyNotFound.arg(createcut);
  }
  else {
    action->setDisabled(false);
    QObject::connect(action, &QAction::triggered, this, &VerticalCutPlugin::slotOpenCut);
  }
}


VerticalCutPlugin::~VerticalCutPlugin()
{
}

void VerticalCutPlugin::slotOpenCut()
{
  map::proto::Document blank;
  blank.set_doctype( meteo::map::proto::kVerticalCut );
  blank.set_projection(kGeneral);
  GeoPointPb* gp = blank.mutable_map_center();
  gp->set_lat_radian(VProfileDoc::kBlankWidth/2 + 100);
  gp->set_lon_radian(750);
  gp->set_type( kLA_GENERAL );
  blank.mutable_doc_center()->CopyFrom(*gp);
  blank.set_punchorient( map::proto::kNoOrient );
  blank.set_scale(20);
  blank.set_smooth_iso(true);
  blank.set_smooth_iso(true);

  VProfileDoc* vdoc = new VProfileDoc(blank);
  vdoc->init();

  meteo::map::MapWindow* mapwindow = new meteo::map::MapWindow(mainwindow(), vdoc, MnCommon::etcPath("meteo") + "/tasks.menu.conf");
  meteo::CreateCutWidget* cut = new meteo::CreateCutWidget(mapwindow);

  meteo::map::AxisAction* axisAct = new meteo::map::AxisAction(mapwindow->mapscene());
  mapwindow->mapscene()->addAction(axisAct, kBottomLeft, QPoint(0,0));

  meteo::map::PointAction* pointAct = new meteo::map::PointAction(mapwindow->mapscene());
  mapwindow->mapscene()->addAction(pointAct, kBottomLeft, QPoint(0,0));

  mapwindow->document()->setBackgroundColor(QColor(255,255,255));
  mapwindow->addPluginGroup("meteo.map.common");

  mapwindow->document()->setRamkaWidth(1);
  mapwindow->document()->ramka()->setHalfWidth(1);
  meteo::map::proto::Document prop = mapwindow->document()->property();
  prop.set_ruleview(kBadInvisible); 
  prop.mutable_legend()->set_pos(kNoPosition);
  prop.set_cross_type(meteo::map::proto::kCrossDocument);
  mapwindow->document()->setProperty(prop);
  mapwindow->setWindowTitle(tr("Вертикальный разрез"));

  app::MdiSubWindow* sub = nullptr;
  if(nullptr != mainwindow()) {
    sub = mainwindow()->toMdi(mapwindow);
//    QObject::connect( sub, SIGNAL(mdiSubWindowClosed(MdiSubWindow*)), SLOT(slotWindowClosed()) );
  }
  mapwindow->show();

  CreateCutAction* ccact = new CreateCutAction( mapwindow->mapscene());
  ccact->setWidget(cut);
  mapwindow->mapscene()->addAction( ccact, kTopRight, QPoint(1,0));
  ccact->slotShow(true);

  if(nullptr != sub && nullptr != cut) {
    QObject::connect(sub, SIGNAL(mdiSubWindowClosed(MdiSubWindow*)), cut, SLOT(slotWindowClosed()));
  }
  if(nullptr != cut){
       cut->setCutDoc(vdoc);
    }
  //cut->setCutWindow(mapwindow);
  //cut->show();
}

} // map
} // meteo


