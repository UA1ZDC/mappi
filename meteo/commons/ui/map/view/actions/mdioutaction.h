#ifndef METEO_MAP_VIEW_ACTIONS_MDIOUTACTION_H
#define METEO_MAP_VIEW_ACTIONS_MDIOUTACTION_H

#include "action.h"
#include <meteo/commons/ui/map/view/actionbutton.h>

#include <QtGui>

namespace meteo {
namespace map {

class MdiOutAction : public Action
{
  Q_OBJECT
public:
  MdiOutAction( MapScene* scene );
  virtual ~MdiOutAction();
  void addActionsToMenu( Menu* menu ) const ;

private slots:
  void extractFromMdi();

private:
  ActionButton* btnlayers_;
  QGraphicsProxyWidget* btnitem_;

protected:
  void keyPressEvent(QKeyEvent* event);
};

}
}

#endif
