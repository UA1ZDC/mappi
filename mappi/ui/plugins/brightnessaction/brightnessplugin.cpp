#include "brightnessplugin.h"
#include "brightnessaction.h"

namespace meteo {
namespace map {

meteo::map::Action *meteo::map::BrightnessPlugin::create(meteo::map::MapScene *sc) const
{
  return new BrightnessAction(sc);
}

}
}
