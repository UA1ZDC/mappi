#include "schedulesetting.h"
#include "schedulesettingwidget.h"

#include <mappi/ui/plugins/settings/base/settingswidget.h>

namespace mappi {

ScheduleSetting::ScheduleSetting()
  : SettingPlugin("schedulesetting"), widget_(nullptr) { }

ScheduleSetting::~ScheduleSetting() { }

const QString ScheduleSetting::name() const { return name_; }

meteo::app::SettingWidget* ScheduleSetting::createWidget(meteo::app::SettingsWidget *settings)
{
  if(nullptr == widget_){
    widget_ = new ScheduleSettingWidget(settings);
    widget_->setWindowFlags(Qt::Dialog);
    QObject::connect(qobject_cast<QObject*>(widget_), &QObject::destroyed, this, &ScheduleSetting::slotWidgetDelete);
  }
  return widget_;
}

void ScheduleSetting::slotWidgetDelete() { widget_ = nullptr; }

} // mappi
