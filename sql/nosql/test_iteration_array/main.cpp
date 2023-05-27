#include <qstringlist.h>
#include <qcoreapplication.h>
#include <qfile.h>

#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/targ.h>
#include <cross-commons/app/helpformatter.h>
#include <cross-commons/app/paths.h>

#include <sql/nosql/document.h>
#include <sql/nosql/array.h>
#include <sql/dbi/dbiquery.h>
#include <sql/dbi/dbientry.h>
#include <sql/rapidjson/prettywriter.h>
#include <sql/rapidjson/stringbuffer.h>

#include <meteo/commons/global/global.h>
#include <meteo/commons/settings/settings.h>

int main(int argc, char** argv)
{
  TAPPLICATION_NAME("meteo");
  QCoreApplication* app = new QCoreApplication(argc,argv);
  ::meteo::gSettings(meteo::global::Settings::instance());

  if ( false == meteo::global::Settings::instance()->load() ) {
    error_log << QObject::tr("Не удалось загрузить настройки.");
    return EXIT_FAILURE;
  }

  std::unique_ptr<meteo::Dbi> db(meteo::global::dbMeteo());
  if ( nullptr == db ) {
    debug_log << QObject::tr("Не удается выполнить запрос в базу данных");
    return EXIT_FAILURE;
  }
  meteo::surf::QueryType qtype = meteo::surf::kTermValue;

  while (true) {
    bool result = false;
    std::unique_ptr<meteo::DbiQuery> query = db->queryptrByName("get_meteo_data");
    query->argDt( "start_dt", QString("2021-04-18T00:00:00") );
    query->argDt( "end_dt", QString("2021-04-18T00:00:00") );
    query->arg( "level", 0 );
    query->arg( "level_type", 1 );
    query->arg( "descrname", QStringList() << "T" );
    debug_log << "QUERY =" << query->query();

    QString err;
    if ( false == query->execInit(&err) ){
      debug_log << "NE UDALOS!";
      return EXIT_FAILURE;
    }


    while ( true == query->next()) {
      const meteo::DbiEntry& doc = query->entry();
      //debug_log<<doc.jsonExtendedString();
      meteo::surf::MeteoDataProto proto;
      if ( true == meteo::global::json2meteoproto( &doc, &proto, nullptr ) ) {
//      debug_log << "PROTOMETEO =" << proto.Utf8DebugString();
      }
    }
  }

  return EXIT_SUCCESS;
}
