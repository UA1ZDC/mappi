#ifndef CITYPLUGIN_H
#define CITYPLUGIN_H

#include <meteo/commons/ui/mainwindow/mainwindowplugin.h>
#include <meteo/commons/ui/city/citypluginhandler.h>

namespace meteo {
namespace map {

class CityPlugin: public app::MainWindowPlugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA( IID "cityplugin" FILE "cityplugin.json" )
public:
  CityPlugin();
  ~CityPlugin();

private:
  meteo::map::CityPluginHandler *handler_;
};

}
}
#endif
