#include "citypluginhandler.h"
#include "citywidget.h"
#include <meteo/commons/ui/mainwindow/mainwindowplugin.h>
#include <meteo/commons/ui/map/loader.h>

#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>

#include <fstream>

#include <meteo/commons/ui/map/geopixmap.h>
#include <meteo/commons/ui/map/geotext.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/ui/map/document.h>
#include <meteo/commons/ui/map/layer.h>
#include <meteo/commons/proto/map_city.pb.h>
#include "meteo/commons/ui/map/commongrid.h"
#include <meteo/commons/settings/settings.h>
#include <meteo/commons/proto/locsettings.pb.h>

#include <qaction.h>

namespace meteo {
namespace map {

CityPluginHandler::CityPluginHandler(app::MainWindow* mw):
  mw_(mw),
  wgt_(nullptr)
{
  meteo::map::Loader::instance()->registerCitiesHandler(meteo::global::kCitiesLoaderCommon, &meteo::map::loadCitiesLayer);//TODO?
  static const QString& citySettings = QObject::tr("citysettings");
  QAction* action = mw_->findAction(citySettings);
  if(nullptr == action) {
    warning_log << meteo::msglog::kTargetMenyNotFound.arg(citySettings);
  }
  else {
    action->setDisabled(false);
    action->setIcon(QIcon(":/meteo/icons/map/city.png"));
    QObject::connect(action, &QAction::triggered, this, &CityPluginHandler::slotOpenWindow);
  }
}


CityPluginHandler::~CityPluginHandler()
{
}

QWidget* CityPluginHandler::createWidget(app::MainWindow* mw, const QString& option ) const
{
    Q_UNUSED(option);
    return new CityWidget(mw);
}

void CityPluginHandler::slotOpenWindow()
{
  if(nullptr == wgt_) {
    wgt_ = new CityWidget( mw_ );
    mw_->toMdi(wgt_);
    connect(wgt_, &CityWidget::destroyed, this, &CityPluginHandler::slotWgtDel);
  }
  else {
    wgt_->show();
    wgt_->setFocus();
  }
}

void CityPluginHandler::slotWgtDel()
{
  wgt_ = nullptr;
}

} // map
} // meteo
