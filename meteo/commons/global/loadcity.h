#ifndef METEO_COMMONS_GLOBAL_LOADCITY_H
#define METEO_COMMONS_GLOBAL_LOADCITY_H

#include <qstring.h>
#include <qmap.h>

#include <meteo/commons/proto/map_city.pb.h>

namespace meteo {
namespace map {

bool loadCitiesMap( QMap< int, meteo::map::proto::CitySetting>* citiesMap, const QString& citiesFileName );

bool loadDisplaySettings( QMap<int, meteo::map::proto::DisplaySetting>* cityDisplaySettingMap,
                          QMap<meteo::map::proto::CityMode, meteo::map::proto::DisplaySetting>* cityModeDisplaySettingMap,
                          meteo::map::proto::DisplaySetting* defaultDisplaySetting,
                          const QString& displaySettingsFileName );

bool isEmpty(const proto::DisplaySetting& ds);
bool isEmpty(const Font& font);
bool isEmpty(const Pen& pen);

}
}

#endif
