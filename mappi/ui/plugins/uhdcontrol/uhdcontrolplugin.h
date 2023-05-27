#ifndef MAPPI_UI_PLUGINS_UHDCONTROL_UHDCONTROLPLUGIN_H
#define MAPPI_UI_PLUGINS_UHDCONTROL_UHDCONTROLPLUGIN_H

#include "uhdcontrolplugin.h"
#include "uhdcontrol.h"
#include "layerspectr.h"
#include "ramkaspectr.h"
#include "axisspectr.h"
#include "funcs.h"
#include "uhdwindow.h"

#include <meteo/commons/ui/mainwindow/mainwindowplugin.h>

class UhdControlPlugin : public meteo::app::MainWindowPlugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA( IID "uhdcontrol" FILE "uhdcontrol.json" )
public:
  UhdControlPlugin();
  virtual ~UhdControlPlugin();
  UhdWindow* getMapWindow();

public slots:
  UhdWindow* slotCreateWindow();

private slots:
  void slotWidgetDeleted();

private:
  UhdWindow* mapWindow_ = 0;
};


#endif // MAPPI_UI_PLUGINS_UHDCONTROL_UHDCONTROLPLUGIN_H
