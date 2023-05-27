#ifndef METEO_MAP_VIEW_ACTIONS_OPENSXFDOCACTION_H
#define METEO_MAP_VIEW_ACTIONS_OPENSXFDOCACTION_H

#include "action.h"
#include "../actionbutton.h"
#include <qpoint.h>
#include <qgraphicsitem.h>
#include <qslider.h>

class QToolButton;
class QEvent;

namespace meteo {
namespace map {

class ImportSxf;

class OpenSxfaction: public Action
{
  Q_OBJECT
  public:
    OpenSxfaction( MapScene* scene );
    ~OpenSxfaction();

  private:
    ImportSxf* dlg_;

  private slots:
    void slotOpenSxfDialog();
    void slotDlgDestroyed( QObject* o );
};

}
}

#endif
