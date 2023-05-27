#include "settingswidget.h"
#include <cross-commons/debug/tlog.h>

namespace meteo {

SettingsWidget::SettingsWidget(QWidget* parent) :
  QWidget(parent),
  changed_(false)
{
}

SettingsWidget::~SettingsWidget()
{
}

bool SettingsWidget::isChanged() const
{
  return changed_;
}

void SettingsWidget::init()
{
}

void SettingsWidget::slotSave()
{
  if ( true == changed_ )  
  save();
}

}
