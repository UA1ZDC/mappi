#include "histogramaction.h"

#include <qevent.h>

#include <meteo/commons/ui/map/view/mapscene.h>
#include <meteo/commons/ui/map/view/mapview.h>
#include <meteo/commons/ui/map/view/mapwindow.h>
#include <meteo/commons/ui/map/view/menu.h>

namespace meteo {

const QString HistogramAction::kName = "histogramaction";

HistogramAction::HistogramAction(map::MapScene* mapscene)
  : map::Action(mapscene,HistogramAction::kName)
{
  mapscene_ = mapscene;
  widget_ = new HistogramEditor(mapscene->mapview()->window());

  //  menuAct_ = scene_->mapview()->window()->addActionToMenuFromFile(kName);
  auto path = QList<QPair<QString, QString>>({QPair<QString, QString>("tools", QObject::tr("Инструменты"))});
  auto title = QPair<QString, QString>(kName, QObject::tr("Гистограмма"));
  menuAct_ = scene_->mapview()->window()->addActionToMenu(title, path);
  if ( nullptr == menuAct_ ) {
    menuAct_ = new QAction(tr("Гистограмма"), this);
  }
  QObject::connect(menuAct_, SIGNAL(triggered(bool)), this, SLOT(slotShowWidget()));
  menuAct_->setVisible(false);
  map::Document* doc = mapscene_->document();
  if ( !doc->isStub() ) {
    doc->turnEvents();
    if ( 0 != doc->eventHandler()) {
      doc->eventHandler()->installEventFilter(this);
    }
  }
}

HistogramAction::~HistogramAction()
{
  delete widget_;
  widget_ = nullptr;
}

void HistogramAction::addActionsToMenu(QMenu* menu) const
{
  for (auto l : mapscene_->document()->layers() ) {
    if(meteo::kLayerSatellite == l->type()) {
      menu->addAction(menuAct_);
      break;
    }
  }
}

void HistogramAction::slotShowWidget()
{
  widget_->showNormal();
  //widget_->slotLoadImageFromlayer(QString("void"));
}

bool HistogramAction::eventFilter(QObject *obj, QEvent *event) {
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


}//meteo
