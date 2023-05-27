#ifndef METEO_COMMONS_UI_PLUGINS_NAVIGATORPLUGIN_H
#define METEO_COMMONS_UI_PLUGINS_NAVIGATORPLUGIN_H

#include <meteo/commons/ui/map/view/actions/action.h>

namespace meteo {
namespace map {

class NavigatorPlugin : public ActionPlugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA( IID "navigator" FILE "navigator.json" )

public:
  NavigatorPlugin();
  ~NavigatorPlugin();

  Action* create(MapScene* scene = 0) const;

};

} // map
} // meteo

#endif // METEO_COMMONS_UI_PLUGINS_NAVIGATORPLUGIN_H
