#include "synopaction.h"

#include <qevent.h>
#include <qtoolbutton.h>
#include <qgraphicssceneevent.h>
#include <qgraphicsproxywidget.h>
#include <qgraphicswidget.h>
#include <qobject.h>
#include <qdebug.h>
#include <qicon.h>
#include <qlabel.h>
#include <qcursor.h>

#include <cross-commons/debug/tlog.h>
#include <commons/geobasis/geopoint.h>
#include <commons/obanal/tfield.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/ui/map/document.h>
#include <meteo/commons/ui/map/incut.h>
#include <meteo/commons/ui/map/geopolygon.h>
#include <meteo/commons/ui/map/layeriso.h>
#include <meteo/commons/ui/map/view/mapview.h>
#include <meteo/commons/ui/map/view/mapscene.h>
#include <meteo/commons/ui/map/view/widgetitem.h>
#include <meteo/commons/ui/map/view/menu.h>


namespace meteo {
namespace map {

SynopAction::SynopAction( MapScene* scene )
  : Action(scene, "SynopAction" ),
    swidget_(nullptr)
{


 }

SynopAction::~SynopAction()
{
}

void SynopAction::addActionsToMenu(Menu *menu) const
{
  if(nullptr == menu ) {
    return;
  }
  if( !menu->isEmpty()){
    menu->addSeparator();
  }
  Layer* l = scene_->document()->activeLayer();
  if(nullptr == l) {
    return ;
  }
  if ( true != l->hasField() || nullptr == l->field()) {
    return;
  }
  int dscr = l->field()->getDescr();
  if ( (10009 != dscr) && (10051 != dscr) && (10004 != dscr) ) {
    return;
  }
    
    QMenu* m =  menu->addMenu(QObject::tr("Анализ"));
    if(nullptr != m ){
      m->addSeparator();
      m->addAction(QObject::tr("Синоптическая ситуация"), this, SLOT(slotSynop()));
      menu->addAnalyseAction(m->menuAction());
    }  
}

void SynopAction::mousePressEvent(QMouseEvent *e)
{
  if( nullptr == scene_->mapview() ){
    Action::mousePressEvent(e);
    return;
  }
  point_ = scene_->mapview()->mapToScene( e->pos() ).toPoint();
  Action::mousePressEvent(e);
}



void SynopAction::slotSynop()
{
  QAction* act = static_cast<QAction*>(sender());
  if( nullptr == act ){
    return;
  }
    GeoPoint gp = scene_->screen2coord(point_);
    Layer* l = scene_->document()->activeLayer();
    if(nullptr == l) {
      return ;
    }

    obanal::TField* f = l->field();
    if(nullptr == f){
      return;
    }

    QString sit;
    QString fenom;
    int num_sit;


    f->oprSynSit5(gp,&sit,&fenom,&num_sit);

    if( nullptr == sit || nullptr == fenom ){
      return;
    }
//    QString head1 = QString("<b>Тип синоптической ситуации в точке с координатами:</b>\n");
//    QString head2 = QString("\n<b>Ожидаемые опасные явления погоды:</b>");
    QString head = QString("<b>Тип синоптической ситуации в точке с координатами:</b>"+gp.strLat()+" "+gp.strLon()) ;

    if( nullptr == swidget_ ){
      swidget_ = new SynopWidget(scene_->mapview());
      swidget_->setHeader1(head);
      swidget_->setSit(sit);
      swidget_->setFenom(fenom);
      swidget_->activateWindow();
    }
    if( nullptr !=swidget_  ){
      swidget_->show();
      swidget_->setHeader1(head);
      swidget_->setSit(sit);
      swidget_->setFenom(fenom);
      swidget_->activateWindow();

    }


}

}
}
