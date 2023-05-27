#ifndef OCEANPLUGIN_H
#define OCEANPLUGIN_H

#include <QtGui>
#include <meteo/commons/ui/mainwindow/mainwindowplugin.h>

namespace meteo{
namespace map{

class OceanPlugin : public app::MainWindowPlugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA( IID "oceanplugin" FILE "oceanplugin.json" )
public:
  OceanPlugin();
  ~OceanPlugin();

public slots:
  void slotOpenOceanDiag();

private:

};

}
}
#endif
