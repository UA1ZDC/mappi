#include "plugin.h"
#include <cross-commons/app/paths.h>
#include <cross-commons/debug/tlog.h>
#include <meteo/commons/ui/mainwindow/mainwindow.h>
#include <qaction.h>


namespace mappi {

AntennaPlugin::AntennaPlugin():
    meteo::app::MainWindowPlugin("antennaplugin"),
  widget_(nullptr)
{
  QAction* action = mainwindow()->findAction(QObject::tr("antenna_test"));
  if (action == nullptr) {
    error_log << QObject::tr("PATH:") << MnCommon::etcPath() + "/application.menu.conf";
    error_log << QObject::tr("Не найден пункт меню с идентификатором antenna_test");
    return;
  }

  QObject::connect(action, &QAction::triggered, this, &AntennaPlugin::onOpen);
}

AntennaPlugin::~AntennaPlugin()
{
  delete widget_;
  widget_ = nullptr;
}

QWidget* AntennaPlugin::createWidget(meteo::app::MainWindow* mw, const QString& option) const
{
  Q_UNUSED(option);
  return new antenna::AntennaWidget(mw);
}

void AntennaPlugin::onOpen()
{
  qApp->setOverrideCursor(Qt::WaitCursor);

  if (widget_ == nullptr) {
    widget_ = new antenna::AntennaWidget(mainwindow());
    widget_->setWindowFlags(Qt::Dialog);
    widget_->setAttribute(Qt::WA_DeleteOnClose);
    mainwindow()->toMdi(widget_);

    QObject::connect(widget_, &antenna::AntennaWidget::destroyed, this, &AntennaPlugin::onDestroyed);
  }

  widget_->show();
  widget_->setFocus();

  qApp->restoreOverrideCursor();
}

void AntennaPlugin::onDestroyed()
{
  widget_ = NULL;
}

}
