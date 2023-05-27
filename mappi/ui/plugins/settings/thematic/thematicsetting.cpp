#include "thematicsetting.h"
#include "thematicsettingwidget.h"

#include <mappi/ui/plugins/settings/base/settingswidget.h>

namespace mappi {

ThematicSetting::ThematicSetting()
  : SettingPlugin("thematicsetting"), widget_(nullptr) { }

ThematicSetting::~ThematicSetting() { }

const QString ThematicSetting::name() const { return name_; }

meteo::app::SettingWidget* ThematicSetting::createWidget(meteo::app::SettingsWidget *settings)
{
  if(nullptr == widget_) {
    widget_ = new ThematicSettingWidget(settings);
    widget_->slotLoadConf();

    widget_->setWindowFlags(Qt::Dialog);
    QObject::connect(qobject_cast<QObject*>(widget_), &QObject::destroyed, this, &ThematicSetting::slotWidgetDelete);
  }
  return widget_;
}

void ThematicSetting::slotWidgetDelete() {  widget_ = nullptr; }

} // mappi
