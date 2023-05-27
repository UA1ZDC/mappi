#ifndef METEO_COMMONS_UI_STATIONPLUGIN_H
#define METEO_COMMONS_UI_STATIONPLUGIN_H

#include <meteo/commons/ui/mainwindow/mainwindowplugin.h>
#include <meteo/commons/ui/station.prognoz/punktwidget.h>

namespace meteo{
namespace map{

class StationPlugin : public app::MainWindowPlugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA( IID "station" FILE "station.json" )
public:
  StationPlugin();
  ~StationPlugin();

private:
  meteo::map::PunktWidget* wgt_;

private slots:
  void slotOpenPunkts();
  void slotWgtDel();
};


}
}
#endif // STATIONPLUGIN_H
