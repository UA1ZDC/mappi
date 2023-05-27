#ifndef METEO_COMMONS_UI_MAP_VIEW_ACTIONS_COORDSACTION_H
#define METEO_COMMONS_UI_MAP_VIEW_ACTIONS_COORDSACTION_H

#include "action.h"

#include <qwidget.h>

class ActionButton;

namespace meteo {
namespace map {

class MapView;
class WidgetItem;

class CoordsAction : public Action
{
  Q_OBJECT
public:
  CoordsAction( MapScene* scene );
  ~CoordsAction();

  void mousePressEvent( QMouseEvent* e );
  void mouseReleaseEvent( QMouseEvent* e );
  void mouseClickEvent(QMouseEvent* e);

private:
  bool isWidgetUnderMouse(const QPointF& screenPos) const;
};

}
}

#endif
