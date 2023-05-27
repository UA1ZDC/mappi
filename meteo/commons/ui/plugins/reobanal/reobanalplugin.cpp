#include "reobanalplugin.h"

namespace meteo{
namespace map{

  ReobanalPlugin::ReobanalPlugin() : ActionPlugin("reobanalaction")
{
}

Action* ReobanalPlugin::create(MapScene* sc) const
{
  return new ReobanalAction(sc);
}

}
}
