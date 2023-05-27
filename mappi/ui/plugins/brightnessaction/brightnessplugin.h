#ifndef BRIGHTNESSPLUGIN_H
#define BRIGHTNESSPLUGIN_H

#include <meteo/commons/ui/map/view/actions/action.h>

namespace meteo {
namespace map {

class BrightnessPlugin : public ActionPlugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA( IID "brightnessaction" FILE "brightnessplugin.json" )
public:
  BrightnessPlugin() : ActionPlugin("brightnessaction") {}
  virtual ~BrightnessPlugin () {}

  Action* create( MapScene* sc = 0) const;
};

} //map
} //meteo

#endif // BRIGHTNESSPLUGIN_H
