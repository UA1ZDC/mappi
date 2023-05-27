#include "transparencyplugin.h"
#include "transparencyaction.h"

namespace meteo {
namespace map {

meteo::map::Action *meteo::map::TransparencyPlugin::create(meteo::map::MapScene *sc) const
{
  return new TransparencyAction(sc);
}

}
}
