#include "maskplugin.h"
#include "maskaction.h"

namespace meteo {
namespace map {

meteo::map::Action *meteo::map::MaskPlugin::create(meteo::map::MapScene *sc) const
{
  return new MaskAction(sc);
}

} //map
} //meteo
