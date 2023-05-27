#include "plugin.h"
#include "pointvalueaction.h"

namespace meteo{
namespace map{

Action* PointValuePlugin::create(MapScene* sc) const
{
  return new PointValueAction(sc);

}

} // map
} // meteo
