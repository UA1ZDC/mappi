#ifndef METEO_COMMONS_UI_PLUGINS_RADARMAP_PLUGIN_H
#define METEO_COMMONS_UI_PLUGINS_RADARMAP_PLUGIN_H

#include <meteo/commons/ui/map/view/actions/action.h>

namespace meteo {
namespace map {

class RadarMap;

class Plugin : public ActionPlugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA( IID "radarmap" FILE "radarmap.json" )
  public:
    Plugin();
    ~Plugin();

    Action* create( MapScene* scene ) const ;
};

class RadarAction : public Action
{
  Q_OBJECT
  public:
    RadarAction( MapScene* scene );
    ~RadarAction();

  private:
    RadarMap* widget_;

  private slots:
    void slotOpenRadar();
    void slotRadarDestroyed( QObject* o );
};

}
}

#endif
