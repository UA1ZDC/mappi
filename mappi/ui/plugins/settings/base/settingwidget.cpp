#include "settingwidget.h"

namespace meteo {
namespace app {

SettingWidget::SettingWidget(QWidget* parent)
  : QWidget(parent) { }

SettingWidget::~SettingWidget() { }

QString SettingWidget::title() const { return title_; }

QPixmap SettingWidget::icon()  const { return QPixmap(); }

void SettingWidget::slotLoadConf() { }

void SettingWidget::slotSaveConf() const { }

void SettingWidget::slotChanged() const { Q_EMIT(changed(true)); }

} // app
} // meteo
