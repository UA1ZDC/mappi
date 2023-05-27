#include "frontidentaction.h"
#include "frontidentwidget.h"

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

  FrontIdentAction ::FrontIdentAction ( MapScene* scene )
  : Action(scene, "frontidentaction" ),
  dlg_(nullptr)
{
}

FrontIdentAction ::~FrontIdentAction (  ){
  if(nullptr != dlg_ ){ delete dlg_; dlg_ = nullptr;}
  
}

void FrontIdentAction::addActionsToMenu( Menu* menu ) const
{
  if(nullptr == menu ||nullptr == scene_
    || nullptr == scene_->document()
    || nullptr == scene_->document()->activeLayer()) {
    return;
  }
/*
  Layer* l = scene_->document()->activeLayer();

  if ( true != l->hasField() ) {
    return ;
  }
*/
  QMenu* m =  menu->addMenu(QObject::tr("Анализ"));
  if(nullptr != m ){
    m->addSeparator();
    m->addAction("Фронтологический анализ", this, SLOT(slotAddFronts()));
    menu->addAnalyseAction(m->menuAction()); 
  }
}


void FrontIdentAction::slotAddFronts()
{
  QAction* act = static_cast<QAction*>(sender());
  
  if( nullptr == act || nullptr == scene_
    || nullptr == scene_->document()
    || nullptr == scene_->document()->activeLayer()) {
    return;
    }
    
   Layer* l = scene_->document()->activeLayer();
  
  if(nullptr != dlg_){
    delete dlg_;
    dlg_ = nullptr;
  }
  QApplication::setOverrideCursor(Qt::WaitCursor);

  dlg_ = new FrontIdentWidget(scene_->mapview());
  dlg_->setDocument( scene_->document() );
  if (nullptr != l ) {
      dlg_->setProp(l->field());
    }
  dlg_->slotConnect();
  QApplication::restoreOverrideCursor();
  dlg_->show();

 /* if(dlg_->prepFields(l->field())){
    dlg_->show();  
  } else {
    error_log.msgBox()<< meteo::msglog::kDataNoDataSrok
    .arg(l->field()->getDate().toString("dd-MM hh:mm"))
    .arg(l->field()->getCenterName());
    if(nullptr != dlg_){
      delete dlg_;
      dlg_ = nullptr;
    }
    
  }*/
  
  
}

  



}
}
