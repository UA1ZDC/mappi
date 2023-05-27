#ifndef METEO_MAP_VIEW_ACTIONS_LEGENDACTION_H
#define METEO_MAP_VIEW_ACTIONS_LEGENDACTION_H

#include "action.h"
#include "../actionbutton.h"
#include <qpoint.h>
#include <qgraphicsitem.h>
#include <qslider.h>

#include <meteo/commons/ui/map/legend.h>

class QToolButton;
class QEvent;
class QTimer;

namespace meteo {
namespace map {

class LegendAction : public Action
{
  Q_OBJECT
  public:
    LegendAction( MapScene* scene );
    ~LegendAction();

    void addActionsToMenu( Menu* menu ) const ;

  private:
    QMenu* legendmenu_;
    QAction* hideaction_;

    Legend* legend() const ;
    void showLegend( Position pos );

  private slots:
    void slotSaveLegend();
    void slotHideLegend();
    void slotShowLegendTop();
    void slotShowLegendBottom();
    void slotShowLegendLeft();
    void slotShowLegendRight();

    void slotAboutToShowParentMenu();
};

}
}

#endif
