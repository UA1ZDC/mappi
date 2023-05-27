#ifndef MAPPI_UI_PLUGINS_SETTINGS_RECEIVER_RECEIVERSETTING_H
#define MAPPI_UI_PLUGINS_SETTINGS_RECEIVER_RECEIVERSETTING_H

#include <mappi/ui/plugins/settings/base/settingplugin.h>

namespace mappi {

class ReceiverSettingWidget;

class ReceiverSetting : public meteo::app::SettingPlugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "receiversetting" FILE "receiver.json")

public:
  explicit ReceiverSetting();
  ~ReceiverSetting();

  const QString name() const override;
  meteo::app::SettingWidget* createWidget(meteo::app::SettingsWidget *settings) override;

public slots:
  void slotWidgetDelete();

private:
  ReceiverSettingWidget* widget_;
};

} // mappi

#endif // MAPPI_UI_PLUGINS_SETTINGS_RECEIVER_RECEIVERSETTING_H
