#include "gribdataplugin.h"
#include "gribdatawidget.h"

namespace meteo{
namespace map{

GribDataPlugin::GribDataPlugin() : WidgetPlugin()
{
  meteo::menu::Menu menu;
  menu.set_id("data");
  menu.set_title("Данные");

  meteo::menu::MenuItem* item = menu.add_item();
  item->set_id("gribdatawidgetsurface");
  item->set_title(QObject::tr("GRIB (приземные)").toStdString());
  item->set_disabled(false);
  item->set_dockwidget(true);
  item->set_options("gribsurface");

  meteo::menu::MenuItem* item2 = menu.add_item();
  item2->set_id("gribdatawidgetaero");
  item2->set_title(QObject::tr("GRIB (аэрология)").toStdString());
  item2->set_disabled(false);
  item2->set_dockwidget(true);
  item2->set_options("gribaerology");

  setMenu(menu);
  setName("gribdatawidget");
}

GribDataPlugin::~GribDataPlugin()
{

}

QWidget* GribDataPlugin::createWidget(QWidget* parent) const
{
  return new GribDataWidget(parent);
}

Q_EXPORT_PLUGIN2( gribdataplug, GribDataPlugin )

}
}


