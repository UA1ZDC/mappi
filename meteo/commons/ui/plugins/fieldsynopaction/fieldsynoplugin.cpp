#include "fieldsynoplugin.h"
#include <meteo/commons/ui/mainwindow/widgethandler.h>

namespace meteo{
namespace map{

FieldSynoPlugin::FieldSynoPlugin() : ActionPlugin("fieldsynopaction")
{
}

Action*FieldSynoPlugin::create(MapScene* sc) const
{
  return new FieldSynopAction(sc);
}

Q_EXPORT_PLUGIN2( fieldsynplug, FieldSynoPlugin )

}
}
