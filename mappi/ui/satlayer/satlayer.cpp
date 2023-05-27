#include "satlayer.h"

#include <meteo/commons/ui/map/document.h>

#include <mappi/ui/satlayer/satlayermenu.h>
#include <mappi/ui/satelliteimage/satelliteimage.h>

namespace meteo {
namespace map {

SatLayer::SatLayer(Document* map, const QString& name):
  Layer(map, name)
{
}

SatLayer::~SatLayer()
{
  // чтобы меню не удалялось в процессе удаления слоёв используем deleteLater()
  menu_->deleteLater();
  menu_ = nullptr;

  delete proj_;
  proj_ = nullptr;
}

SatelliteImage* SatLayer::currentObject() const
{
  foreach ( Object* o, objects() ) {
    SatelliteImage* i = mapobject_cast<SatelliteImage*>(o);
    if ( nullptr != i && i->visible() && i->thematicType() == params_.type() ) {
      return i;
    }
  }

  return nullptr;
}

LayerMenu* SatLayer::layerMenu()
{
  if ( nullptr == menu_ ) {
    menu_ = new SatLayerMenu(this);
  }
  return menu_;
}

} //map
} //meteo
