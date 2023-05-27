#ifndef METEO_MAP_VIEW_ACTIONS_SAVEBMPACTION_H
#define METEO_MAP_VIEW_ACTIONS_SAVEBMPACTION_H

#include "action.h"
#include "../actionbutton.h"
#include <qpoint.h>
#include <qgraphicsitem.h>
#include <qslider.h>

class QToolButton;
class QEvent;

namespace meteo {
namespace map {

class SaveBitmap;

class SavebmpAction: public Action
{
  Q_OBJECT
  public:
    SavebmpAction( MapScene* scene );
    ~SavebmpAction();

  private:
    SaveBitmap* dlg_;

  private slots:
    void slotOpenSaveBmpDialog();
    void slotDlgDestroyed( QObject* o );
};

}
}

#endif
