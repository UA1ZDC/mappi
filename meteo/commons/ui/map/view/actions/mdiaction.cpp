#include "mdiaction.h"
#include <meteo/commons/ui/mainwindow/widgethandler.h>
#include "../mapview.h"
#include "../mapscene.h"
#include "../menu.h"

namespace meteo {
namespace map {

MdiAction::MdiAction( MapScene* scene )
  : Action(scene, "mdiaction" ),
  btnlayers_( new ActionButton )
{
  btnlayers_->setToolTip( QObject::tr("Встроить окно") );
  btnlayers_->setMaximumSize(QSize(16,16));
  btnlayers_->setMinimumSize(QSize(16,16));
  btnlayers_->setIconSize(QSize(16,16));
  btnlayers_->setIcon( QIcon(":/meteo/icons/map/rollup16x16.png") );
  btnlayers_->setCheckable(false);

  connect(btnlayers_, SIGNAL(clicked()), SLOT(insertIntoMdi()));

  btnitem_ = scene_->addWidget(btnlayers_);
  btnitem_->setZValue(15000);
  item_ = btnitem_;
}

MdiAction::~MdiAction()
{
  delete btnitem_; btnitem_= 0;
  item_ = 0;
}

void MdiAction::addActionsToMenu( Menu* menu ) const
{
  if(0 == menu ) return;
  if( !menu->isEmpty()){  menu->addSeparator();  }
  menu->addAction("Встроить окно",this, SLOT (insertIntoMdi()) );
}

void MdiAction::insertIntoMdi()
{
  if( 0 == scene_ ){
    return;
  }
  if( 0 == scene_->document() ){
    return;
  }
}

void MdiAction::keyPressEvent(QKeyEvent* event)
{
  if( event->key() == Qt::Key_O ){
    if(  Qt::CTRL == qApp->keyboardModifiers()  ){
      insertIntoMdi();
    }
  }
  Action::keyPressEvent(event);
}

} //map
} //meteo
