#ifndef PLOTTINGPLUGIN_H
#define PLOTTINGPLUGIN_H

#include <QtGui>
#include <meteo/commons/ui/map/view/actions/action.h>

namespace meteo {

class GeoVector;

namespace map {

class PlottingPlugin : public ActionPlugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA( IID "plottingplug" FILE "plottingplug.json" )
  public:
    PlottingPlugin();
    ~PlottingPlugin();

    Action* create( MapScene* scene ) const ;
};

class PlottingWidget;

class PlottingAction : public Action
{
  Q_OBJECT
  public: 
    PlottingAction( MapScene* scene );
    ~PlottingAction();

  private:
    PlottingWidget* plotwidget_;

  private slots:
    void slotOpenPlottingWidget();
};

}
}
#endif // PLOTTINGPLUGIN_H

