#include "settingswidget.h"
#include "ui_settingswidget.h"
#include "settingplugin.h"
#include "settingwidget.h"

#include <functional>

namespace meteo {
namespace app {

SettingsWidget::SettingsWidget(QWidget *parent) :
  QWidget(parent),
  ui_(new Ui::SettingsWidget)
{
  ui_->setupUi(this);

  QObject::connect(ui_->settingsList, &QListWidget::currentRowChanged, [&](int row) {
   ui_->settingsContent->setCurrentIndex(row);
  });

  QObject::connect(ui_->saveBtn,      &QPushButton::clicked, this, &SettingsWidget::slotSave);
  QObject::connect(ui_->restoreBtn,   &QPushButton::clicked, this, &SettingsWidget::slotRestore);
  QObject::connect(ui_->closeBtn,     &QPushButton::clicked, this, &SettingsWidget::close);
}

SettingsWidget::~SettingsWidget() { delete ui_; }

bool SettingsWidget::addSetting(SettingPlugin* plugin)
{
  if(widgets_.contains(plugin->name())) return false;
  auto widget = plugin->createWidget(this);
  widgets_.insert(plugin->name(), widget);
  ui_->settingsContent->addWidget(widget);
  QString title = widget->title();
  ui_->settingsList->insertItem(ui_->settingsList->count(), title);
  if(false == widget->icon().isNull()) {
    ui_->settingsList->item(ui_->settingsList->count())->setIcon(QIcon(widget->icon()));
  }
  QObject::connect(widget, &SettingWidget::changed, this, &SettingsWidget::slotChanged);
  return true;
}

void SettingsWidget::slotChanged(bool changed)
{
  QString name = dynamic_cast<SettingWidget*>(sender())->title();
  for(auto row = 0, sz = ui_->settingsList->count(); row < sz; ++row)
  {
    if(ui_->settingsList->item(row)->text().contains(name)) {
      ui_->settingsList->item(row)->setText(changed ? name + '*' : name);
      break;
    }
  }
}

void SettingsWidget::slotSave()
{
  for(auto widget : widgets_) {
    widget->slotSaveConf();
  }
}

void SettingsWidget::slotRestore()
{
  for(auto widget : widgets_) {
    widget->slotLoadConf();
  }
}

} // app
} // meteo
