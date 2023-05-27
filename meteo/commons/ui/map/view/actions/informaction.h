#ifndef METEO_MAP_VIEW_ACTIONS_INFORMACTION_H
#define METEO_MAP_VIEW_ACTIONS_INFORMACTION_H

#include "action.h"
#include "../actionbutton.h"
#include <qpoint.h>
#include <qgraphicsitem.h>

class QToolButton;
class QEvent;
class QTimer;

namespace meteo {
namespace map {

class InformWidget;

class InformAction : public Action
{
  Q_OBJECT
  public:
    InformAction( MapScene* scene );
    ~InformAction();

    void addActionsToMenu( Menu* menu ) const ;

  private:
    QAction* showaction_;
    InformWidget* info_;


  private slots:
    void slotShowInfo();
};

}
}

#endif
