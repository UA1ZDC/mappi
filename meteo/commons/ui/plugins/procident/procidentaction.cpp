#include "procidentaction.h"
//#include "procidentwidget.h"
#include "fielddescrwidget.h"
#include "object_trajection.h"
#include "prepeartrajwidget.h"

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

  ProcIdentAction ::ProcIdentAction ( MapScene* scene )
  : Action(scene, "procidentaction" ),
  dlg_(nullptr),obj_traj_widget_(nullptr)
{
}

ProcIdentAction ::~ProcIdentAction (  ){
  if(nullptr != dlg_ ){ delete dlg_; }
  if(nullptr != obj_traj_widget_ ){ delete obj_traj_widget_; }
  
}

void ProcIdentAction::addActionsToMenu( Menu* menu ) const
{
  if(nullptr == menu ||nullptr == scene_
    || nullptr == scene_->document()
    || nullptr == scene_->document()->activeLayer()) {
    return;
  }

  Layer* l = scene_->document()->activeLayer();

  if ( true != l->hasField() ) {
    return ;
  }
  
  
  if( !menu->isEmpty()){
    menu->addSeparator();
  }
  if ( 15000 != l->field()->getLevelType() && (10009 == l->field()->getDescr() || 10051 == l->field()->getDescr() ) ) {
    
    QMenu* m =  menu->addMenu(QObject::tr("Анализ"));
    if(nullptr != m ){
      m->addSeparator();
     // m->addAction("Определить типы синоптических процессов ", this, SLOT(slotRecognFields()));
     // m->addAction("Добавить поле в список эталонных полей", this, SLOT(slotAddFields()));
      m->addAction("Построить траектории барических образований", this, SLOT(slotAddObjTrajectory()));
      menu->addAnalyseAction(m->menuAction());
    }
  } 
}


void ProcIdentAction::slotAddObjTrajectory()
{
  QAction* act = static_cast<QAction*>(sender());
  
  if(nullptr != obj_traj_widget_){
    
    delete obj_traj_widget_;
    obj_traj_widget_ = nullptr;
    
  }
  
  if( nullptr == act || nullptr == scene_
    || nullptr == scene_->document()
    || nullptr == scene_->document()->activeLayer()) {
    return;
  }

  Layer* l = scene_->document()->activeLayer();
  
  if ( true != l->hasField() ||  15000 == l->field()->getLevelType()
       ||  (10009 != l->field()->getDescr() &&  10051 != l->field()->getDescr() ) ) {
    return ;
  }
  obj_traj_widget_ = new PrepearTrajWidget( scene_->document(), view()->window() );
  obj_traj_widget_->show();
}


void ProcIdentAction::slotAddFields()
{
  QAction* act = static_cast<QAction*>(sender());
  
  if( nullptr == act || nullptr == scene_
    || nullptr == scene_->document()
    || nullptr == scene_->document()->activeLayer()) {
    return;
    }
    
    Layer* l = scene_->document()->activeLayer();
  
  if ( true != l->hasField() ||  (10009 != l->field()->getDescr() &&  10051 != l->field()->getDescr())  ) {
    return ;
  }
  
  if(nullptr != dlg_){
    delete dlg_;
    dlg_ = nullptr;
  }
  
  dlg_ = new ProcIdentWidget(scene_->mapview(),l->field(),0);
  dlg_->setDocument( scene_->document() );
  
  connect(dlg_, SIGNAL(addRect(const QRect& )), this, SLOT(getField(const QRect& )));
  
  dlg_->show();
}

void ProcIdentAction::slotRecognFields()
{
  QAction* act = static_cast<QAction*>(sender());
  
  if( nullptr == act || nullptr == scene_
    || nullptr == scene_->document()
    || nullptr == scene_->document()->activeLayer()) {
    return;
    }
    
    Layer* l = scene_->document()->activeLayer();
  
  if ( true != l->hasField() ) {
    return ;
  }
  
  if(nullptr != dlg_){
    delete dlg_;
    dlg_ = nullptr;
  }

  
  dlg_ = new ProcIdentWidget(scene_->mapview(),l->field(),1);
  dlg_->setDocument( scene_->document() );
  dlg_->show();
  
}

void ProcIdentAction::getField(const QRect& ){
  
    QApplication::restoreOverrideCursor();
}
  



}
}
