#include "oceanplugin.h"
#include "oceanaction.h"

#include <meteo/commons/ui/mainwindow/mainwindow.h>
#include <meteo/commons/ui/mainwindow/widgethandler.h>
#include <meteo/commons/ui/map/view/actions/mousecoordaction.h>
#include <meteo/commons/ui/graph/coordaction/graphcoordaction.h>
#include <meteo/commons/ui/graph/valueaction/graphvalueaction.cpp>
#include <meteo/commons/ui/oceandiagram/oceancore/createoceandiagwidget.h>

namespace meteo {
namespace map {

/*! \brief kMenuItem - Пункт меню { id: "createoceandiag", title: "Диаграмма состояния моря" } */
static const auto kMenuItem = QPair<QString, QString>("createoceandiag", QObject::tr("Диаграмма состояния моря"));

/*!
 * \brief kMenuPath - Путь к пункту меню { id: "tasks", title: "Прикладные задачи"} ->
 * Для добавления вложенности нужно добавить пару в список
*/
static const auto kMenuPath = QList<QPair<QString, QString>>(
{
      QPair<QString, QString>("tasks", QObject::tr("Прикладные задачи"))
});

OceanPlugin::OceanPlugin()
  : MainWindowPlugin("oceanplugin")
{
  static const QString& createOceanDiag = QObject::tr("createoceandiag");
  QAction* action = mainwindow()->findAction(createOceanDiag);
  if ( nullptr == action ){
    warning_log.msgBox() << msglog::kTargetMenyNotFound.arg(createOceanDiag);
  }
  else {
    action->setDisabled(false);
    QObject::connect(action, &QAction::triggered, this, &OceanPlugin::slotOpenOceanDiag);
  }
}

OceanPlugin::~OceanPlugin()
{

}

void OceanPlugin::slotOpenOceanDiag()
{
  meteo::map::proto::Document blank;
  blank.set_projection(kGeneral);
  blank.set_doctype( meteo::map::proto::kOceanDiagram );
  GeoPointPb *gp = blank.mutable_map_center();
  gp->set_lat_radian(32.5);
  gp->set_lon_radian(650);
  gp->set_type(kLA_GENERAL);
  blank.mutable_doc_center()->CopyFrom(*gp);
  blank.set_punchorient(meteo::map::proto::kNoOrient);
  blank.set_scale(16);

  meteo::map::MapWindow* window_ =
      new meteo::map::MapWindow(mainwindow(), blank, "old.geo", MnCommon::etcPath("meteo") + "/tasks.menu.conf");
  window_->setWindowTitle(QObject::tr("Диаграмма состояния моря"));
  window_->document()->setBackgroundColor(QColor(255,255,255));

  if (nullptr != window_->mapscene()) {
    window_->mapscene()->addPluginGroup("meteo.map.common");

  QList< meteo::map::Action*> al =  window_->mapscene()->actions();
  for(int i=0; i< al.count(); ++i){
    if(al.at(i)->name() == "mousecoordaction") {
      meteo::map::MouseCoordAction* act = qobject_cast< meteo::map::MouseCoordAction* >(al.at(i));
      act->setTextFormat("температура %1\xC2\xB0""С   глубина %2 м");
    }
  }
  }

  mainwindow()->toMdi(window_);

  OceanAction* act =new OceanAction( window_->mapscene());
  window_->mapscene()->addAction(act, kTopRight, QPoint(1,0));
  act->slotShow(true);
  auto graph = new GraphCoordAction(window_->mapscene(), GraphCoordAction::GraphType::kOcean);
  window_->mapscene()->addAction(graph);
  auto value = new GraphValueAction(window_->mapscene(), GraphValueAction::GraphType::kOcean);
  window_->mapscene()->addAction(value);
}

}
}
