#ifndef METEO_COMMONS_UI_PLUGINS_NABLUDENIA_PLUGIN_H
#define METEO_COMMONS_UI_PLUGINS_NABLUDENIA_PLUGIN_H

#include <meteo/commons/ui/map/view/actions/action.h>

namespace meteo {
namespace map {

class Plugin : public ActionPlugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "nabludenia" FILE "nabludenia.json")
  public:
    Plugin();
    ~Plugin();

    Action* create(MapScene* scene) const;
};

class Nabludenia;

class NabludeniaAction : public Action
{
  Q_OBJECT
  public:
    NabludeniaAction(MapScene* scene);
    ~NabludeniaAction();

  private:
    Nabludenia* widget_;

  private slots:
    void slotOpenNabludenia();
    void slotNabludeniaDestroyed(QObject*);
};

}
}

#endif
