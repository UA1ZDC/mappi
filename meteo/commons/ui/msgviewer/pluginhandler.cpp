#include <cross-commons/app/paths.h>
#include <cross-commons/debug/tlog.h>

#include <meteo/commons/ui/mainwindow/mainwindow.h>

#include <qaction.h>

#include "pluginhandler.h"
#include <meteo/commons/ui/msgviewer/msgviewer.h>

static const QString& msgviewer = QObject::tr("watchgmi");

void PluginHandler::slotWidgetDeleted()
{
  widget_ = nullptr;
}

PluginHandler::PluginHandler(meteo::app::MainWindow *mw):
  mw_(mw),
  widget_(nullptr)
{
  QAction* action = mw_->findAction(msgviewer);

  if(nullptr == action){
    warning_log.msgBox() << meteo::msglog::kTargetMenyNotFound.arg(msgviewer);
  }
  else {
    action->setDisabled(false);
    action->setIcon(QIcon(":/meteo/icons/gmi.png"));
    QObject::connect(action, &QAction::triggered, this, &PluginHandler::slotOpenWindow);
  }
}

PluginHandler::~PluginHandler()
{
}

QWidget *PluginHandler::createWidget(meteo::app::MainWindow *mw, const QString &option)
{
  Q_UNUSED(option);
  return new meteo::MsgViewer(mw);
}

void PluginHandler::slotOpenWindow()
{
  qApp->setOverrideCursor(Qt::WaitCursor);

  if (nullptr == widget_ ) {
    widget_ = new meteo::MsgViewer(mw_);
    mw_->toMdi(widget_);
    if ( true == widget_->init()) {
      QObject::connect(widget_, &meteo::MsgViewer::destroyed, this, &PluginHandler::slotWidgetDeleted);
    }
  }
  if (nullptr != widget_ ){
    widget_->show();
    widget_->setFocus();
  }

  qApp->restoreOverrideCursor();
}


