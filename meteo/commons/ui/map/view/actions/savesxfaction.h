#ifndef METEO_MAP_VIEW_ACTIONS_SAVESXFACTION_H
#define METEO_MAP_VIEW_ACTIONS_SAVESXFACTION_H

#include "action.h"
#include "../actionbutton.h"
#include <qpoint.h>
#include <qgraphicsitem.h>
#include <qslider.h>

class QToolButton;
class QEvent;

namespace meteo {
namespace map {

class SaveSxf;

class SavesxfAction: public Action
{
  Q_OBJECT
  public:
    SavesxfAction( MapScene* scene );
    ~SavesxfAction();

  private:
    SaveSxf* dlg_;

  private slots:
    void slotOpenSaveSxfDialog();
    void slotDlgDestroyed( QObject* o );
};

}
}

#endif
