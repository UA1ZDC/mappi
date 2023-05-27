#include "rulerplugin.h"

namespace meteo{
namespace map{

RulerPlugin::RulerPlugin() : ActionPlugin("ruleaction")
{
}

Action*RulerPlugin::create(MapScene* sc) const
{
  return new RulerAction(sc);
}

}
}
