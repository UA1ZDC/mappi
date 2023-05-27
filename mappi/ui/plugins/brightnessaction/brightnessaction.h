#ifndef BRIGHTNESSACTION_H
#define BRIGHTNESSACTION_H

#include <qmenu.h>
#include <qpoint.h>

#include <meteo/commons/ui/map/view/actions/action.h>

#include "brightnesscontrastwidget.h"

namespace  meteo {
namespace map {

class BrightnessAction : public Action
{
  Q_OBJECT
public:
  BrightnessAction(MapScene* scene);
  virtual ~BrightnessAction();
  void addActionsToMenu(QMenu *menu) const;

  static const QString kName;

protected:
  virtual bool eventFilter(QObject* obj, QEvent* event);

private:
  map::MapScene* mapscene_ = nullptr;
  QAction* menuAct_   = nullptr;
  BrightnessContrastWidget* widget_ = nullptr;
private slots:
  void slotShowDialog();
};

}
}

#endif // BRIGHTNESSACTION_H
