#ifndef MAPPI_UI_PLUGINS_HISTOGRAMEDITOR_HISTOGRAMPLUGIN_H
#define MAPPI_UI_PLUGINS_HISTOGRAMEDITOR_HISTOGRAMPLUGIN_H

#include "histogrameditor.h"
#include <qwidget.h>

#include <meteo/commons/ui/map/view/actions/action.h>

namespace meteo {

class HistogramPlugin : public map::ActionPlugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA( IID "histogramaction" FILE "histogramplugin.json" )

public:
  HistogramPlugin() : ActionPlugin("histogramaction") {}
  ~HistogramPlugin(){}

  map::Action* create(map::MapScene* mapscene = 0) const;
};

} //meteo
#endif // MAPPI_UI_PLUGINS_HISTOGRAMEDITOR_HISTOGRAMPLUGIN_H
