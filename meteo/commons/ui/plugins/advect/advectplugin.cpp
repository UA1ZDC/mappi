#include "advectplugin.h"

namespace meteo{
namespace map{

AdvectPlugin::AdvectPlugin() : ActionPlugin("advectaction")
{
}

Action*AdvectPlugin::create(MapScene* sc) const
{
  return new AdvectAction(sc);
}

}
}
