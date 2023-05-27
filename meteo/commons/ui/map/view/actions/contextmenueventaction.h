#ifndef METEO_MAP_VIEW_ACTIONS_CONTEXTMENUEVENTACTION_H
#define METEO_MAP_VIEW_ACTIONS_CONTEXTMENUEVENTACTION_H

#include "action.h"
#include <qmenu.h>
#include <qpoint.h>

namespace meteo {
namespace map {

class MapView;

class ContextMenuEventAction : public Action
{
  Q_OBJECT
  public:
    ContextMenuEventAction( MapScene* scene );
    ~ContextMenuEventAction();

    void contextMenuEvent( QContextMenuEvent* e );

  private:
};

}
}

#endif
