#include "receiversetting.h"
#include "receiversettingwidget.h"

#include <mappi/ui/plugins/settings/base/settingswidget.h>

namespace mappi {

ReceiverSetting::ReceiverSetting()
  : SettingPlugin("receiversetting"), widget_(nullptr) { }

ReceiverSetting::~ReceiverSetting() { }

const QString ReceiverSetting::name() const { return name_; }

meteo::app::SettingWidget* ReceiverSetting::createWidget(meteo::app::SettingsWidget *settings)
{
  if(nullptr == widget_) {
    widget_ = new ReceiverSettingWidget(settings);
    widget_->setWindowFlags(Qt::Dialog);
    QObject::connect(qobject_cast<QObject*>(widget_), &QObject::destroyed, this, &ReceiverSetting::slotWidgetDelete);
  }
  return widget_;
}

void ReceiverSetting::slotWidgetDelete() { widget_ = nullptr; }

} // mappi
