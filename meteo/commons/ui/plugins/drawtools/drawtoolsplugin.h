#ifndef METEO_COMMONS_UI_PLUGINS_DRAWTOOLS_DRAWTOOLSPLUGIN_H
#define METEO_COMMONS_UI_PLUGINS_DRAWTOOLS_DRAWTOOLSPLUGIN_H

#include <meteo/commons/ui/map/view/actions/action.h>

namespace meteo{
namespace map{

class DrawToolsPlugin : public ActionPlugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA( IID "drawtools" FILE "drawtools.json" )
public:
  DrawToolsPlugin();
  virtual ~DrawToolsPlugin(){}

  Action* create(MapScene* sc = 0) const;
};

} // map
} // meteo

#endif // METEO_COMMONS_UI_PLUGINS_DRAWTOOLS_DRAWTOOLSPLUGIN_H
