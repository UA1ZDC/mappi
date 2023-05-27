#ifndef TRANSPARENCYACTION_H
#define TRANSPARENCYACTION_H

#include <qmenu.h>
#include <qpoint.h>

#include <meteo/commons/ui/map/view/actions/action.h>

#include "transparencywidget.h"

namespace  meteo {
namespace map {

class TransparencyAction : public Action
{
  Q_OBJECT
public:
  TransparencyAction(MapScene* scene);
  virtual ~TransparencyAction();
  void addActionsToMenu(QMenu *menu) const;

  static const QString kName;

protected:
  virtual bool eventFilter(QObject* obj, QEvent* event);

private:
  map::MapScene* mapscene_ = nullptr;
  QAction* menuAct_   = nullptr;
  TransparencyWidget* widget_ = nullptr;

private slots:
  void slotShowDialog();
};

}
}

#endif // TRANSPARENCYACTION_H
