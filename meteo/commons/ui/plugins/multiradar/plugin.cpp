#include "plugin.h"

#include <cross-commons/debug/tlog.h>
#include <meteo/commons/ui/map/view/mapwindow.h>
#include <meteo/commons/ui/map/view/mapview.h>
#include <meteo/commons/ui/map/view/menu.h>
#include <meteo/commons/ui/map/layermrl.h>
#include <meteo/commons/global/weatherloader.h>

#include "multiradar.h"

namespace meteo {
namespace map {

Plugin::Plugin()
  : ActionPlugin("multiradaraction")
{
}

Plugin::~Plugin()
{
}

Action* Plugin::create( MapScene* scene ) const
{
  return new MultiRadarAction(scene);
}

MultiRadarAction::MultiRadarAction( MapScene* scene )
  : Action( scene, "multiradaraction" ),
  multiaction_( new QAction("Сделать мультик", this ) ),
  widget_(0)
{
  QObject::connect( multiaction_, SIGNAL( triggered() ), this, SLOT( slotOpenMultiRadar() ) );
}

MultiRadarAction::~MultiRadarAction()
{
  delete multiaction_; multiaction_ = 0;
}

void MultiRadarAction::addActionsToMenu( Menu* menu ) const
{
  if ( 0 == scene_->document() ) {
    return;
  }
  LayerMrl* l = maplayer_cast<LayerMrl*>( scene_->document()->activeLayer() );
  if ( 0 == l ) {
    return;
  }
  menu->addLayerAction(multiaction_);
}
    
void MultiRadarAction::keyReleaseEvent( QKeyEvent* event )
{
  if ( Qt::Key_Escape == event->key() && 0 != widget_ && true == widget_->animation() ) {
    widget_->stopAnimation();
  }
}

void MultiRadarAction::slotOpenMultiRadar()
{
  meteo::map::MapWindow* mainwidget = scene_->mapview()->window();
  if ( 0 == mainwidget ) {
    return;
  }
  if ( 0 == widget_ ) {
    widget_ = new MultiRadar(mainwidget);
    Layer* l = maplayer_cast<LayerMrl*>( scene_->document()->activeLayer() );
    if ( 0 != l ) {
      widget_->setCurrentLayer(l);
    }
    QObject::connect( widget_, SIGNAL( destroyed( QObject* ) ), this, SLOT( slotMultiRadarDestroyed( QObject* ) ) );
  }
  widget_->show();
}

void MultiRadarAction::slotMultiRadarDestroyed( QObject* o )
{
  if ( o == widget_ ) {
    widget_ = 0;
  }
}

Q_EXPORT_PLUGIN2( punchplug, Plugin )

}
}
