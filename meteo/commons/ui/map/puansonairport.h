#ifndef METEO_COMMONS_UI_MAP_PUANSONAIRPORT_H
#define METEO_COMMONS_UI_MAP_PUANSONAIRPORT_H

#include "puanson.h"

namespace meteo {
namespace map {

class PuansonAirport : public Puanson
{
public:
  enum AirportType {
    kAlly,
    kEnemy
  };
  PuansonAirport( Layer* layer );
  const puanson::proto::Puanson& punch() const override;
  void setPunch( const puanson::proto::Puanson& p ) override;
  void setCCCC( const QString& cccc ) { cccc_ = cccc; }
  void setName( const QString& name ) { name_ = name; }
  QString cccc() { return cccc_; }
  QString name() { return name_; }
  AirportType airportType() { return type_; }
  void setAirportType( AirportType type ) { type_ = type; }

private:
  QString name_;
  QString cccc_;
  AirportType type_ = kAlly;

};

}
}



#endif // METEO_COMMONS_UI_MAP_PUANSONAIRPORT_H
