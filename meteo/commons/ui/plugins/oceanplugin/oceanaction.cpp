#include "oceanaction.h"

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
#include <meteo/commons/ui/oceandiagram/oceancore/createoceandiagwidget.h>

#include <qgraphicseffect.h>


namespace meteo {
namespace map {

  //кнопка на карте для открытия/закрыти окна с списком точек
OceanAction::OceanAction( MapScene* scene )
  : Action(scene, "oceanaction" ),
    btnocean_( new ActionButton ),
    oceanwidget_(nullptr)
{
  btnocean_->setToolTip( QObject::tr("Список станций") );
  btnocean_->setIcon( QIcon(":/meteo/icons/map/buoy.png") );
  QObject::connect( btnocean_, SIGNAL( toggled(bool) ), this, SLOT( slotShow(bool) ) );
  btnitem_ = scene_->addWidget(btnocean_);
  btnitem_->setZValue(15000);
  item_ = btnitem_;
}

OceanAction::~OceanAction()
{
  if (nullptr != oceanwidget_) {
    delete oceanwidget_;
    oceanwidget_ = nullptr;
  }
}

void OceanAction::slotShow( bool fl )
{
  if( true == fl ) {
    if ( nullptr == oceanwidget_ ) {
      oceanwidget_ = new meteo::odiag::CreateOceanDiagWidget( scene_->mapview()->window() );
      btnocean_->toggle();
      QObject::connect( oceanwidget_, SIGNAL( finished(int) ), btnocean_, SLOT( toggle() ) );
    }
    oceanwidget_->showNormal();
  }
  else {
    oceanwidget_->hide();
  }
}



void OceanAction::slotLayerWidgetClosed()
{
  btnocean_->toggle();
}

} //map
} //meteo
