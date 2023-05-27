#include <cross-commons/app/paths.h>
#include <cross-commons/debug/tlog.h>

#include <qaction.h>

#include "histogramplugin.h"
#include "histogramaction.h"

namespace meteo {

map::Action* HistogramPlugin::create(map::MapScene *mapscene) const
{
  return new HistogramAction(mapscene);
}

}//meteo
