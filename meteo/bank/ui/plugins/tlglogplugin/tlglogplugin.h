#ifndef METEO_BANK_UI_PLUGINS_TLGMONITOR_H
#define METEO_BANK_UI_PLUGINS_TLGMONITOR_H

#include <qmap.h>

#include <meteo/commons/ui/mainwindow/mainwindowplugin.h>

#include <meteo/commons/ui/tlglog/lgmonitor.h>

namespace meteo {

class TlgLogPlugin : public app::MainWindowPlugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA( IID "tlglog" FILE "tlglog.json" )
public:
  TlgLogPlugin();
  virtual ~TlgLogPlugin();

public slots:
  void slotOpenWidget();
  void slotWidgetClosed();

private:
  Lgmonitor* w_;
};

}

#endif

