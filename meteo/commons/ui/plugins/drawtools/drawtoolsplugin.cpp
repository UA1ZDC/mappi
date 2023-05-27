#include "drawtoolsplugin.h"
#include <meteo/commons/ui/map/view/drawtools/drawtoolsaction.h>

namespace meteo{
namespace map{

DrawToolsPlugin::DrawToolsPlugin() : ActionPlugin("drawtoolsaction")
{
}

Action* DrawToolsPlugin::create(MapScene* sc) const
{
  return new DrawToolsAction(sc);
}

} // map
} // meteo
