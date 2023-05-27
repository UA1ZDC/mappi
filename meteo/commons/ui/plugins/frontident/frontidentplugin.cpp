#include "frontidentplugin.h"

namespace meteo{
namespace map{

  FrontIdentPlugin::FrontIdentPlugin() : ActionPlugin("frontidentaction")
{
}

Action*FrontIdentPlugin::create(MapScene* sc) const
{
  return new FrontIdentAction(sc);
}

}
}
