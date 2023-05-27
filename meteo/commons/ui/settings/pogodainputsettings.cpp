#include "pogodainputsettings.h"
#include "stationsearch.h"
#include "settingsfuncs.h"
#include "datasortitem.h"

#include <commons/textproto/tprototext.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/proto/appconf.pb.h>
#include <meteo/commons/settings/settings.h>
#include <meteo/commons/global/common.h>

namespace meteo {

static const QString& kSettingsName = QObject::tr("Аэродромы для ввода ФП");

WgtPogodaInputSettings::WgtPogodaInputSettings( const meteo::settings::Location& loc,
                                                const QString& username,
                                                QWidget *parent) :
  StationSelectForm (loc, parent),
  username_(username)
{
  setObjectName(kSettingsName);
}

QString WgtPogodaInputSettings::stationListLabelText()
{
  return QObject::tr("Аэродромы, по которым разрешен ввод фактической погоды пользователю %1").arg(this->username_);
}

QList<sprinf::MeteostationType> WgtPogodaInputSettings::stationTypes()
{
  return { sprinf::MeteostationType::kStationAerodrome };
}

QList<sprinf::Station> WgtPogodaInputSettings::loadStations()
{
  auto sett = data_;

  QList<sprinf::Station> result;
  if ( 0 == sett.enabled_inputs().address_size() ) {
    return result;
  }
  sprinf::MultiStatementRequest request;
  for( auto stationName : sett.enabled_inputs().address() ){
    request.add_station(stationName);
  }
  for ( auto type: this->stationTypes() ){
    request.add_type(type);
  }

  sprinf::Stations resp;
  if ( false == global::loadStations(request, &resp) ){
    error_log << QObject::tr("Ошибка при загрузке аэродромов разрешенных для ввода погоды");
    return result;
  }
  for ( auto station: resp.station()){
    result << station;
  }
  return result;
}

void WgtPogodaInputSettings::setData(const meteo::PogodaInputSts& data)
{
  this->data_ = data;
}

const meteo::PogodaInputSts& WgtPogodaInputSettings::getData()
{
  return this->data_;
}

bool WgtPogodaInputSettings::saveStations(const QList<sprinf::Station>& stations)
{
  meteo::PogodaInputSts *sett = &this->data_;
  sett->Clear();
  for ( auto station: stations ){
    auto stationId = station.station();
    if ( 0 == stationId.length() ){
      continue;
    }
    sett->mutable_enabled_inputs()->add_address(stationId);
  }
  return true;
}

}
