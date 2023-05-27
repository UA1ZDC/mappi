#include "layersaction.h"
#include <meteo/commons/ui/mainwindow/widgethandler.h>
#include "../widgets/layerswidget.h"

#include <qevent.h>
#include <qgraphicssceneevent.h>
#include <qgraphicsproxywidget.h>
#include <qgraphicslinearlayout.h>
#include <qobject.h>
#include <qtoolbutton.h>
#include <qdebug.h>
#include <qpalette.h>
#include <qtreewidget.h>
#include <qheaderview.h>
#include <qmessagebox.h>
#include <qgraphicseffect.h>
#include <qevent.h>

#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>
#include <commons/geobasis/projection.h>
#include <meteo/commons/ui/map/document.h>
#include <meteo/commons/ui/map/object.h>
#include <meteo/commons/ui/map/event.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/ui/map/geopolygon.h>
#include <meteo/commons/ui/map/geotext.h>
#include "../mapwindow.h"

#include <qgraphicseffect.h>

#include "../mapview.h"
#include "../mapscene.h"
#include "../menu.h"

static const auto kMenuItem = QPair<QString, QString>("layermanager", QObject::tr("Управление"));
static const auto kMenuPath = QList<QPair<QString, QString>>
    ({QPair<QString, QString>("layers", QObject::tr("Слои"))});

namespace meteo {
namespace map {

LayersAction::LayersAction( MapScene* scene )
  : Action(scene, "layersaction" ),
  btnlayers_( new ActionButton ),
    layerswidget_(0),
    deletevisible_(true)
{
  btnlayers_->setToolTip( QObject::tr("Слои") );
  btnlayers_->setIcon( QIcon(":/meteo/icons/map/layers.png") );
  btnlayers_->setToolTip("Слои");
  QObject::connect( btnlayers_, SIGNAL( toggled(bool) ), this, SLOT( slotShow(bool) ) );
  btnitem_ = scene_->addWidget(btnlayers_);
  btnitem_->setZValue(15000);
  item_ = btnitem_;
//  QAction* a = scene_->mapview()->window()->addActionToMenuFromFile("layermanager");
  QAction* a = scene_->mapview()->window()->addActionToMenu(kMenuItem, kMenuPath);
  if ( 0 != a ) {
    QObject::connect( a, SIGNAL( triggered() ), btnlayers_, SLOT( toggle() ) );
  }
}

LayersAction::~LayersAction()
{
}

void LayersAction::slotShow( bool fl )
{
  if( true == fl ) {
    if ( 0 == layerswidget_ ) {
      layerswidget_ = new LayersWidget( scene_->mapview()->window() );
      layerswidget_->setDeleteVisible(deletevisible_);
      layerswidget_->installEventFilter(this);
      QObject::connect( layerswidget_, SIGNAL( closed() ), btnlayers_, SLOT( toggle() ) );
    }
    layerswidget_->showNormal();
  }
  else {
    layerswidget_->hide();
  }
}

void LayersAction::slotLayerWidgetClosed()
{
  btnlayers_->toggle();
//  slotShow(false);
}

void LayersAction::addActionsToMenu( Menu* menu ) const
{
  if(0 == menu ){
    return;
  }
  if( !menu->isEmpty()){
    menu->addSeparator();
  }
  if( false == btnlayers_->isChecked() ) {
    QAction* a = new QAction( QObject::tr("Управление слоями"), 0 );
    QObject::connect( a, SIGNAL( triggered() ), btnlayers_, SLOT( toggle() ) );
    menu->addLayerAction(a);
  }
}

void LayersAction::setDeleteVisible( bool fl )
{
  deletevisible_ = fl;
  if ( 0 != layerswidget_ ) {
    layerswidget_->setDeleteVisible(fl);
  }
}


bool LayersAction::eventFilter(QObject* obj, QEvent *event)
{
  Q_UNUSED(obj);
  if (QEvent::Hide == event->type()) {
    if (0!= layerswidget_) {
      btnlayers_->blockSignals(true);
      btnlayers_->toggle();
      btnlayers_->blockSignals(false);
    }
  }
  if(QEvent::HideToParent == event->type()) {
    if (0!= layerswidget_) {
      btnlayers_->blockSignals(true);
      btnlayers_->toggle();
      btnlayers_->blockSignals(false);
    }
  }
  return false;
}

} //map
} //meteo
