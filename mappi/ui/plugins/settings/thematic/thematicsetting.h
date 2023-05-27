#ifndef MAPPI_UI_PLUGINS_SETTINGS_THEMATIC_THEMATICSETTING_H
#define MAPPI_UI_PLUGINS_SETTINGS_THEMATIC_THEMATICSETTING_H

#include <mappi/ui/plugins/settings/base/settingplugin.h>

namespace mappi {

class ThematicSettingWidget;

class ThematicSetting : public meteo::app::SettingPlugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "thematicsetting" FILE "thematic.json")

public:
  explicit ThematicSetting();
  ~ThematicSetting();

  const QString name() const override;
  meteo::app::SettingWidget* createWidget(meteo::app::SettingsWidget *settings) override;

public slots:
  void slotWidgetDelete();

private:
  ThematicSettingWidget* widget_;
};

} // mappi

#endif // MAPPI_UI_PLUGINS_SETTINGS_THEMATIC_THEMATICSETTING_H
