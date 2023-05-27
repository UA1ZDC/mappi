#include "stationeditplugin.h"

#include <meteo/commons/ui/mainwindow/widgethandler.h>
#include <meteo/commons/ui/mainwindow/mainwindow.h>

namespace meteo {
namespace map {

static const auto kMenuItem = QPair<QString, QString>("stationedit", QObject::tr("Станции"));
static const auto kMenuPath = QList<QPair<QString, QString>>
    ({ QPair<QString, QString>("parameters",  QObject::tr("Параметры")) });

StationEditPlugin::StationEditPlugin()
: app::MainWindowPlugin("StationEditPlugin"),
  wgt_(nullptr)
{
  QAction* a = mainwindow()->addActionToMenu(kMenuItem, kMenuPath);
  if(nullptr != a) {
    a->setDisabled(false);
    QObject::connect(a, &QAction::triggered, this, &StationEditPlugin::slotOpenEditor);
  }
}

StationEditPlugin::~StationEditPlugin()
{
}

void StationEditPlugin::slotOpenEditor()
{
  if(nullptr == wgt_) {
    wgt_ = new meteo::map::StationEditWidget(mainwindow());
    mainwindow()->toMdi(wgt_);
    wgt_->init();
    QObject::connect(wgt_, &StationEditWidget::destroyed, this, &StationEditPlugin::slotWgtDel);
  }
  else {
    mainwindow()->setActiveWindow(wgt_);
  }
}

void StationEditPlugin::slotWgtDel()
{
  wgt_ = nullptr;
}

} // map
} // meteo
