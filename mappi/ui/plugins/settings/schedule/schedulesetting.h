#ifndef MAPPI_UI_PLUGINS_SETTINGS_SCHEDULE_SCHEDULESETTING_H
#define MAPPI_UI_PLUGINS_SETTINGS_SCHEDULE_SCHEDULESETTING_H

#include <mappi/ui/plugins/settings/base/settingplugin.h>

namespace mappi {

class ScheduleSettingWidget;

class ScheduleSetting : public meteo::app::SettingPlugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "schedulesetting" FILE "schedule.json")

public:
  explicit ScheduleSetting();
  ~ScheduleSetting();

  const QString name() const override;
  meteo::app::SettingWidget* createWidget(meteo::app::SettingsWidget *settings) override;

public slots:
  void slotWidgetDelete();

private:
  ScheduleSettingWidget* widget_;
};

} // mappi

#endif // MAPPI_UI_PLUGINS_SETTINGS_SCHEDULE_SCHEDULESETTING_H
