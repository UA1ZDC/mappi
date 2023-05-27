#ifndef METEO_COMMONS_UI_PLUGINS_ADVECT_ADVECTPLUGIN_H
#define METEO_COMMONS_UI_PLUGINS_ADVECT_ADVECTPLUGIN_H

#include <meteo/commons/ui/map/view/actions/action.h>
#include "advectaction.h"

namespace meteo{
namespace map{

class AdvectPlugin : public ActionPlugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA( IID "advect" FILE "advect.json" )
public:
  AdvectPlugin();
  ~AdvectPlugin(){}
  Action* create( MapScene* sc = nullptr ) const;
};

}
}
#endif //METEO_COMMONS_UI_PLUGINS_ADVECT_ADVECTPLUGIN_H
