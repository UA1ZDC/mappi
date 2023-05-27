#include "createcutaction.h"

#include <meteo/commons/ui/mainwindow/widgethandler.h>

#include <qevent.h>
#include <qgraphicssceneevent.h>
#include <qgraphicsproxywidget.h>
#include <qgraphicslinearlayout.h>
#include <qobject.h>
#include <qdebug.h>

#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>
#include <commons/geobasis/projection.h>
#include <meteo/commons/ui/map/document.h>
#include <meteo/commons/ui/map/object.h>
#include <meteo/commons/ui/map/event.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/ui/map/view/mapview.h>
#include <meteo/commons/ui/map/view/mapscene.h>
#include <meteo/commons/ui/aero/verticalcut/createcutwidget.h>

#include <qgraphicseffect.h>


namespace meteo {

  //кнопка на карте для открытия/закрыти окна с списком точек
CreateCutAction::CreateCutAction( map::MapScene* scene )
  : Action(scene, "createcutaction" ),
    btncut_( new ActionButton ),
    cutwidget_(nullptr)
{
  btncut_->setToolTip( QObject::tr("Маршрут") );
  btncut_->setIcon( QIcon(":/meteo/icons/map/settings.png") );
  QObject::connect( btncut_, SIGNAL( toggled(bool) ), this, SLOT( slotShow(bool) ) );
  btnitem_ = scene_->addWidget(btncut_);
  btnitem_->setZValue(15000);
  item_ = btnitem_;
}

CreateCutAction::~CreateCutAction()
{
  //delete cutwidget_;
  //cutwidget_ = 0;
}

void CreateCutAction::slotShow( bool fl )
{
  if(nullptr == cutwidget_) return;
  if( true == fl ) {
    cutwidget_->show();
  }
  else {
    cutwidget_->hide();
  }
}

void CreateCutAction::setWidget(meteo::CreateCutWidget* widget)
{
  cutwidget_ = widget;
  if ( nullptr == cutwidget_ ) {return;}
  btncut_->toggle();
  QObject::connect( cutwidget_, SIGNAL( finished(int) ), btncut_, SLOT( toggle() ) );

}

void CreateCutAction::slotLayerWidgetClosed()
{
  btncut_->toggle();
}

}
