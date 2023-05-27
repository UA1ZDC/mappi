#include "montageplugin.h"
#include "montageaction.h"

namespace meteo {
namespace map {

Action *MontagePlugin::create(MapScene *sc) const
{
  return new MontageAction(sc);
}

} //map
} //meteo
