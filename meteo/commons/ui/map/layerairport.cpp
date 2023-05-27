#include "layerairport.h"
#include "layermenu.h"
#include <commons/meteo_data/meteo_data.h>
#include <meteo/commons/ui/map/puansonairport.h>
#include <meteo/commons/global/global.h>


namespace meteo {
namespace map {

LayerAirport::LayerAirport(Document* map, const puanson::proto::Puanson& punch) : LayerPunch (map, punch)
{
  proto::Document doc = global::lastBlankParams(meteo::map::proto::DocumentType::kGeoMap);
  showCCCC_ = doc.showcccc();
}

LayerMenu* LayerAirport::layerMenu()
{
  if ( nullptr == menu_ ) {
    menu_ = new AirportMenu(this);
  }
  return menu_;
}

void LayerAirport::changeLabel()
{
  showCCCC_ = !showCCCC_;
  std::unordered_set<PuansonAirport*> list = objectsByType<PuansonAirport*>();
  if (showCCCC_) {
    foreach( PuansonAirport* p, list ) {
      TMeteoData md{p->meteodata()};
      TMeteoParam param(  p->cccc() , 2, control::RIGHT );
      md.set("CCCC", param);
      p->setMeteodata(md);
    }
  }
  else {
    for ( PuansonAirport* p : list ) {
      TMeteoData md{p->meteodata()};
      TMeteoParam param(  p->name() , 2, control::RIGHT );
      md.set("CCCC", param);
      p->setMeteodata(md);
    }
  }
  proto::Document doc = global::lastBlankParams(meteo::map::proto::DocumentType::kGeoMap);
  doc.set_showcccc( showCCCC_ );
  global::saveLastBlankParams(doc);
  repaint();
}

}
}


