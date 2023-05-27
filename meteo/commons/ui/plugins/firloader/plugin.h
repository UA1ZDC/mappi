#ifndef METEO_COMMONS_UI_PLUGINS_FIRLOADER_PLUGIN_H
#define METEO_COMMONS_UI_PLUGINS_FIRLOADER_PLUGIN_H

#include <meteo/commons/ui/map/view/actions/action.h>

namespace meteo {
namespace map {

class FirLoader;

class Plugin : public ActionPlugin
{
  Q_OBJECT
  public:
    Plugin();
    ~Plugin();

    Action* create( MapScene* scene ) const ;
};

class FirAction : public Action
{
  Q_OBJECT
  public:
    FirAction( MapScene* scene );
    ~FirAction();

  private:
    FirLoader* widget_;

  private slots:
    void slotOpenFir();
    void slotFirDestroyed( QObject* o );
};

}
}

#endif
