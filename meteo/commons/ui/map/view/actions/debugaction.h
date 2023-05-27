#ifndef METEO_MAP_VIEW_ACTIONS_DEBUGACTION_H
#define METEO_MAP_VIEW_ACTIONS_DEBUGACTION_H

#include "action.h"
#include "../actionbutton.h"
#include <qpoint.h>

class QDialog;
class QToolButton;
class QGraphicsProxyWidget;
class QAbstractButton;

namespace Ui {
class Debug;
}

namespace meteo {
namespace map {

class MapView;
class WidgetItem;

enum SearchType
{
  kNoSearch     = 0,
  kSearchUnder  = 1,
  kSearchNear   = 2
};

class DebugAction : public Action
{
  Q_OBJECT
  public:
    DebugAction( MapScene* scene );
    ~DebugAction();

    void mousePressEvent( QMouseEvent* e );
    void mouseMoveEvent( QMouseEvent* e );

    void addActionsToMenu( Menu* menu ) const ;

  private:
    ActionButton* btndebug_;
    WidgetItem* widgetdebug_;
    Ui::Debug* ui_;
    QList<QGraphicsItem*> drawed_;
    SearchType searchtype_;
    QAction* showact_;

  private slots:
    void slotActionChanged( QAbstractButton* btn );
    void slotShowToggle();
};

}
}

#endif
