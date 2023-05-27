#include <cross-commons/app/paths.h>
#include <cross-commons/debug/tlog.h>
#include <sql/dbi/dbi.h>
#include <sql/dbi/dbientry.h>
#include <sql/psql/psql.h>
#include <sql/psql/psqlquery.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/settings/settings.h>
#include <meteo/commons/global/loadcity.h>

#include <qcoreapplication.h>

#include <stdlib.h>

int main( int argc, char* argv[] )
{
  Q_UNUSED( argc );
  Q_UNUSED( argv );

  TAPPLICATION_NAME("meteo");

  QCoreApplication app( argc, argv );

  meteo::gSettings(meteo::global::Settings::instance());

  if ( false == meteo::global::Settings::instance()->load() ) {
    error_log << QObject::tr("Не удалось загрузить настройки.");
    return EXIT_FAILURE;
  }

  QMap< int, meteo::map::proto::CitySetting > cities;
  bool res = meteo::map::loadCitiesMap( &cities, meteo::global::kCitySettingsFileName );
  debug_log << "LOAD CITY STATE =" << res;
  for ( auto it = cities.begin(), end = cities.end(); it != end; ++it ) {
    debug_log << "CITY =" << it.key() << it.value().Utf8DebugString();
  }

  QMap< int, meteo::map::proto::DisplaySetting > cityconfig;
  QMap< meteo::map::proto::CityMode, meteo::map::proto::DisplaySetting > citymodeconfig;
  meteo::map::proto::DisplaySetting defaultconfig;

  res = meteo::map::loadDisplaySettings( &cityconfig, &citymodeconfig, &defaultconfig, meteo::global::kDisplaySettingsFileName );
  debug_log << "LOAD DISPLAY STATE =" << res;
  debug_log << "========== CITYCONF ============";
  for ( auto it = cityconfig.begin(), end = cityconfig.end(); it != end; ++it ) {
    debug_log << "conf = " << it.value().Utf8DebugString();
  }
  debug_log << "========== CITYMODE ============";
  for ( auto it = citymodeconfig.begin(), end = citymodeconfig.end(); it != end; ++it ) {
    debug_log << "conf = " << it.value().Utf8DebugString();
  }
  debug_log << "========== CITYDEFA ============";
  debug_log << "conf = " << defaultconfig.Utf8DebugString();

  return EXIT_SUCCESS;
}
