#include "settingsplugin.h"
#include <meteo/commons/ui/isoline/settingswidget.h>

#include <meteo/commons/global/log.h>
#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>

namespace meteo {
namespace map {

/*! \brief kMenuItem - Пункт меню { id: "isolines", title: "Изолинии" } */
static const auto kMenuItem = QPair<QString, QString>("isolines", QObject::tr("Изолинии"));

/*!
 * \brief kMenuPath - Путь к пункту меню { id: "parameters", title: "Параметры"} ->
 * Для добавления вложенности нужно добавить пару в список
*/
static const auto kMenuPath = QList<QPair<QString, QString>>(
{
      QPair<QString, QString>("parameters", QObject::tr("Параметры"))
});

SettingsPlugin::SettingsPlugin()
  : MainWindowPlugin("settingsplugin")
  , wgt_(nullptr)
{
  static const QString isolineSettings = QObject::tr("isolinesettings");
  QAction* action = mainwindow()->findAction(isolineSettings);

  if(nullptr == action) {
    error_log << meteo::msglog::kTargetMenyNotFound.arg(isolineSettings);
  }
  else {
    action->setDisabled(false);
    QObject::connect(action, &QAction::triggered, this, &SettingsPlugin::slotOpenWindow);
  }
}

SettingsPlugin::~SettingsPlugin()
{
}

void SettingsPlugin::slotOpenWindow()
{
  if(nullptr == wgt_) {
    wgt_ = new SettingsWidget(mainwindow());
    mainwindow()->toMdi(wgt_);
    connect(wgt_, &SettingsWidget::destroyed, this, &SettingsPlugin::slotWgtDel);
  }
  else {
    wgt_->show();
    wgt_->setFocus();
  }
}

void SettingsPlugin::slotWgtDel()
{
  wgt_ = nullptr;
}

QWidget* SettingsPlugin::createWidget( app::MainWindow* mw, const QString& option ) const
{
  SettingsWidget* wgt = new SettingsWidget(mw);
  wgt->setCurrentIsoline(option.toInt());
  return wgt;
}

} // map
} // meteo


