#include <meteo/commons/ui/map/view/mapscene.h>
#include <meteo/commons/ui/map/view/mapview.h>
#include <meteo/commons/ui/map/view/mapwindow.h>

#include <cross-commons/debug/tlog.h>


#include "montageaction.h"

namespace meteo {
namespace map {

const QString MontageAction::kName = "montageaction";
  
MontageAction::MontageAction(meteo::map::MapScene *scene):
  Action(scene, MontageAction::kName),
  mapscene_(scene),
  widget_(new ThematicListWidget(scene->mapview()->window()))
{
  auto path = QList<QPair<QString, QString>>({QPair<QString, QString>("deposition", QObject::tr("Наноска"))});
  auto title = QPair<QString, QString>(kName, QObject::tr("Монтаж снимков"));
  
  menuAct_ = scene_->mapview()->window()->addActionToMenu(title, path);
  if ( nullptr == menuAct_ ) {
    menuAct_ = new QAction(tr("Монтаж снимков"), this);
  }
  connect(menuAct_, SIGNAL(triggered(bool)), this, SLOT(slotShowDialog()));
}

MontageAction::~MontageAction()
{
  delete widget_;
  delete menuAct_;
}

void MontageAction::addActionsToMenu(QMenu *menu) const
{
  menu->addAction(menuAct_);
}

void MontageAction::slotShowDialog()
{
  widget_->init();
  widget_->showNormal();
}

} // map
} // meteo
