#include "navigatorplugin.h"
#include "navigatoraction.h"

namespace meteo {
namespace map {

NavigatorPlugin::NavigatorPlugin() :
  ActionPlugin("navigatoraction")
{
}

NavigatorPlugin::~NavigatorPlugin()
{
}

Action* NavigatorPlugin::create(MapScene* scene) const
{
  return new NavigatorAction(scene);
}

} // map
} // meteo
