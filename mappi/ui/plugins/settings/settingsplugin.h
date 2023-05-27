#ifndef METEO_APP_SETTINGS_SETTINGSPLUGIN_H
#define METEO_APP_SETTINGS_SETTINGSPLUGIN_H

#include <meteo/commons/ui/mainwindow/mainwindowplugin.h>

namespace meteo {
namespace  app {

class SettingPlugin;
class SettingsWidget;

class  SettingsPlugin : public MainWindowPlugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "settingsplugin" FILE "settings.json")

public:
  explicit SettingsPlugin();
  ~SettingsPlugin();
  QWidget* createWidget(MainWindow *mw);
  QWidget* createWidget(MainWindow *mw, const QString& option);

private slots:
  void slotOpenWindow();
  void slotWidgetDelete();

private:
  void loadPlugins(const QString& path);

private:
  SettingsWidget* widget_;
  QMap<QString, SettingPlugin*> plugins_;
};


} // app
} // meteo

#endif // MAPPI_UI_PLUGINS_SETTINGS_SETTINGSPLUGIN_H
