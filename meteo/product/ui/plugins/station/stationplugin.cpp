#include "stationplugin.h"

#include <qdialog.h>

#include <cross-commons/debug/tlog.h>
#include <meteo/commons/global/log.h>
#include <meteo/commons/ui/mainwindow/widgethandler.h>
#include <meteo/commons/ui/mainwindow/mainwindow.h>

namespace meteo {
namespace map {

static const auto kMenuItem = QPair<QString, QString>("stations", QObject::tr("Пункты прогнозирования"));
static const auto kMenuPath = QList<QPair<QString, QString>>
    ({ QPair<QString, QString>("parameters",  QObject::tr("Параметры")) });

StationPlugin::StationPlugin()
: app::MainWindowPlugin("StationPlugin"),
  wgt_(nullptr)
{
  static const QString& punktSettings = QObject::tr("punktsettings");
  QAction* action = mainwindow()->findAction(punktSettings);
  if ( nullptr == action ) {
    warning_log << meteo::msglog::kTargetMenyNotFound.arg(punktSettings);
  }
  else {
   action->setDisabled(false) ;
   QObject::connect(action, &QAction::triggered, this, &StationPlugin::slotOpenPunkts);
  }
}

StationPlugin::~StationPlugin()
{
}

void StationPlugin::slotOpenPunkts()
{
  if(nullptr == wgt_) {
    wgt_ = new meteo::map::PunktWidget(mainwindow());
    mainwindow()->toMdi(wgt_);
    wgt_->init();
    QObject::connect(wgt_, &PunktWidget::destroyed, this, &StationPlugin::slotWgtDel);
  }
  else {
    wgt_->show();
    wgt_->setFocus();
  }
}

void StationPlugin::slotWgtDel()
{
  wgt_ = nullptr;
}

} // map
} // meteo
