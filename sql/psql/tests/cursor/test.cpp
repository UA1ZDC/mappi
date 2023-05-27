#include <qcoreapplication.h>

#include <cross-commons/app/paths.h>
#include <cross-commons/debug/tlog.h>

#include <sql/psql/psql.h>
#include <sql/psql/psqlquery.h>

#include <meteo/commons/global/global.h>
#include <meteo/commons/settings/settings.h>

int main( int argc, char* argv[] )
{
  TAPPLICATION_NAME("meteo");

  QCoreApplication app( argc, argv );
  meteo::gSettings(meteo::global::Settings::instance());
  if ( false == meteo::global::Settings::instance()->load() ) {
    error_log << QObject::tr("Не удалось загрузить настройки.");
    return EXIT_FAILURE;
  }
  [[maybe_unused]] meteo::Dbi* db = meteo::global::dbSprinf();
  auto query = db->queryptrByName("get_station_types");
  if ( false == query->exec() ) {
    error_log << QObject::tr("Не удалось выполнить запрос = %1").arg( query->query() ); 
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
