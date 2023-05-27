#include "settingplugin.h"

#include <meteo/commons/ui/mainwindow/widgethandler.h>
#include <meteo/commons/ui/mainwindow/mainwindow.h>

namespace meteo {
namespace app {

SettingPlugin::SettingPlugin(const QString& name)
  : QObject(WidgetHandler::instance()->mainwindow())
    , name_(name) { }

SettingPlugin::~SettingPlugin() { }

} // app
} // meteo
