#ifndef METEO_COMMONS_UI_AERO_VERTICALCUT_PLUGIN_CREATECUTACTION_H
#define METEO_COMMONS_UI_AERO_VERTICALCUT_PLUGIN_CREATECUTACTION_H

#include <meteo/commons/ui/map/view/actions/action.h>
#include <meteo/commons/ui/map/view/actionbutton.h>
#include <qwidget.h>

class QToolButton;
class QEvent;
class QTreeWidgetItem;

namespace meteo {
  class CreateCutWidget;
}

namespace meteo {

class MapView;
class Map;
class Document;

class CreateCutAction : public map::Action
{
  Q_OBJECT
  public:
    CreateCutAction( map::MapScene* scene );
    ~CreateCutAction();

    CreateCutWidget* cutwidget() const { return cutwidget_; }
    void setWidget(meteo::CreateCutWidget* widget);

  public slots:
    void slotShow( bool fl );    

  private:
    ActionButton* btncut_;
    QGraphicsProxyWidget* btnitem_;
    meteo::CreateCutWidget* cutwidget_;
    bool deletevisible_;
                 
  private slots:
    void slotLayerWidgetClosed();
};

}


#endif
