#include "journalplugin.h"

#include <qaction.h>
#include <qlist.h>

#include <cross-commons/app/paths.h>
#include <cross-commons/debug/tlog.h>
#include <meteo/commons/global/log.h>

#include <meteo/commons/ui/mainwindow/widgethandler.h>
#include <meteo/commons/ui/mainwindow/mainwindow.h>

#include "journalwidgetdatamodel.h"

namespace meteo{

JournalPlugin::JournalPlugin()
: app::MainWindowPlugin("journalplugin"),
  widget_(nullptr)
{
  static const QString& journal = QObject::tr("journal");
  QAction* action = mainwindow()->findAction(journal);
  if(nullptr == action) {
    warning_log << meteo::msglog::kTargetMenyNotFound.arg(journal);
  }
  else {
    action->setDisabled(false);
    QObject::connect(action, &QAction::triggered, this, &JournalPlugin::slotOpenJournal);
  }
}

JournalPlugin::~JournalPlugin()
{
}

void JournalPlugin::slotWidgetDestroyed()
{
  this->widget_ = nullptr;
}

QWidget* JournalPlugin::slotOpenJournal()
{
  if( nullptr == widget_) {
    auto model = new JournalWidgetDataModel(100);
    widget_ = new CustomViewer(nullptr, model);
    connect(widget_, SIGNAL(destroyed()),  SLOT(slotWidgetDestroyed()));
    widget_->init();    
    mainwindow()->toMdi(widget_);
    widget_->show();
    widget_->setFocus();
    model->forceUpdate();
  }
  else {
    widget_->show();
    widget_->setFocus();
  }
  return widget_;
}



}
