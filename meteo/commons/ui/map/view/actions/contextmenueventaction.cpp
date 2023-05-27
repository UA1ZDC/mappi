#include "contextmenueventaction.h"

#include <qevent.h>
#include <qgraphicssceneevent.h>
#include <qobject.h>
#include <qdebug.h>

#include <cross-commons/debug/tlog.h>
#include <meteo/commons/global/global.h>

#include "../mapview.h"
#include "../mapscene.h"
#include "../menu.h"

namespace meteo {
namespace map {

ContextMenuEventAction::ContextMenuEventAction( MapScene* s )
  : Action( s, "contextmenueventaction" )
{
}

ContextMenuEventAction::~ContextMenuEventAction()
{
}

void ContextMenuEventAction::contextMenuEvent( QContextMenuEvent* e )
{
  Menu* menu = new Menu( view() );
  QList<Action*> list = scene_->baseactions();
  for ( int i = 0, sz = list.size(); i < sz; ++i ) {
    list[i]->addActionsToMenu( menu );
  }
  list = scene_->actions();
  for ( int i = 0, sz = list.size(); i < sz; ++i ) {
    list[i]->addActionsToMenu( menu );
  }
  menu->buildMenu();
  if ( false == menu->isEmpty() ) {
    menu->exec( e->globalPos() );
  }
  delete menu;
}

}
}
