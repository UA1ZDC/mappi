#include <cross-commons/app/paths.h>
#include <cross-commons/debug/tlog.h>

#include <qaction.h>

#include <meteo/commons/ui/mainwindow/mainwindow.h>

#include "sessionviewerplugin.h"

namespace meteo {
namespace map {

SessionViewerPlugin::SessionViewerPlugin():
meteo::app::MainWindowPlugin("sessionviewerplugin"),
widget_(NULL)
{
  //QAction* a = mainwindow()->actionFromFile(MnCommon::etcPath() + "/application.menu.conf", "sessionviewer");
  static const QString& actionName = QObject::tr("sessionviewer");
  QAction* a = mainwindow()->findAction(actionName);
  if ( a == NULL ) {
      error_log << QObject::tr("PATH:") << MnCommon::etcPath() + "/application.menu.conf";
      error_log << QObject::tr("Не найден пункт меню с идентификатором sessionviewer");
      return;
  }
  connect( a, SIGNAL( triggered() ), this, SLOT(slotOpenWindow()) );
}

SessionViewerPlugin::~SessionViewerPlugin()
{

}

QWidget* SessionViewerPlugin::createWidget(meteo::app::MainWindow *mw, const QString &option)
{
  Q_UNUSED(option);
  if(nullptr == widget_) {
    widget_ = new SessionViewerWidget(mw);
    widget_->setWindowFlags(Qt::Dialog);
    QObject::connect(widget_, SIGNAL(destroyed(QObject*)), SLOT(slotWidgetDeleted()));
  }
  return widget_;
}

void SessionViewerPlugin::slotOpenWindow()
{
  qApp->setOverrideCursor(Qt::WaitCursor);
  if(widget_ == nullptr ){
    createWidget(mainwindow());
    if (nullptr != mainwindow())
      mainwindow()->toMdi(widget_);
  }
  else
    widget_->show();
  qApp->restoreOverrideCursor();
}

void SessionViewerPlugin::slotWidgetDeleted()
{
  widget_ = NULL;
}

} //map
} //meteo
