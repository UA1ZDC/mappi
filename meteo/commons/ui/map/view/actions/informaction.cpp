#include "informaction.h"

#include <qevent.h>
#include <qgraphicssceneevent.h>
#include <qobject.h>
#include <qtoolbutton.h>
#include <qpalette.h>
#include <qgraphicslinearlayout.h>
#include <qgraphicsproxywidget.h>
#include <qdebug.h>
#include <qmatrix.h>
#include <qmath.h>
#include <qcoreapplication.h>
#include <qcursor.h>
#include <qtimer.h>

#include <cross-commons/debug/tlog.h>
#include <commons/geobasis/projection.h>
#include <meteo/commons/global/global.h>

#include "../mapview.h"
#include "../mapscene.h"
#include "../widgetitem.h"
#include "../menu.h"
#include "informwidget.h"

namespace meteo {
namespace map {

InformAction::InformAction( MapScene* scene )
  : Action(scene, "informaction" ),
  showaction_( new QAction( QObject::tr("Служебная информация"), this ) ),
  info_(0)
{
  QObject::connect( showaction_, SIGNAL( triggered() ), this, SLOT( slotShowInfo() ) );
}

InformAction::~InformAction()
{
  delete showaction_; showaction_ = 0;
  delete info_; info_ = 0;
}

void InformAction::addActionsToMenu( Menu* menu ) const 
{
  menu->addDocumentAction(showaction_);
}

void InformAction::slotShowInfo()
{
  if ( 0 == view() ) {
    return;
  }
  if ( 0 == info_ ) {
    info_ = new InformWidget( view()->window() );
  }
  info_->show();
}

}
}
