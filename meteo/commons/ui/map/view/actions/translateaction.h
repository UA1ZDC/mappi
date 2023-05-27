#ifndef METEO_MAP_VIEW_ACTIONS_TRANSLATEACTION_H
#define METEO_MAP_VIEW_ACTIONS_TRANSLATEACTION_H

#include "action.h"
#include <qmenu.h>
#include <qpoint.h>

namespace meteo {
namespace map {

class MapView;

class TranslateAction : public Action
{
  Q_OBJECT
  public:
    TranslateAction( MapScene* scene );
    ~TranslateAction();

    void mousePressEvent( QMouseEvent* e );
    void mouseReleaseEvent( QMouseEvent* e );
    void mouseMoveEvent( QMouseEvent* e );

  private:
    bool sceneGrabbed_;
    QPoint oldpos_;
    QPoint newpos_;
};

}
}

#endif
