#include "settingsfuncs.h"
#include <meteo/commons/global/global.h>
#include <meteo/commons/proto/sprinf.pb.h>

const QString stationType(meteo::sprinf::MeteostationType type)
{  
  QHash<meteo::sprinf::MeteostationType, meteo::sprinf::StationType> types;
  if ( false == types.contains(type) ){
    meteo::sprinf::StationTypes resp;
    meteo::sprinf::MultiStatementRequest req;
    meteo::global::loadStationTypes(&resp, req);
    for ( auto station: resp.station() ){
      meteo::sprinf::MeteostationType stType =
          static_cast<meteo::sprinf::MeteostationType>(station.type());
      types[stType] = station;
    }
  }

  if ( false == types.contains(type) ){
    return QObject::tr("Неизвестная станция");
  }
  return QString::fromStdString(types[type].name());
}
