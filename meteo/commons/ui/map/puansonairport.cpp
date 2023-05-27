#include "puansonairport.h"
#include "layerairport.h"

namespace meteo {
namespace map {

PuansonAirport::PuansonAirport( Layer* layer ) : Puanson (layer)
{
}

const puanson::proto::Puanson& PuansonAirport::punch() const
{
  LayerAirport* l = maplayer_cast<LayerAirport*>( layer() );
  if ( nullptr == l ) {
    return punch_;
  }
  if ( kAlly == type_) {
    return l->allyPunch();
  }
  else {
    return l->enemyPunch();
  }
}


void PuansonAirport::setPunch( const puanson::proto::Puanson& p )
{
  LayerAirport* l = maplayer_cast<LayerAirport*>( layer() );
  if ( nullptr == l ) {
    punch_.CopyFrom(p);
  }
}

}
}


