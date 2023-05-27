#ifndef MASKACTION_H
#define MASKACTION_H

#include <qmenu.h>
#include <qpoint.h>

#include <meteo/commons/ui/map/view/actions/action.h>

#include "transparencyslider.h"

namespace meteo {
namespace map {

class MaskAction: public Action
{
  Q_OBJECT
public:
  MaskAction(MapScene* scene);
  virtual ~MaskAction();
  void addActionsToMenu(QMenu *menu) const;

  static const QString kName;

protected:
  virtual bool eventFilter(QObject* obj, QEvent* event);

private:
  map::MapScene* mapscene_ = nullptr;
  TransparencySlider* widget_ = nullptr;
  QAction* menuAct_   = nullptr;
private slots:
  void slotShowDialog();
};

} //map
} //meteo

#endif // MASKACTION_H
