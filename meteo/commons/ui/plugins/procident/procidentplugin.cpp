#include "procidentplugin.h"

namespace meteo{
namespace map{

ProcIdentPlugin::ProcIdentPlugin()
  : ActionPlugin("procidentaction")
{
}

ProcIdentPlugin::~ProcIdentPlugin()
{
}

Action* ProcIdentPlugin::create(MapScene* sc) const
{
  return new ProcIdentAction(sc);
}

//Q_PLUGIN_METADATA( procidentplug ProcIdentPlugin )

}
}
