#ifndef MAPPI_UI_PLUGINS_POINTVALUE_PLUGIN_H
#define MAPPI_UI_PLUGINS_POINTVALUE_PLUGIN_H

#include <meteo/commons/ui/map/view/actions/action.h>

namespace meteo {
namespace map {

class PointValuePlugin : public ActionPlugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA( IID "pointvalueaction" FILE "pointvalueplugin.json" )

public:
  PointValuePlugin() : ActionPlugin("pointvalueaction") {}
  virtual ~PointValuePlugin(){}

  Action* create( MapScene* sc = 0 ) const;
};

} // map
} // meteo

#endif // MAPPI_UI_PLUGINS_POINTVALUE_PLUGIN_H
