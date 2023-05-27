#ifndef VERTICALCUTPLUGIN_H
#define VERTICALCUTPLUGIN_H

#include <QtGui>
#include <meteo/commons/ui/mainwindow/mainwindowplugin.h>


namespace meteo{

class CreateCutWidget;
namespace map{

class VerticalCutPlugin : public app::MainWindowPlugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA( IID "verticalcutplugin" FILE "verticalcutplugin.json" )
  public:
    VerticalCutPlugin();
    ~VerticalCutPlugin();

  private slots:
    void slotOpenCut();

 
};

}
}
#endif

