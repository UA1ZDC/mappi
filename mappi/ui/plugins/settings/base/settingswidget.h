#ifndef METEO_APP_SETTINGS_SETTINGSWIDGET_H
#define METEO_APP_SETTINGS_SETTINGSWIDGET_H

#include <qwidget.h>
#include <qmap.h>

namespace Ui {
  class SettingsWidget;
}

namespace meteo {
namespace app {

class SettingPlugin;
class SettingWidget;

class SettingsWidget : public QWidget
{
  Q_OBJECT

public:
  explicit SettingsWidget(QWidget *parent = 0);
  ~SettingsWidget();
  
  bool addSetting(SettingPlugin* widget);

public slots:
  void slotChanged(bool changed);

private slots:
  void slotSave();
  void slotRestore();

private:
  Ui::SettingsWidget*           ui_;
  QMap<QString, SettingWidget*> widgets_;
};

} // app
} // meteo

#endif // MAPPI_UI_PLUGINS_SETTINGS_SETTINGSWIDGET_H
