#ifndef METEO_BANK_UI_PLUGINS_MSGVIEWERPLUGIN_MSGVIEWERPLUGIN_H
#define METEO_BANK_UI_PLUGINS_MSGVIEWERPLUGIN_MSGVIEWERPLUGIN_H

#include <meteo/commons/ui/mainwindow/mainwindowplugin.h>
#include <meteo/commons/ui/msgviewer/pluginhandler.h>

class MsgViewer;

class MsgViewerPlugin: public meteo::app::MainWindowPlugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA( IID "msgviewerplugin" FILE "msgviewerplugin.json" )

public:
  MsgViewerPlugin();
  virtual ~MsgViewerPlugin() override;

  QWidget* createWidget( meteo::app::MainWindow* mw, const QString& option );
private:
  PluginHandler* handler_;

};

#endif // MSGVIEWERPLUGIN_H
