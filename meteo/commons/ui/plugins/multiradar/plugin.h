#ifndef METEO_COMMONS_UI_PLUGINS_MULTIRADAR_PLUGIN_H
#define METEO_COMMONS_UI_PLUGINS_MULTIRADAR_PLUGIN_H

#include <meteo/commons/ui/map/view/actions/action.h>

namespace meteo {
namespace map {

class MultiRadar;

class Plugin : public ActionPlugin
{
  Q_OBJECT
  public:
    Plugin();
    ~Plugin();

    Action* create( MapScene* scene ) const ;
};

class MultiRadarAction : public Action
{
  Q_OBJECT
  public:
    MultiRadarAction( MapScene* scene );
    ~MultiRadarAction();

    void addActionsToMenu( Menu* menu ) const ;

    void keyReleaseEvent( QKeyEvent* event );

  private:
    QAction* multiaction_;
    MultiRadar* widget_;

  private slots:
    void slotOpenMultiRadar();
    void slotMultiRadarDestroyed( QObject* o );

};

}
}

#endif
