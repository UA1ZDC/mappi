#ifndef METEO_MAP_VIEW_ACTIONS_MDIACTION_H
#define METEO_MAP_VIEW_ACTIONS_MDIACTION_H

#include "action.h"
#include <meteo/commons/ui/map/view/actionbutton.h>

#include <QtGui>

namespace meteo {
namespace map {

class MdiAction : public Action
{
  Q_OBJECT
public:
  MdiAction( MapScene* scene );
  virtual ~MdiAction();
  void addActionsToMenu( Menu* menu ) const ;

private slots:
  void insertIntoMdi();

private:
  ActionButton* btnlayers_;
  QGraphicsProxyWidget* btnitem_;

protected:
  void keyPressEvent(QKeyEvent* event);
};

}
}

#endif
