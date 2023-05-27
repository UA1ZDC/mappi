#ifndef METEO_MAP_VIEW_ACTIONS_SCALEACTION_H
#define METEO_MAP_VIEW_ACTIONS_SCALEACTION_H

#include <meteo/commons/ui/map/view/actions/action.h>
#include <meteo/commons/ui/map/view/actionbutton.h>

#include <qpoint.h>
#include <qgraphicsitem.h>
#include <qslider.h>

class QToolButton;
class QEvent;
class QTimer;

namespace meteo {
namespace map {

class MapView;
class WidgetItem;
class ScaleWidget;

class ScaleAction : public Action
{
  Q_OBJECT
  public:
    ScaleAction( MapScene* scene );
    ~ScaleAction();

    void wheelEvent( QWheelEvent* event );
    void keyPressEvent( QKeyEvent* event );

    void setMaxScale(int val) { maxScale_ = val; }
    void setMinScale(int val) { minScale_ = val; }
    void setScaleSeparator(double val) { scaleSeparator_ = val; }

  private:
    QTimer* tm_;
    bool scale(int sc, const QPoint& point );
    ActionButton* plusBtn_;
    ActionButton* minusBtn_;
    QGraphicsProxyWidget* plusItem_;

    double scaleSeparator_;
    int maxScale_;
    int minScale_;

  private slots:
    void slotTimeout();
    void slotZoomIn();
    void slotZoomOut();
};

}
}

#endif
