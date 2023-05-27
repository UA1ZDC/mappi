#ifndef AEROPLUGIN_H
#define AEROPLUGIN_H

#include <QtGui>
#include <meteo/commons/ui/mainwindow/mainwindowplugin.h>

namespace meteo{
namespace map{

class AeroPlugin : public app::MainWindowPlugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA( IID "aeroplugin" FILE "aeroplugin.json" )
public:
  AeroPlugin();
  ~AeroPlugin();

private slots:
  void slotOpenAeroDiag();


};

}
}
#endif
