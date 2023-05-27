#ifndef METEO_MAP_VIEW_ACTIONS_PRINTDOCACTION_H
#define METEO_MAP_VIEW_ACTIONS_PRINTDOCACTION_H

#include "action.h"
#include "../actionbutton.h"
#include <qpoint.h>
#include <qgraphicsitem.h>
#include <qslider.h>

class QPrintDialog;
class QToolButton;
class QEvent;

namespace meteo {
namespace map {

class PrintDoc;

class PrintdocAction: public Action
{
  Q_OBJECT
  public:
    PrintdocAction( MapScene* scene );
    ~PrintdocAction();

  private:
  QPrintDialog* dlg_;

  private slots:
    void slotOpenPrintDialog();
    void slotDlgDestroyed( QObject* o );
};

}
}

#endif
