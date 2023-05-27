#include "cityplugin.h"
#include <qpair.h>
#include <qstring.h>
#include <meteo/commons/ui/mainwindow/mainwindow.h>

namespace meteo {
namespace map {

/*!
 * \brief kMenuPath - Путь к пункту меню { id: "service", title: "Сервис"} ->
 * Для добавления вложенности нужно добавить пару в список
*/
static const auto kMenuPath = QList<QPair<QString, QString>>(
{
      QPair<QString, QString>("parameters", QObject::tr("Параметры"))
});



CityPlugin::CityPlugin(): MainWindowPlugin("cityplugin")
{
  mainwindow()->addActionToMenu({"citysettings", QObject::tr("Города")}, kMenuPath);
  handler_ = new meteo::map::CityPluginHandler(mainwindow());
}

CityPlugin::~CityPlugin()
{
  if ( nullptr != handler_ ){
    delete handler_;
    handler_ = nullptr;
  }
}


}
}
