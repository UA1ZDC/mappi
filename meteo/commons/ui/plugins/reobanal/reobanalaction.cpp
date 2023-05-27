#include "reobanalaction.h"
#include "reobanalwidget.h"

#include <meteo/commons/ui/map/view/mapscene.h>
#include <meteo/commons/ui/map/view/widgetitem.h>
#include <meteo/commons/ui/map/view/menu.h>
#include <cross-commons/debug/tlog.h>

#include <commons/geobasis/geopoint.h>
#include <commons/obanal/tfield.h>

#include <meteo/commons/global/global.h>
#include <meteo/commons/ui/map/document.h>

#include <meteo/commons/ui/map/geopolygon.h>
#include <meteo/commons/ui/map/view/mapview.h>

#include <meteo/commons/ui/map/view/mapscene.h>

#include <meteo/commons/services/map/mapservice.h>

namespace meteo {
namespace map {

  ReobanalAction ::ReobanalAction ( MapScene* scene )
  : Action(scene, "ReobanalAction" ),
  dlg_(nullptr)
{
}

ReobanalAction ::~ReobanalAction (  ){
  if(nullptr != dlg_ ){ delete dlg_; dlg_ = nullptr;}
  
}

void ReobanalAction::addActionsToMenu( Menu* menu ) const
{
  if(nullptr == menu ||nullptr == scene_
    || nullptr == scene_->document()
    || nullptr == scene_->document()->activeLayer()) {
    return;
  }

  Layer* l = scene_->document()->activeLayer();

  switch (l->type()) {
    case kLayerPunch:
      break;
    case kLayerIso:
     if( 250 != scene_->document()->activeLayer()->info().center()
           && 251 != scene_->document()->activeLayer()->info().center()){
     return;
      }
      if ( true != l->hasField() ) {
        return ;
      }
      break;
    default:
      return;
    }

  QMenu* m =  menu->addMenu(QObject::tr("Анализ"));
  if(nullptr != m ){
    m->addSeparator();
    m->addAction("Повторный анализ", this, SLOT(slotNeedReanalyse()));
    menu->addAnalyseAction(m->menuAction()); 
  }
}


void ReobanalAction::slotNeedReanalyse()
{
  QAction* act = static_cast<QAction*>(sender());
  
  if( nullptr == act || nullptr == scene_
    || nullptr == scene_->document()
    || nullptr == scene_->document()->activeLayer()) {
    return;
    }
    
   Layer* l = scene_->document()->activeLayer();
   if(nullptr == l || (false == l->hasData())){
       return;
     }
  if(nullptr != dlg_){
    delete dlg_;
    dlg_ = nullptr;
  }

  dlg_ = new ReobanalWidget(scene_->mapview());
  dlg_->setLayer(l);
  dlg_->setProp(l->info());
  dlg_->show();
  
  
}

  



}
}
