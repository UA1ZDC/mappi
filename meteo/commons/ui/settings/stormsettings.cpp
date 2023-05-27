#include "stormsettings.h"
#include "stationsearch.h"
#include "settingsfuncs.h"
#include "datasortitem.h"

#include <commons/textproto/tprototext.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/settings/settings.h>
#include <meteo/commons/global/common.h>

namespace meteo {

static const QString& kSettingsName = QObject::tr("Штормовое кольцо");

enum Column
{
  kColumnIsoNumber = 0,
  kColumnIdx,
  kColumnRuName,
  kColumnLat,
  kColumnLon,
  kColumnAlt,
  kColumnType,
  kColumnDist
};

WgtStormSettings::WgtStormSettings( const meteo::settings::Location& loc,
                                      QWidget *parent) :
  StationSelectForm(loc,parent)
{  
  this->setObjectName(kSettingsName);
}

bool WgtStormSettings::saveStations(const QList<sprinf::Station>& stations)
{
  meteo::StormSettings sett;
  for ( auto station: stations ){
    sett.mutable_storm_ring()->add_address(station.station());
  }
  auto filePath = QObject::tr("%1/%2")
      .arg(MnCommon::etcPath())
      .arg(meteo::global::Settings::instance()->stormSettingsFileName());
  if ( false == TProtoText::toFile(sett, filePath) ){
    error_log << meteo::msglog::kFileWriteFailed.arg(filePath);
    return false;
  }
  return true;
}

QList<sprinf::Station> WgtStormSettings::loadStations()
{
  auto filePath = QObject::tr("%1/%2")
      .arg(MnCommon::etcPath())
      .arg(meteo::global::Settings::instance()->stormSettingsFileName());

  meteo::StormSettings sett;
  if ( false == TProtoText::fromFile(filePath, &sett) ){
    error_log << meteo::msglog::kFileReadFailed.arg(filePath);
    return QList<sprinf::Station>();
  }

  QList<sprinf::Station> stations;
  if ( 0 != sett.storm_ring().address_size() ) {
    sprinf::MultiStatementRequest request;
    for( auto stationName : sett.storm_ring().address() ){
      request.add_station(stationName);
    }
    for ( auto type: this->stationTypes() ){
      request.add_type(type);
    }

    sprinf::Stations resp;

    if ( false == global::loadStations(request, &resp) ){
      error_log.msgBox() << QObject::tr("Ошибка при загрузке станций штормового кольца");
    }
    else {
      for ( auto station: resp.station()){
        stations << station;
      }
    }
  }
  return stations;
}

QList<sprinf::MeteostationType> WgtStormSettings::stationTypes() {
  return { sprinf::MeteostationType::kStationSynop,
        sprinf::MeteostationType::kStationSynmob,
        sprinf::MeteostationType::kStationAirport,
        sprinf::MeteostationType::kStationNode,
        sprinf::MeteostationType::kStationNode
  };
}

QString WgtStormSettings::stationListLabelText()
{
  return QObject::tr("Станции, включенные в штормовое кольцо");
}

}
