#include "tlglogplugin.h"

#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>

#include <meteo/commons/ui/mainwindow/mainwindow.h>
#include <meteo/commons/ui/mainwindow/widgethandler.h>
#include <meteo/commons/ui/mainwindow/mdisubwindow.h>

namespace meteo {

static const QString& tlglog = QObject::tr("tlglog");


TlgLogPlugin::TlgLogPlugin()
  : MainWindowPlugin("tlglogplugin")
  , w_(nullptr)
{
  QAction* action = mainwindow()->findAction(tlglog);
  if ( nullptr == action ){
    warning_log.msgBox() << meteo::msglog::kTargetMenyNotFound.arg(tlglog);
  }
  else {
    action->setDisabled(false);
    QObject::connect(action, &QAction::triggered, this, &TlgLogPlugin::slotOpenWidget);
  }
}

TlgLogPlugin::~TlgLogPlugin()
{
  if(nullptr != w_) {
    w_->deleteLater();
    w_ = nullptr;
  }
}

void TlgLogPlugin::slotOpenWidget()
{
  QAction* a = qobject_cast<QAction*>(sender());

  if(nullptr == a ) { return; }

  if(nullptr == w_ ) {
    w_ = new Lgmonitor(WidgetHandler::instance()->mainwindow());
    w_->setWindowFlags(Qt::Dialog);
  }
  w_->show();
  w_->activateWindow();
}

} // meteo

