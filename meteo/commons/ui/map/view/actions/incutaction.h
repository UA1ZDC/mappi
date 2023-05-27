#ifndef METEO_MAP_VIEW_ACTIONS_INCUTACTION_H
#define METEO_MAP_VIEW_ACTIONS_INCUTACTION_H

#include "action.h"
#include "../actionbutton.h"
#include <qpoint.h>
#include <qgraphicsitem.h>
#include <qslider.h>

#include <meteo/commons/ui/map/incut.h>

class QToolButton;
class QEvent;
class QTimer;

namespace meteo {
namespace map {

class IncutAction : public Action
{
  Q_OBJECT
  public:
    IncutAction( MapScene* scene );
    ~IncutAction();

    void addActionsToMenu( Menu* menu ) const ;

  private:
    QMenu* incutmenu_;
    QAction* hideaction_;
    QAction* sostavaction_;

    QAction* mapshow_;
    QAction* wmoshow_;
    QAction* authorshow_;
    QAction* dateshow_;
    QAction* termshow_;
    QAction* scaleshow_;
    QAction* projshow_;
    QAction* baselayershow_;
    QAction* layershow_;

    bool hasIncuts() const ;

    Incut* incut() const ;

    void showIncut( Position pos );

  private slots:
    void slotSaveIncut();
    void slotHideIncut();
    void slotShowIncutTopLeft();
    void slotShowIncutTopRight();
    void slotShowIncutBottomLeft();
    void slotShowIncutBottomRight();
    void slotShowIncutTop();
    void slotShowIncutBottom();
    void slotShowIncutLeft();
    void slotShowIncutRight();

    void slotShowMap( bool fl );
    void slotShowWMO( bool fl );
    void slotShowAuthor( bool fl );
    void slotShowDate( bool fl );
    void slotShowTerm( bool fl );
    void slotShowMashtab( bool fl );
    void slotShowProjection( bool fl );
    void slotShowBaseLayer( bool fl );
    void slotShowLayer( bool fl );

    void slotAboutToShowParentMenu();
};

}
}

#endif
