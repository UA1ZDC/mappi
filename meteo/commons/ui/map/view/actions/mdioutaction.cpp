#include "mdioutaction.h"
#include <meteo/commons/ui/mainwindow/widgethandler.h>
#include "../mapview.h"
#include "../mapscene.h"
#include "../menu.h"

namespace meteo {
namespace map {

MdiOutAction::MdiOutAction( MapScene* scene )
  : Action(scene, "mdioutaction" ),
  btnlayers_( new ActionButton )
{
  btnlayers_->setToolTip( QObject::tr("Извлечь окно") );
  btnlayers_->setIcon( QIcon(":/meteo/icons/map/expand16x16") );
  btnlayers_->setMaximumSize(QSize(16,16));
  btnlayers_->setMinimumSize(QSize(16,16));
  btnlayers_->setIconSize(QSize(16,16));
  btnlayers_->setCheckable(false);

  connect(btnlayers_, SIGNAL(clicked()), SLOT(extractFromMdi()));

  btnitem_ = scene_->addWidget(btnlayers_);
  btnitem_->setZValue(15000);
  item_ = btnitem_;
}

MdiOutAction::~MdiOutAction()
{
  delete btnitem_; btnitem_= 0;
  item_ = 0;
}

void MdiOutAction::addActionsToMenu( Menu* menu ) const
{
  if(0 == menu ) {
    return;
  }
  if( false == menu->isEmpty() ) {
    menu->addSeparator();
  }
  menu->addAction("Извлечь окно",this, SLOT (extractFromMdi()) );
}

void MdiOutAction::extractFromMdi()
{
  //TODO
}

void MdiOutAction::keyPressEvent(QKeyEvent* event)
{
  if( event->key() == Qt::Key_O ){
    if(  Qt::CTRL == qApp->keyboardModifiers()  ){
      extractFromMdi();
    }
  }
  Action::keyPressEvent(event);
}

} //map
} //meteo
