#pragma once

#include <meteo/commons/ui/mainwindow/mainwindowplugin.h>
#include <mappi/ui/antenna/antennawidget.h>


namespace mappi {

class AntennaPlugin :
  public meteo::app::MainWindowPlugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA( IID "antennaplugin" FILE "antennaplugin.json" )
public :
  AntennaPlugin();
  virtual ~AntennaPlugin();

  virtual QWidget* createWidget(meteo::app::MainWindow* mw, const QString& option) const;

private slots:
  void onOpen();
  void onDestroyed();

private:
  antenna::AntennaWidget* widget_;
};

}
