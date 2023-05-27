#ifndef MAPPI_UI_PLUGINS_SETTINGS_RECEIVER_RECEIVERSETTINGSWIDGETS_H
#define MAPPI_UI_PLUGINS_SETTINGS_RECEIVER_RECEIVERSETTINGSWIDGETS_H

#include <mappi/ui/plugins/settings/base/settingwidget.h>

namespace Ui {
  class ReceiverSettingWidget;
}

namespace mappi {

class ReceiverSettingWidget : public meteo::app::SettingWidget
{
  Q_OBJECT

public:
  ReceiverSettingWidget(QWidget *parent = nullptr);
  ~ReceiverSettingWidget();

public slots:
  void slotLoadConf();
  void slotSaveConf() const;

private:
  Ui::ReceiverSettingWidget *ui_;
};

} // mappi

#endif // MAPPI_UI_PLUGINS_SETTINGS_RECEIVER_RECEIVERSETTINGSWIDGETS_H
