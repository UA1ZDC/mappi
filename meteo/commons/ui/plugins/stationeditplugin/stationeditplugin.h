#ifndef METEO_COMMONS_UI_STATIONEDITPLUGIN_H
#define METEO_COMMONS_UI_STATIONEDITPLUGIN_H

#include <meteo/commons/ui/mainwindow/mainwindowplugin.h>
#include "stationeditwidget.h"

namespace meteo{
namespace map{

class StationEditPlugin : public app::MainWindowPlugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA( IID "stationedit" FILE "stationedit.json" )
public:
  StationEditPlugin();
  ~StationEditPlugin();

private:
  meteo::map::StationEditWidget* wgt_;


private slots:
  void slotOpenEditor();
  void slotWgtDel();

};


}
}
#endif // METEO_COMMONS_UI_STATIONEDITPLUGIN_H
