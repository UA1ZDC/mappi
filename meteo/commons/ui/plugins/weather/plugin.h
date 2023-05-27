#ifndef METEO_COMMONS_MAP_WIDGETS_WEATHER_PLUGIN_H
#define METEO_COMMONS_MAP_WIDGETS_WEATHER_PLUGIN_H

#include <meteo/commons/ui/mainwindow/mainwindowplugin.h>
#include <meteo/commons/ui/map/view/mapwindow.h>
#include <meteo/commons/proto/weather.pb.h>

namespace meteo {
namespace map {
namespace weather {

class TermSelect;

class Plugin : public app::MainWindowPlugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA( IID "weatherplug" FILE "weatherplug.json" )
  public:
    Plugin();
    ~Plugin();

  private:
    MapWindow* createMapWindow();

  private slots:
    void slotOpenTermSelect();
    void slotOpenEmptyMap();
    void slotOpenBlankParam();
    void slotOpenMapFile();
};

class MenuSlot : public QObject
{
  Q_OBJECT
  public:
    MenuSlot( MapWindow* window );
    ~MenuSlot();

  public slots:
    void slotOpenTermSelect();

  private:
    MapWindow* window_;
    TermSelect* term_;
};

}
}
}

#endif
