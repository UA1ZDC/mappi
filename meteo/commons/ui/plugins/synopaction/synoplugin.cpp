#include "synoplugin.h"

namespace meteo{
namespace map{

SynopPlugin::SynopPlugin() : ActionPlugin("synopaction")
{
}

Action*SynopPlugin::create(MapScene* sc) const
{
  return new SynopAction(sc);
}

}
}
