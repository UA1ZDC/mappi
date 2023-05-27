#include "automaplugin.h"
#include <qstring.h>
#include <qobject.h>
#include <meteo/commons/global/log.h>
#include <cross-commons/debug/tlog.h>
#include <meteo/commons/ui/mainwindow/mainwindow.h>

namespace meteo {
namespace app {

AutoMapPlugin::AutoMapPlugin()
  : MainWindowPlugin("automap"),
    wgt_(nullptr)
{
  static const QString& autoMapSettings = QObject::tr("automapsettings");
  QAction* a = mainwindow()->findAction(autoMapSettings);
  if(nullptr == a) {
    warning_log << meteo::msglog::kTargetMenyNotFound.arg(autoMapSettings);
  }
  else {
    a->setDisabled(false);
    QObject::connect(a, &QAction::triggered, this, &AutoMapPlugin::slotOpenAutomap);
  }
}

AutoMapPlugin::~AutoMapPlugin()
{
}

void AutoMapPlugin::slotOpenAutomap()
{
  if (nullptr == wgt_) {
    wgt_ = new meteo::map::AutoMap(mainwindow());
    mainwindow()->toMdi(wgt_);
    QObject::connect(wgt_, &map::AutoMap::destroyed, this, &AutoMapPlugin::slotWgtDel);
  }
  else {
    mainwindow()->setActiveWindow(wgt_);
  }
}

void AutoMapPlugin::slotWgtDel()
{
  trc;
  wgt_ = nullptr;
}


} // app
} // meteo
