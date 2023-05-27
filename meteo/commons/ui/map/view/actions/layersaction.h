#ifndef METEO_MAP_VIEW_ACTIONS_LAYERSACTION_H
#define METEO_MAP_VIEW_ACTIONS_LAYERSACTION_H

#include <meteo/commons/ui/map/view/actions/action.h>
#include <meteo/commons/ui/map/view/actionbutton.h>
#include <qwidget.h>

class QToolButton;
class QEvent;
class QTreeWidgetItem;

namespace Ui
{
class LayersWidget;
class LayerActions;
}

namespace meteo {
namespace map {

class MapView;
class Map;
class Document;
class LayersWidget;

class LayersAction : public Action
{
  Q_OBJECT
  public:
    LayersAction( MapScene* scene );
    virtual ~LayersAction();
    void addActionsToMenu( Menu* menu ) const ;

    void setDeleteVisible( bool fl );

protected:
  virtual bool eventFilter(QObject *obj, QEvent *event);

  private:
    ActionButton* btnlayers_;
    QGraphicsProxyWidget* btnitem_;
    LayersWidget* layerswidget_;
    bool deletevisible_;

  private slots:
    void slotShow( bool fl = true );
    void slotLayerWidgetClosed();
};

}
}

#endif
