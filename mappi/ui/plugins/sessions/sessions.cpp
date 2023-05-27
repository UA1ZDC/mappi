#include "sessions.h"
#include "sessionswidget.h"

#include <qaction.h>

#include <cross-commons/app/paths.h>
#include <cross-commons/debug/tlog.h>
#include <meteo/commons/global/log.h>

#include <meteo/commons/ui/mainwindow/mainwindow.h>

namespace mappi {


SessionsPlugin::SessionsPlugin()
  : meteo::app::MainWindowPlugin("sessionsplugin")
{
  //  QAction* a = mainwindow()->actionFromFile(MnCommon::etcPath() + "application.menu.conf", "sessions");
  static const QString& actionName = QObject::tr("sessions");
  QAction* a = mainwindow()->findAction(actionName);
  if (nullptr == a) {
    warning_log.msgBox() << meteo::msglog::kTargetMenyNotFound.arg(actionName);
  } else {
    a->setDisabled(false);
    connect(a, &QAction::triggered, this, &SessionsPlugin::slotOpenWindow);
  }
}

SessionsPlugin::~SessionsPlugin() { }

QWidget* SessionsPlugin::createWidget(meteo::app::MainWindow *mw, const QString& option)
{
  Q_UNUSED(option);
  if(nullptr == widget_) {
    widget_ = new SessionsWidget(mw);
    widget_->setWindowFlags(Qt::Dialog);
    QObject::connect(widget_, SIGNAL(destroyed(QObject*)), SLOT(slotWidgetDeleted()));
  }
  return widget_;
}

void SessionsPlugin::slotOpenWindow()
{
  qApp->setOverrideCursor(Qt::WaitCursor);
  if(widget_ == nullptr) {
    createWidget(mainwindow());
    if (nullptr != mainwindow()) {
      mainwindow()->toMdi(widget_);
    }
  }
  else {
    widget_->show();
    widget_->setFocus();
  }
  qApp->restoreOverrideCursor();
}

void SessionsPlugin::slotWidgetDeleted()
{
   widget_ = nullptr;
}

} // mappi
