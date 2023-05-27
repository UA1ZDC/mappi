#ifndef MAPPI_UI_PLUGIN_SESSION_SESSION_H
#define MAPPI_UI_PLUGIN_SESSION_SESSION_H

#include <meteo/commons/ui/mainwindow/mainwindowplugin.h>

namespace mappi {

class SessionsWidget;

class SessionsPlugin : public meteo::app::MainWindowPlugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "sessionsplugin" FILE "sessions.json" )

public:
  SessionsPlugin();
  ~SessionsPlugin();
  QWidget* createWidget(meteo::app::MainWindow* mw, const QString& option = "");

private slots:
  void slotOpenWindow();
  void slotWidgetDeleted();

private:
  SessionsWidget* widget_ = nullptr;
};

} //mappi

#endif // MAPPI_UI_PLUGIN_SESSION_SESSION_H
