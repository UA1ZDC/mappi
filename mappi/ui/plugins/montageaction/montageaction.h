#ifndef MONTAGEACTION_H
#define MONTAGEACTION_H

#include <qmenu.h>
#include <qpoint.h>

#include <meteo/commons/ui/map/view/actions/action.h>

#include "thematiclistwidget.h"

namespace meteo {
namespace map {

class MontageAction: public Action
{
  Q_OBJECT
public:
  MontageAction(MapScene* scene);
  virtual ~MontageAction();
  void addActionsToMenu(QMenu *menu) const;

  static const QString kName;

private:
  map::MapScene* mapscene_ = nullptr;
  ThematicListWidget* widget_ = nullptr;
  QAction* menuAct_   = nullptr;

private slots:
  void slotShowDialog();
};

} //map
} //meteo

#endif // MONTAGEACTION_H
