#ifndef FOCUSACTION_H
#define FOCUSACTION_H

#include "action.h"

#include <qwidget.h>

class ActionButton;

namespace meteo {
namespace map {

class MapView;
class WidgetItem;

class FocusAction : public Action
{
  Q_OBJECT
  public:
    FocusAction( MapScene* scene );
    ~FocusAction();

     void mouseReleaseEvent( QMouseEvent* e );
     void keyPressEvent( QKeyEvent* event );

  private slots:
    void slotActivate(bool act);

  private:
    ActionButton* focusBtn_;
    QGraphicsProxyWidget* focusItem_;
    QCursor oldcursor_;
    bool active_;
    bool started_;
  
    void setRotateAngle(double meridian);
};

}
}
#endif // ROTATEACTION_H
