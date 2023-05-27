#include <cross-commons/app/paths.h>
#include <cross-commons/debug/tlog.h>
#include <sql/dbi/dbi.h>
#include <sql/dbi/dbientry.h>
#include <sql/psql/psql.h>
#include <sql/psql/psqlquery.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/settings/settings.h>

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


  QString queryName("get_stations");

  debug_log << meteo::global::dbSprinf() << meteo::global::mongodbConfMeteo().print();

  std::unique_ptr<meteo::Dbi> db(meteo::global::dbMeteo());
  if ( nullptr == db.get() ) {
    debug_log << QObject::tr("Нет подключения к базе данных");
    return EXIT_FAILURE;
  }
  auto query = db->queryptrByName(queryName);
  if(nullptr == query) {
    return EXIT_FAILURE;
  }
  if ( false == query->exec() ) {
    debug_log << QObject::tr("Не удалось выполнить запрос '%1'")
               .arg( query->query() );
    return EXIT_FAILURE;
  }
  if ( false == query->initIterator() ) {
    debug_log << "ERR ITERATOR";
    return EXIT_FAILURE;
  }
  while ( true == query->next() ) {
    const meteo::DbiEntry& doc = query->entry();
    const meteo::Document& docjson = doc.valueDocument("countryinfo");
    Q_UNUSED(docjson);
//    debug_log << "DOC =" << docjson.jsonExtendedString();
  }

  return EXIT_SUCCESS;
}
