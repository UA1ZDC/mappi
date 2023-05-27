#ifndef METEO_COMMONS_UI_PLUGINS_NABLUDENIA_H
#define METEO_COMMONS_UI_PLUGINS_NABLUDENIA_H

#include <meteo/commons/ui/map/view/actions/action.h>

namespace meteo {

class GeoVector;

namespace map {

class SaveDoc;

class Plugin : public ActionPlugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA( IID "savedoc" FILE "savedoc.json" )
  public:
    Plugin();
    ~Plugin();

    Action* create( MapScene* scene ) const ;
};

class SaveDocAction : public Action
{
  Q_OBJECT
  public:
    SaveDocAction( MapScene* scene );
    ~SaveDocAction();

  private:
    SaveDoc* widget_;

  private slots:
    void slotOpenSaveDoc();
    void slotSaveDocDestroyed( QObject* o );

};

}
}

#endif
