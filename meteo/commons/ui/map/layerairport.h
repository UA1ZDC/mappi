#ifndef METEO_COMMONS_UI_MAP_LAYERAIRPORT_H
#define METEO_COMMONS_UI_MAP_LAYERAIRPORT_H

#include <meteo/commons/ui/map/layerpunch.h>

namespace meteo {
namespace map {

class LayerAirport : public LayerPunch
{
public:
  LayerAirport(Document* map, const puanson::proto::Puanson& punch);
  enum {
    Type = kLayerAirport
  };
  int type() const override { return Type; }
  LayerMenu* layerMenu() override;
  void changeLabel();
  bool showCCCC() { return showCCCC_; }
  const puanson::proto::Puanson& allyPunch() const { return allyPunch_; }
  void setAllyPunch( const puanson::proto::Puanson& p ) { allyPunch_.CopyFrom(p); }
  const puanson::proto::Puanson& enemyPunch() const { return enemyPunch_; }
  void setEnemyPunch( const puanson::proto::Puanson& p ) { enemyPunch_.CopyFrom(p); }

private:
  bool showCCCC_ = true;
  puanson::proto::Puanson allyPunch_;
  puanson::proto::Puanson enemyPunch_;
};

}
}

#endif // METEO_COMMONS_UI_MAP_LAYERAIRPORT_H
