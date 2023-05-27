#include "geodezplugin.h"
#include "geodez.h"

namespace meteo{
namespace map{

GeodezPlugin::GeodezPlugin() : WidgetPlugin()
{
}

GeodezPlugin::~GeodezPlugin()
{

}

QWidget* GeodezPlugin::createWidget(QWidget* parent) const
{
  return new Geodez(parent);
}

Q_EXPORT_PLUGIN2( geodezplug, GeodezPlugin )

}
}


