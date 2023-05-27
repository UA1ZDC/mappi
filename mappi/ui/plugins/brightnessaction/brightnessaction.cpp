#include "brightnessaction.h"

#include <meteo/commons/ui/map/view/mapscene.h>
#include <meteo/commons/ui/map/view/mapview.h>
#include <meteo/commons/ui/map/view/mapwindow.h>

#include <cross-commons/debug/tlog.h>

namespace meteo {
namespace map {

const QString BrightnessAction::kName = "brightnessaction";

BrightnessAction::BrightnessAction(MapScene* scene):
  Action(scene, BrightnessAction::kName),
  mapscene_(scene),
  widget_(new BrightnessContrastWidget(scene->mapview()->window()))
{
  auto path = QList<QPair<QString, QString>>({QPair<QString, QString>("tools", QObject::tr("Инструменты"))});
  auto title = QPair<QString, QString>(kName, QObject::tr("Яркость/Контраст"));

  //  menuAct_ = scene_->mapview()->window()->addActionToMenuFromFile(kName);
  menuAct_ = scene_->mapview()->window()->addActionToMenu(title, path);
  //var(menuAct_);
  if ( nullptr == menuAct_ ) {
    menuAct_ = new QAction(tr("Яркость и контраст"), this);
  }
  connect(menuAct_, SIGNAL(triggered(bool)), this, SLOT(slotShowDialog()));
  
  menuAct_->setVisible(false);
  map::Document* doc = mapscene_->document();
  if ( !doc->isStub() ) {
    doc->turnEvents();
    if ( 0 != doc->eventHandler()) {
      doc->eventHandler()->installEventFilter(this);
    }
  }
}

BrightnessAction::~BrightnessAction()
{
  delete widget_;
  delete menuAct_;
}

void BrightnessAction::addActionsToMenu(QMenu *menu) const
{
  for (auto l : mapscene_->document()->layers() ) {
    if(meteo::kLayerSatellite == l->type()) {
      menu->addAction(menuAct_);
      break;
    }
  }
}

bool BrightnessAction::eventFilter(QObject *obj, QEvent *event)
{
  Q_UNUSED( obj );
  if (map::LayerEvent::LayerChanged != event->type() ) {return false; }

  map::Document* doc = mapscene_->document();
  if ( 0 == doc ) {
    return false;
  }
  map::LayerEvent* ev = static_cast<map::LayerEvent*>(event);

  if ( map::LayerEvent::Added == ev->changeType() ) {
    map::Layer* layer = doc->layerByUuid( ev->layer());
    if ( meteo::kLayerSatellite == layer->type() ) {
      menuAct_->setVisible(true);
    }
  }

  if ( map::LayerEvent::Deleted == ev->changeType() ) {
    QList<map::Layer*> layers = doc->layers();
    bool visible=false;
    foreach (map::Layer* l, layers) {
      if (meteo::kLayerSatellite == l->type()) {
        visible=true;
        break;
      }
    }
    menuAct_->setVisible(visible);
  }
  return false;
}

void BrightnessAction::slotShowDialog()
{
  widget_->showNormal();
}

}
}

