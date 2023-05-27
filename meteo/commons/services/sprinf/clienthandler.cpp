#include "clienthandler.h"
#include "sprinfservice.h"

#include <qjsonobject.h>
#include <qjsonarray.h>
#include <qjsondocument.h>
#include <QElapsedTimer>

#include <commons/geobasis/projection.h>
#include <commons/meteo_data/meteo_data.h>
#include <commons/meteo_data/tmeteodescr.h>
#include <commons/textproto/pbtools.h>

#include <meteo/commons/global/global.h>
#include <meteo/commons/global/common.h>
#include <meteo/commons/global/loadcity.h>

#include <sql/dbi/dbi.h>
#include <sql/dbi/dbiquery.h>

static const QString kDbName    = "meteodb"; ///TODO: поменять, когда у спринфа появится своя база

namespace meteo {
namespace sprinf {

ClientHandler::ClientHandler(TSprinfService* service)
  :QObject(), service_(service)
{
  connections_ = new QMap<rpc::Channel*, HandlerConnection*>();
}
ClientHandler::~ClientHandler()
{
  for(auto c: *connections_) {
    c->blockSignals(true);
    c->channel()->disconnect();
    delete c;
  }
  delete connections_;
}

HandlerConnection* ClientHandler::connection(rpc::Channel *channel) const
{
  if(false == connections_->contains(channel)) {
    return nullptr;
  }
  return (*connections_)[channel];
}
void ClientHandler::removeConnection(rpc::Channel *channel)
{
  if(nullptr == channel)
    return;

  if(false == connections_->contains(channel))
    return;

  connections_->remove(channel);
}
bool ClientHandler::contains(rpc::Channel *channel) const
{
  return connections_->contains(channel);
}

void ClientHandler::slotClientConnected(meteo::rpc::Channel *client)
{
  HandlerConnection* conn = new HandlerConnection(this, client);
  connections_->insert(client, conn);
}

void ClientHandler::setUnlockEditDb() { service_->setUnlockEditDb(); }

HandlerConnection::HandlerConnection(ClientHandler *handler, rpc::Channel *channel)
  : QObject(), handler_(handler), channel_(channel), used_(false)
{
  QObject::connect(channel_, SIGNAL(disconnected()), this, SLOT(slotClientDisconnect()));
}
HandlerConnection::~HandlerConnection()
{
  while (false == deferredcalls_.isEmpty()) {
    ::google::protobuf::Closure* c = deferredcalls_.takeFirst();
    delete c;
  }
  handler_->removeConnection(channel_);
}

Station HandlerConnection::stationFromNoSql(const DbiEntry& doc )
{
  Station station;
  stationFromNoSql(doc, &station);
  return station;
}

bool HandlerConnection::stationFromNoSql(const DbiEntry& doc, Station* s)
{
  if ( nullptr == s ){
    return false;
  }
  s->set_id(doc.valueOid("_id").toStdString());
  bool ok = false;
  if ( true == doc.hasField("index")) {
    Document index = doc.valueDocument("index", &ok);
    if ( true == ok ) {
      if ( true == index.hasField("station_index") ) {
        s->set_index(index.valueInt32("station_index"));
      }
      if ( true == index.hasField("cccc") ) {
        s->set_cccc(index.valueString("cccc").toStdString());
      }
    }
  }

  if ( true == doc.hasField("station") ) {
    s->set_station( doc.valueString("station").toStdString() );
  }
  ok = false;
  if ( true == doc.hasField("countryinfo") ) {
    Document country = doc.valueDocument("countryinfo", &ok);
    if ( true == ok ) {
      countryFromNoSql( country, s->mutable_country() );
    }
  }
  if ( true == doc.hasField("station_type") ) {
    s->set_type(doc.valueInt32("station_type"));
  }
  if ( true == doc.hasField( "data_type" ) ) {
    s->set_data_type(doc.valueInt32("data_type"));
  }

  ok = false;
  if ( true == doc.hasField("name") ) {
    Document jName = doc.valueDocument("name", &ok);
    if ( true == ok ) {
      StationName name;
      if ( true == jName.hasField("international") ) {
        name.set_international(jName.valueString("international").toStdString());
      }
      if ( true == jName.hasField("short") ) {
        name.set_short_(jName.valueString("short").toStdString());
      }
      if ( true == jName.hasField("ru") ) {
        name.set_rus(jName.valueString("ru").toStdString());
      }
      s->mutable_name()->CopyFrom(name);
    }
  }

  if ( true == doc.hasField("location") ) {
    GeoPointPb *gp = s->mutable_position();
    const meteo::GeoPoint &geoPoint = doc.valueGeo("location");
    gp->set_lon_radian( geoPoint.lon() );
    gp->set_lat_radian( geoPoint.lat() );
    gp->set_height_meters( geoPoint.alt() );
    //s->mutable_position()->CopyFrom(gp);
  }

  if ( true == doc.hasField("hydro_levels") ) {
    meteo::sprinf::HydroLevels *hLvls = s->mutable_hydro_levels();

    Document hLvlsDoc = doc.valueDocument("hydro_levels");
    if (hLvlsDoc.hasField("poima")) {
      double poima = hLvlsDoc.valueDouble("poima");
      hLvls->set_floodplain(poima);
    }
    if (hLvlsDoc.hasField("zatop")) {
      double zatop = hLvlsDoc.valueDouble("zatop");
      hLvls->set_flooding(zatop);
    }
    if (hLvlsDoc.hasField("danger_level")) {
      double danger_level = hLvlsDoc.valueDouble("danger_level");
      hLvls->set_danger_level(danger_level);
    }
    //s->mutable_hydro_levels()->CopyFrom(hLvls);


  }

  if ( true == doc.hasField("airstrip_direction") ) {
    double airstrip_direction = doc.valueDouble("airstrip_direction");
    s->set_airstrip_direction(airstrip_direction);
  }
  return true;
}

bool HandlerConnection::countryFromNoSql( const DbiEntry& doc, Country* c )
{
  if ( nullptr == c ){
    return false;
  }
  if ( true == doc.hasField("number") ) {
    c->set_number( doc.valueInt32("number") );
  }
  if ( true == doc.hasField("code") ) {
    c->set_code( doc.valueString("code").toStdString() );
  }
  if ( true == doc.hasField("icao") ) {
    //TODO реализовать в psql чтение массивов
    auto str = doc.valueString("icao");
    c->add_icao( str.toStdString() );

    //    Array icao = doc.valueArray("icao");
    //    while ( icao.next() ) {
    //      c->add_icao(icao.valueString().toStdString());
    //    }
  }
  if ( true == doc.hasField("name") ) {
    c->set_name( doc.valueString("name").toStdString() );
  }
  if ( true == doc.hasField("name_en") ) {
    c->set_name_en( doc.valueString("name_en").toStdString() );
  }
  if ( true == doc.hasField("name_ru") ) {
    c->set_name_ru( doc.valueString("name_ru").toStdString() );
  }
  if ( true == doc.hasField("wmocode") ) {
    c->set_wmocode( doc.valueString("wmocode").toStdString() );
  }
  if ( true == doc.hasField("wmonumber") ) {
    c->set_wmonumber( doc.valueInt32("wmonumber") );
  }
  return true;
}

StationFull HandlerConnection::fullStationFromNoSql(const DbiEntry &doc)
{
  StationFull sf;
  Station s;

  s.set_id(doc.valueOid("_id").toStdString());

  bool ok = false;
  Document index = doc.valueDocument("index", &ok);
  if ( true == ok ) {
    if ( true == index.hasField("station_index") ) {
      s.set_index(index.valueInt32("station_index"));
    }
    if ( true == index.hasField("cccc") ) {
      s.set_cccc(index.valueString("cccc").toStdString());
    }
  }
  if ( true == doc.hasField("station") ) {
    s.set_station( doc.valueString("station").toStdString() );
  }
  if ( true == doc.hasField("countryinfo") ) {
    auto countries = doc.valueArray("countryinfo");
    if ( true == countries.initIterator() ) {
      while ( true == countries.next() ) {
        Document country;
        if ( true == countries.valueDocument(&country) ) {
          countryFromNoSql( country, s.mutable_country() );
        }
        break;
      }
    }
  }
  if ( true == doc.hasField("station_type") ) {
    s.set_type(doc.valueInt32("station_type"));
  }
  if ( true == doc.hasField("data_type") ) {
    s.set_data_type(doc.valueInt32("data_type"));
  }

  ok = false;
  if ( true == doc.hasField("name") ) {
    Document jName = doc.valueDocument("name", &ok);
    if ( true == ok ) {
      StationName name;
      if ( true == jName.hasField("international") ) {
        name.set_international(jName.valueString("international").toStdString());
      }
      if ( true == jName.hasField("short") ) {
        name.set_short_(jName.valueString("short").toStdString());
      }
      if ( true == jName.hasField("ru")) {
        name.set_rus(jName.valueString("ru").toStdString());
      }
      s.mutable_name()->CopyFrom(name);
    }
  }

  meteo::GeoPoint geoPoint = doc.valueGeo("location");
  s.mutable_position()->CopyFrom(geopoint2pbgeopoint(geoPoint));

  sf.mutable_main()->CopyFrom(s);
  if ( true == index.hasField("cccc") ) {
    sf.set_icao(index.valueString("cccc").toStdString());
  }
  if ( true == doc.hasField("data_type") ) {
    sf.set_data_type(doc.valueInt32("data_type"));
  }
  if ( true == doc.hasField("country") ) {
    sf.set_country(doc.valueInt32("country"));
  }
  if ( true == doc.hasField("last_used") ) {
    sf.set_last_date(doc.valueDt("last_used").toString(Qt::ISODate).toStdString());
  }
  return sf;
}

MeteoCenter HandlerConnection::centerFromNoSql(const DbiEntry &doc)
{
  MeteoCenter c;
  c.set_id(doc.valueInt32("center"));
  c.set_name(doc.valueString("name").toStdString());
  c.set_short_name(doc.valueString("short_name").toStdString());
  c.set_priority(doc.valueInt32("priority"));
  Array subcenters = doc.valueArray("subcenters");
  while ( true == subcenters.next() ) {
    MeteoSubCenter* sub = c.add_subcenter();
    Document subcenter;
    bool ok = subcenters.valueDocument(&subcenter);
    if ( false == ok ) {
      continue;
    }
    sub->set_id(subcenter.valueInt32("id"));
    sub->set_name(subcenter.valueString("name").toStdString());
  }
  return c;
}

MeteoParameter HandlerConnection::parameterFromNoSql(const DbiEntry &doc)
{
  MeteoParameter mp;
  mp.set_id(doc.valueOid("_id").toInt());
  mp.set_name(doc.valueString("name").toStdString());
  mp.set_unit(doc.valueString("unit").toStdString());
  mp.set_grib(doc.valueInt32("grib1_parameter"));
  mp.set_bufr(doc.valueInt32("bufr_descr"));
  mp.set_discipline(doc.valueInt32("discipline"));
  mp.set_category(doc.valueInt32("category"));
  mp.set_unit_en(doc.valueString("unit_en").toStdString());
  return mp;
}

BufrParametersTable HandlerConnection::bufrParameterTableFromNoSql(const DbiEntry &doc)
{
  BufrParametersTable bpt;
  bpt.set_table_code(doc.valueString("bufr_code").toStdString());
  bpt.set_name(doc.valueString("bufr_description").toStdString());
  Array tables = doc.valueArray("values");
  while ( true == tables.next() ) {
    BufrParameter* p = bpt.add_parameter();
    Document table;
    bool ok = tables.valueDocument(&table);
    if ( false == ok ) {
      continue;
    }
    p->set_code(table.valueInt32("code"));
    p->set_name(table.valueString("description").toStdString());
  }
  return bpt;
}

template < class T, template <class C> class A > std::unique_ptr<DbiQuery> HandlerConnection::prepareSimpleQuery( Dbi* db,
                                                                                                                  const QString& queryname,
                                                                                                                  const A<T>& param,
                                                                                                                  const QString& parameter )
{
  auto query = db->queryptrByName(queryname);
  if(nullptr == query) {return nullptr;}
  if ( false == parameter.isEmpty() ) {
    query->arg( "paramname", parameter );
  }
  //  QList<T> values;
  //  for( const auto value : param ) {
  //    values << value;
  //  }

  if ( 0 != param.size() ) {
    query->arg( "paramvalue", param );
  }
  return query;
}

void HandlerConnection::GetParameterByParameter(QString parameter, const MeteoParameterRequest *request, MeteoParameters *response)
{
  auto errFunc = [response](QString err)
  {
    error_log << err;
    response->set_result(false);
    response->set_error_message(err.toStdString());
    return;
  };

  response->set_result(false);

  QString queryName = "get_parameter_by_parameter";

  std::unique_ptr<Dbi> db(meteo::global::dbSprinf());

  if ( nullptr == db.get() ) {
    return errFunc(QObject::tr("Не удается выполнить запрос в базу данных"));
  }

  auto query = prepareSimpleQuery( db.get(), queryName, request->value(), parameter );

  if ( false == query->exec() ) {
    return errFunc(QObject::tr("Не удается выполнить запрос в базу данных"));
  }

  if ( false == query->initIterator() ) {
    return errFunc(QObject::tr("Нет курсора в результате выполнения запроса %1")
                   .arg( query->query() ));
  }

  while( true == query->next() ) {
    const DbiEntry& doc = query->entry();
    MeteoParameter* p = response->add_parameter();
    p->CopyFrom(parameterFromNoSql(doc));
  }
  response->set_result(true);
}

void HandlerConnection::GetStationTypes(const TypesRequest *request, StationTypes *response)
{
  auto errFunc = [response](QString err)
  {
    error_log << err;
    response->set_result(false);
    response->set_error_message(err.toStdString());
    return;
  };

  response->set_result(false);

  QString queryName = "get_station_types";
  //  if ( false == meteo::global::kMongoQueriesNew.contains(queryName)) {
  //    return errFunc(QObject::tr("Не найдена функция для получения данных = '%1'")
  //                   .arg(queryName));
  //  }
  //  QString q = meteo::global::kMongoQueriesNew[queryName];

  QList<int> values;
  for( const auto value : request->type() ) {
    values << value;
  }

  //  NosqlQuery query;
  //  if ( true == values.empty() ) {
  //    q = NosqlQuery::removeArg(q, "filter");
  //  }
  //  query.setQuery(q);
  //  if ( false == values.empty() ) {
  //    query.arg(values);
  //  }

  std::unique_ptr<Dbi> db(meteo::global::dbSprinf());

  if ( nullptr == db.get() ) {
    return errFunc(QObject::tr("Не удается выполнить запрос в базу данных"));
  }
  auto query = prepareSimpleQuery( db.get(), queryName, request->type() );

  if ( false == query->exec() ) {
    return errFunc(QObject::tr("Не удается выполнить запрос в базу данных"));
  }

  if ( false == query->initIterator() ) {
    return errFunc(QObject::tr("Нет курсора в результате выполнения запроса %1")
                   .arg( query->query() ));
  }

  //  Document doc;
  while( true == query->next() ) {
    const DbiEntry& doc = query->entry();
    StationType* st = response->add_station();
    st->set_type(doc.valueInt32("type"));
    st->set_name(doc.valueString("description").toStdString());
    st->set_max_gmi_types(doc.valueInt32("max_data_type"));
    st->set_min_gmi_types(doc.valueInt32("min_data_type"));
  }

  response->set_result(true);
}

void HandlerConnection::GetGmiTypes(const TypesRequest* request, GmiTypes* response)
{
  auto errFunc = [response](QString err)
  {
    error_log << err;
    response->set_result(false);
    response->set_error_message(err.toStdString());
    return;
  };

  response->set_result(false);

  QString queryName = "get_gmi_types";

  std::unique_ptr<Dbi> db(meteo::global::dbSprinf());

  if ( nullptr == db.get() ) {
    return errFunc(QObject::tr("Не удается выполнить запрос в базу данных"));
  }
  auto query = prepareSimpleQuery( db.get(), queryName, request->type() );

  if ( false == query->exec() ) {
    return errFunc(QObject::tr("Не удается выполнить запрос в базу данных"));
  }

  if ( false == query->initIterator() ) {
    return errFunc(QObject::tr("Нет курсора в результате выполнения запроса %1")
                   .arg( query->query() ));
  }

  while ( true == query->next() ) {
    const DbiEntry& doc = query->entry();
    GmiType* gt = response->add_gmi();
    gt->set_type(doc.valueInt32("type"));
    gt->set_name(doc.valueString("description").toStdString());
    gt->set_parent(doc.valueInt32("parent"));
    gt->set_total(doc.valueBool("integrated"));
  }

  response->set_result(true);
}

void HandlerConnection::GetLevelTypes(const TypesRequest* request, LevelTypes* response)
{
  auto errFunc = [response](QString err)
  {
    error_log << err;
    response->set_result(false);
    response->set_error_message(err.toStdString());
    return;
  };

  response->set_result(false);

  QString queryName = "get_level_types";

  std::unique_ptr<Dbi> db(meteo::global::dbSprinf());

  if ( nullptr == db.get() ) {
    return errFunc(QObject::tr("Не удается выполнить запрос в базу данных"));
  }
  auto query = prepareSimpleQuery( db.get(), queryName, request->type() );

  if ( false == query->exec() ) {
    return errFunc(QObject::tr("Не удается выполнить запрос в базу данных"));
  }

  if ( false == query->initIterator() ) {
    return errFunc(QObject::tr("Нет курсора в результате выполнения запроса %1")
                   .arg( query->query() ));
  }

  while ( true == query->next() ) {
    const DbiEntry& doc = query->entry();
    LevelType* lt = response->add_level();
    lt->set_type(doc.valueInt32("code_num"));
    lt->set_name(doc.valueString("mean").toStdString());
    auto standrad_value = doc.valueArray("standard_value");
    while( standrad_value.next() ) {
      lt->add_value(standrad_value.valueInt32());
    }
  }

  response->set_result(true);
}



/**
 * @brief Запрос на получение списка военных округов
 * @param request
 * @param response
 */
void HandlerConnection::GetMilitaryDistrict(const StatementRequest *request, Regions *response)
{
  auto errFunc = [response](QString err)
  {
    error_log << err;
    response->set_result(false);
    response->set_error_message(err.toStdString());
    return;
  };

  response->set_result(false);

  std::unique_ptr<Dbi> db(meteo::global::dbSprinf());
  if ( nullptr == db.get() ) {
    return errFunc(QObject::tr("Не удается выполнить запрос в базу данных"));
  }
  auto query = db->queryptrByName("get_military_district");
  if( nullptr == query ) {
    return errFunc( QObject::tr("Нулевой указатель!") );
  }
  //
  // заполнение SQL-шаблона запроса параметрами
  //
  if ( request->id_size() > 0 ) {
    query->arg( "id", request->id() );
  }
  // если указан запрос на поиск вхождений
  if ( request->has_query() ) {
    query->arg( "query", request->query() );
  }
  if ( request->name_size()  > 0 ) {
    QStringList values;
    for ( const auto& value : request->name() ) {
      values << QString::fromStdString(value);
    }
    query->arg( "name", values );
  }
  if ( false == query->exec() ) {
    return errFunc(QObject::tr("Не удается выполнить запрос в базу данных"));
  }

  if ( false == query->initIterator() ) {
    return errFunc(QObject::tr("Нет курсора в результате выполнения запроса %1")
                   .arg( query->query() ));
  }
  while ( true == query->next() ) {
    const DbiEntry& doc = query->entry();
    auto region = response->add_region();
    region->set_id( doc.valueInt32("id"));
    region->set_name( doc.valueString("name").toStdString());
    region->set_region_center_name( doc.valueString( "center_name" ).toStdString() );
    if(doc.hasField("center_coord")){
      GeoPointPb *gp = region->mutable_region_center();
      const meteo::GeoPoint &geoPoint = doc.valueGeo("center_coord");
      gp->set_lon_radian( geoPoint.lon() );
      gp->set_lat_radian( geoPoint.lat() );
    }
    if(doc.hasField("region_coords")){
      Array geoarr = doc.valueArray("region_coords");
      while ( true == geoarr.next() ) {
        Document geodoc;
        geoarr.valueDocument(&geodoc);
        if ( false == geodoc.hasField("coordinates") ) {
          continue;
	}
        QList<meteo::GeoVector> geov = geodoc.valueGeoVector("coordinates");
        debug_log << "geov.size()"<<geov.size();
	for ( auto gv : geov ) {
          if(0 < geov.size()){
            GeoVectorPb *gvp = region->add_region_coords();
            for(auto agp:gv ){
              GeoPointPb *ap = gvp->add_point();
              ap->set_lon_radian( agp.lon() );
              ap->set_lat_radian( agp.lat() );
            }
            break; 
          }
	}
      }
    }
  }
  response->set_result(true);
}


/**
 * @brief Запрос на получение списка регионов
 *
 * Может включать в себя постраничное отображение (параметры skip, limit)
 * может производить поиск по регионам через параметр query (поиск любого вхождения query)
 * или жесткий поиск по полному совпадению
 *
 * @param request
 * @param response
 */
void HandlerConnection::GetRegions(const MultiStatementRequest *request, Regions *response)
{

  auto errFunc = [response](QString err)
  {
    error_log << err;
    response->set_result(false);
    response->set_error_message(err.toStdString());
    return;
  };

  response->set_result(false);

  std::unique_ptr<Dbi> db(meteo::global::dbSprinf());
  if ( nullptr == db.get() ) {
    return errFunc(QObject::tr("Не удается выполнить запрос в базу данных"));
  }
  auto query = db->queryptrByName("get_regions");
  if( nullptr == query ) {
    return errFunc( QObject::tr("Нулевой указатель!") );
  }

  //
  // заполнение SQL-шаблона запроса параметрами
  //
  if ( request->id_size() > 0 ) {
    query->argOid( "id", request->id() );
  }

  // если указан пропуск - заполняем
  if ( request->has_skip() ) {
    query->arg( "skip", request->skip() );
  }
  // если ограничиваем количество возвращаемых записей - заполняем
  if ( request->has_limit() ) {
    query->arg( "limit", request->limit() );
  }
  // если указан запрос на поиск вхождений
  if ( request->has_query() ) {
    query->arg( "query", request->query() );
  }

  if ( request->name_size()  > 0 ) {
    QStringList values;
    for ( const auto& value : request->name() ) {
      values << QString::fromStdString(value);
    }
    query->arg( "name", values );
  }
  

  if ( false == query->exec() ) {
    return errFunc(QObject::tr("Не удается выполнить запрос в базу данных"));
  }

  if ( false == query->initIterator() ) {
    return errFunc(QObject::tr("Нет курсора в результате выполнения запроса %1")
                   .arg( query->query() ));
  }

  while ( true == query->next() ) {
    const DbiEntry& doc = query->entry();
    auto region = response->add_region();
    //
    //
    region->set_id(    doc.valueInt32(  "id" )  );
    region->set_name(  doc.valueString(  "title" ).toStdString() );

    GeoPointPb *gp = region->mutable_region_center();
    const meteo::GeoPoint &geoPoint = doc.valueGeo("region_center_point");
    gp->set_lon_radian( geoPoint.lon() );
    gp->set_lat_radian( geoPoint.lat() );

    const Document& geodoc = doc.valueDocument("region_coords");
    const QList<meteo::GeoVector>& geov = geodoc.valueGeoVector("coordinates");
    for ( auto gv : geov ) {
      if(0 < geov.size()){
        GeoVectorPb *gvp = region->add_region_coords();
        for(auto agp:gv){
          GeoPointPb *ap = gvp->add_point();
          ap->set_lon_radian( agp.lon() );
          ap->set_lat_radian( agp.lat() );
        }
      }
      break;
    }

  }
  response->set_result(true);
}



/**
 * @brief Получаем группы регионов из базы. либо все, либо по айди
 *
 * @param request
 * @param response
 * @return true
 * @return false
 */
void HandlerConnection::GetRegionGroups(  const RegionGroupsRequest* request, 
                                            RegionGroupsReply* response)
{
  std::unique_ptr<Dbi> db(meteo::global::dbSprinf());
  if ( nullptr == db.get() ) {
    error_log << QObject::tr("Ошибка при подключении к базе данных Sprinf");
    response->set_result(false);
    response->set_comment(QObject::tr("Ошибка при подключении к базе данных Sprinf").toStdString());
    return;
  }

  auto query = db->queryptrByName("get_region_groups");
  if(nullptr == query) { return ; }
  // устанавливаем значение координаты
  
  if ( request->has_id() ) {
    query->arg("id", request->id());
  }
  
  //debug_log<<query->query();

  QString error;
  if(false == query->execInit( &error ) ){
    error_log << error;
    response->set_result(false);
    response->set_comment(error.toStdString());
    return;
  }

  // повторящиеся группы регионов
  // сохраняем айдишщник при первом попадании и потом проверяем, есть ли в списке такой айди
  // если есть - заполняем протосообщение новыми данными, если нет - добавляем
  QMap<int, RegionGroupsReplyResult*> regiongroupids;

  while ( true == query->next()) {
    const DbiEntry& doc = query->entry();
    // если есть параметры на станции
    if ( true == doc.hasField("id")) {
      int region_id = doc.valueInt32("id");

      if (!regiongroupids.contains(region_id)) {
        // создаем новую структуру в протоответе
        regiongroupids.insert(region_id, response->add_data());
        // regiongroupids[region_id]->set_region_id(region_id);
        // заполняем ее
        regiongroupids[region_id]->set_id(          doc.valueInt32( "id") );
        regiongroupids[region_id]->set_title(       doc.valueString("title").toStdString() );
        regiongroupids[region_id]->set_description( doc.valueString("description").toStdString() );
        regiongroupids[region_id]->set_is_active(   doc.valueBool("is_active") );
      }

      // добавляем в ответ станции/регионы
      auto* region = regiongroupids[region_id]->add_regions();
      if ( doc.hasField("region_id")  ){
        region->set_region_id( doc.valueInt32("region_id")                  );
        region->set_title(     doc.valueString("region_name").toStdString() );
        // debug_log << doc.valueGeo("region_coord");
        GeoPointPb *gp = region->mutable_region_coord();
        const meteo::GeoPoint &geoPoint = doc.valueGeo("region_coord");
        gp->set_lon_radian( geoPoint.lon() );
        gp->set_lat_radian( geoPoint.lat() );

      }
      else if ( doc.hasField("station") ){
          region->set_title(      doc.valueString("station_name").toStdString() );
          // debug_log << dc.valueGeo("station_coord");
          GeoPointPb *gp = region->mutable_station_coord();
          const meteo::GeoPoint &geoPoint = doc.valueGeo("station_coord");
          gp->set_lon_radian( geoPoint.lon() );
          gp->set_lat_radian( geoPoint.lat() );
          if ( doc.hasField("station_name") && !doc.valueString("station_name").isNull() && !doc.valueString("station_name").isEmpty() ){
            Station s;
            if ( true == stationFromNoSql(doc.valueDocument("station"), &s) ){
              region->mutable_station()->CopyFrom(s);
            }
          }
        }
    }
  }
  return;
}

/**
 * @brief Сохраняем/создаем новую группу регионов/станций в базе
 *
 * @param data
 * @return true
 * @return false
 */
void HandlerConnection::SaveRegionGroups( const RegionGroupsRequest*  data, 
                                          RegionGroupsReply* response){
  
  std::unique_ptr<Dbi> db(meteo::global::dbSprinf());
  if ( nullptr == db.get() ) {
    error_log << QObject::tr("Ошибка при подключении к базе данных Sprinf");
    response->set_result(false);
    response->set_comment(QObject::tr("Ошибка при подключении к базе данных Sprinf").toStdString());
    return;
  }

  auto query = db->queryptrByName("save_region_groups");
  if(nullptr == query) {return;}

  if ( data->has_id() ){
    query->arg("id"     , data->id()    );
  }else{
    query->argWithoutQuotes("id"     , "DEFAULT"     );
  }
  if ( data->has_title() ){
    query->arg("title"  , data->title() );
  }
  if ( data->has_description() ){
    query->arg("description" , data->description() );
  }
  if ( data->has_is_active() ){
    query->arg("is_active"   , data->is_active()   );
  }

  if ( data->regions_size()>0 ){
    QList<int> regions_id;
    QList<QString> stations;
    QList<int> stations_types;
    for (int i = 0; i < data->regions_size(); ++i){
      // если задан регион, то добавляем region_id
      if (data->regions(i).has_region_id()){
        regions_id.append( data->regions(i).region_id() );
      }else
        if (data->regions(i).has_station()){
          stations.append( QString::fromStdString(data->regions(i).station().station()) );
          stations_types.append( data->regions(i).station().type() );
        }
    }
    query->arg("regions_id"  , regions_id   );
    query->arg("stations" ,  stations  );
    query->arg("stations_types" , stations_types  );
  }
  
  if ( !query->exec() ) {
    error_log << QObject::tr("Ошибка при выполнении запроса в базе данных");
    response->set_result(false);
    response->set_comment(QObject::tr("Ошибка при выполнении запроса в базе данных").toStdString());
    return;
  }

  // а теперь проходимся по полям и добавляем/удаляем лишние из базы
  // удаляем все связанные записи с last_id из базы
  // создаем эти записи заново
  
  return ;
}

/**
 * @brief Удаляем группу регионов из базы по айди
 *
 * @param request
 * @param response
 * @return true
 * @return false
 */
void HandlerConnection::DeleteRegionGroups(  const RegionGroupsRequest* request, 
                                             RegionGroupsReply* response){

  std::unique_ptr<Dbi> db(meteo::global::dbSprinf());
  if ( nullptr == db.get() ) {
    error_log << QObject::tr("Ошибка при подключении к базе данных Sprinf");
    response->set_result(false);
    response->set_comment(QObject::tr("Ошибка при подключении к базе данных Sprinf").toStdString());
    return ;
  }

  auto query = db->queryptrByName("delete_region_groups");
  if(nullptr == query) {
    response->set_result(false);
    response->set_comment(QObject::tr("Не найден запрос delete_region_groups").toStdString());
    return;
  }
  // устанавливаем значение координаты
  
  if ( request->has_id() ) {
    query->arg("id", request->id());
  }

  // debug_log<<query->query();

  QString error;
  if(false == query->execInit( &error ) ){
    error_log << error;
    response->set_result(false);
    response->set_comment(error.toStdString());
    return;
  }

  return;
}


/**
 * @brief Запрос на получение списка станций
 *
 * Может включать в себя постраничное отображение (параметры skip, limit)
 * может производить поиск по станциям через параметр query (поиск любого вхождения query)
 * или жесткий поиск по полному совпадению
 *
 * @param request
 * @param response
 */
void HandlerConnection::GetStations(const MultiStatementRequest *request, Stations *response)
{
  QElapsedTimer timer;
  timer.start();

  auto errFunc = [response](QString err)
  {
    error_log << err;
    response->set_result(false);
    response->set_error_message(err.toStdString());
    return;
  };

  response->set_result(false);

  QString queryName = "get_stations";
  if ( false == meteo::global::kMongoQueriesNew.contains(queryName)) {
    return errFunc(QObject::tr("Не найдена функция для получения данных = '%1'")
                   .arg(queryName));
  }

  std::unique_ptr<Dbi> db(meteo::global::dbSprinf());

  if ( nullptr == db.get() ) {
    return errFunc(QObject::tr("Нет подключения к базе данных!"));
  }
  auto query = db->queryptrByName(queryName);
  if( nullptr == query ) {
    return errFunc( QObject::tr("Нулевой указатель!") );
  }

  //
  // заполнение SQL-шаблона запроса параметрами
  //
  if ( request->id_size() > 0 ) {
    query->argOid( "id", request->id() );
  }

  // если указан пропуск - заполняем
  if ( request->has_skip() ) {
    query->arg( "skip", request->skip() );
  }
  // если ограничиваем количество возвращаемых записей - заполняем
  if ( request->has_limit() ) {
    query->arg( "limit", request->limit() );
  }
  // если указан запрос на поиск вхождений
  if ( request->has_query() ) {
    query->arg( "query", request->query() );
  }

  if ( request->index_size() > 0 ) {
    QList<int> values;
    for ( const auto& value : request->index() ) {
      values << value;
    }
    query->arg( "stationindex", values );
  }

  if ( request->cccc_size() > 0 ) {
    QStringList values;
    for ( const auto& value : request->cccc() ) {
      values << QString::fromStdString(value);
    }
    query->arg( "cccc", values );
  }

  if ( request->type_size() > 0 ) {
    QList<int> values;
    for ( const auto& value : request->type() ) {
      values << value;
    }
    query->arg( "stationtype", values );
  }

  if ( request->data_type_size() > 0 ) {
    QList<int> values;
    for ( const auto& value : request->data_type() ) {
      values << value;
    }
    query->arg( "datatype", values );
  }
  if ( request->country_size() > 0) {
    QList<int> values;
    for ( const auto& value : request->country() ) {
      values << value;
    }
    query->arg( "country", values );
  }
  if ( request->name_size()  > 0 ) {
    QStringList values;
    for ( const auto& value : request->name() ) {
      values << QString::fromStdString(value);
    }
    query->arg( "name", values );
  }
  if ( true == request->has_circle() ) {
    meteo::GeoPoint gp(request->circle().center().lat_radian(),request->circle().center().lon_radian());
    query->argGeoCircle("geometry",gp,request->circle().radius_meter());
  }

  if ( 0 != request->station_size() ){
    query->arg( "station", request->station() );
  }

  //debug_log << QString::fromUtf8("Время на проверку условий: %1 мсек").arg(timer.restart());
  //debug_log << query->query();
  if ( false == query->exec() ) {
    return errFunc(QObject::tr("Не удается выполнить запрос в базу данных"));
  }

  if ( false == query->initIterator() ) {
    return errFunc(QObject::tr("Нет курсора в результате выполнения запроса %1")
                   .arg( query->query() ));
  }

  //debug_log << QString::fromUtf8("Время на запрос к базе: %1 мсек").arg(timer.restart());
  while ( true == query->next() ) {
    const DbiEntry& doc = query->entry();
    Station s;
    if ( true == stationFromNoSql(doc, &s) ){
      response->add_station()->CopyFrom(s);
    }
  }
  debug_log << QString::fromUtf8("Время на обработку ответа: %1 мсек").arg(timer.restart());
  response->set_result(true);
}



void HandlerConnection::GetStationsByPosition(const CircleRegionRequest* request, StationsWithDistance* response)
{
  auto errFunc = [response](QString err)
  {
    error_log << err;
    response->set_result(false);
    response->set_error_message(err.toStdString());
    return;
  };

  response->set_result(false);

  bool completeness = request->has_circle() &&
                      request->circle().has_center() &&
                      request->circle().center().has_lon_radian() &&
                      request->circle().center().has_lat_radian() &&
                      request->circle().has_radius_meter();

  if ( false == completeness ) {
    return errFunc(QObject::tr("Запрос заполнен не полностью"));
  }

  QString queryName = "get_station_by_position";

  QList<double> coords;
  coords << request->circle().center().lon_radian()*RAD2DEG;
  coords << request->circle().center().lat_radian()*RAD2DEG;

  std::unique_ptr<Dbi> db(meteo::global::dbSprinf());

  if ( nullptr == db.get() ) {
    return errFunc(QObject::tr("Не удается выполнить запрос в базу данных"));
  }

  auto query = db->queryptrByName(queryName);
  if(nullptr == query) {return errFunc("");}
  query->arg( "coordinates", coords );
  query->arg( "maxdistance", request->circle().radius_meter());
  if ( true == request->has_station_type() ) {
    query->arg( "stationtype", request->station_type() );
  }

  if ( false == query->exec() ) {
    return errFunc(QObject::tr("Не удается выполнить запрос в базу данных"));
  }

  if ( false == query->initIterator() ) {
    return errFunc(QObject::tr("Нет курсора в результате выполнения запроса %1")
                   .arg( query->query() ));
  }

  while ( true == query->next() ) {
    const DbiEntry& doc = query->entry();
    StationWithDistance* swd = response->add_station();
    bool ok = false;
    Document dist = doc.valueDocument("dist", &ok);
    if ( true == ok ) {
      swd->set_distance_to(dist.valueDouble("calculated"));
    }
    swd->mutable_main()->CopyFrom(stationFromNoSql(doc));
  }

  response->set_result(true);
}

void HandlerConnection::GetMeteoCenters(const MeteoCenterRequest* request, MeteoCenters* response)
{
  auto errFunc = [response](QString err)
  {
    error_log << err;
    response->set_result(false);
    response->set_error_message(err.toStdString());
    return;
  };

  response->set_result(false);

  QString queryName = "get_meteo_centers";


  std::unique_ptr<Dbi> db(meteo::global::dbSprinf());

  if ( nullptr == db.get() ) {
    return errFunc(QObject::tr("Не удается выполнить запрос в базу данных"));
  }
  auto query = db->queryptrByName(queryName);
  if(nullptr == query) {return errFunc("");}

  QList<int> values;
  if ( request->center_id_size() > 0 ) {
    for ( const auto& value : request->center_id() ) {
      values << value;
    }
    query->arg( "@center@", values );
    values.clear();
  }

  if ( request->subcenter_id_size() > 0 ) {
    for ( const auto& value : request->subcenter_id() ) {
      values << value;
    }
    query->arg( "@subcenter@", values );
    values.clear();
  }

  if ( false == query->exec()) {
    return errFunc(QObject::tr("Не удается выполнить запрос в базу данных"));
  }

  if ( false == query->initIterator() ) {
    return errFunc(QObject::tr("Нет курсора в результате выполнения запроса %1")
                   .arg( query->query() ));
  }

  while ( true == query->next()) {
    const DbiEntry& doc = query->entry();
    MeteoCenter* c = response->add_center();
    c->CopyFrom(centerFromNoSql(doc));
  }
  response->set_result(true);
}

void HandlerConnection::GetMeteoParametersByBufr(const MeteoParameterRequest* request, MeteoParameters* response)
{
  GetParameterByParameter("bufr_descr", request, response);
}

void HandlerConnection::GetBufrParametersTables(const BufrParametersTableRequest* request, BufrParametersTables* response)
{
  auto errFunc = [response](QString err)
  {
    error_log << err;
    response->set_result(false);
    response->set_error_message(err.toStdString());
    return;
  };

  response->set_result(false);

  QString queryName = "get_bufr_code";

  std::unique_ptr<Dbi> db(meteo::global::dbSprinf());

  if ( nullptr == db.get() ) {
    return errFunc(QObject::tr("Не удается выполнить запрос в базу данных"));
  }

  auto query = db->queryptrByName(queryName);
  if(nullptr == query) {return errFunc("");}

  QStringList values;
  if ( request->table_code_size() > 0 ) {
    for( const auto value : request->table_code() ) {
      if ( 5 > value.length() ) {
        values << QString("0%1").arg( QString::fromStdString(value) );
      }
      else {
        values << QString::fromStdString(value);
      }
    }
    query->arg( "@bufrcode@", values );
  }

  if ( false == query->exec()) {
    return errFunc(QObject::tr("Не удается выполнить запрос в базу данных"));
  }

  if ( false == query->initIterator() ) {
    //    return errFunc(QObject::tr("Нет курсора в результате выполнения запроса %1")
    //                   .arg( query->query() ));
  }

  while ( true == query->next() ) {
    const DbiEntry& doc = query->entry();
    BufrParametersTable* s = response->add_table();
    s->CopyFrom(bufrParameterTableFromNoSql(doc));
  }

  response->set_result(true);
}

void HandlerConnection::GetCountry( const ::meteo::sprinf::CountryRequest* req, ::meteo::sprinf::Countries* resp )
{
  Q_UNUSED(req);
  auto setresult = [resp]( bool fl, const QString& str = QString() ) {
    resp->set_result(fl);
    if ( 0 != str.size() ) {
      resp->set_comment( str.toStdString() );
    }
    if ( false == fl ) {
      error_log << str;
    }
    else if ( 0 != str.size() ) {
      info_log << str;
    }
  };
  QString queryName("get_country");

  std::unique_ptr<Dbi> db(meteo::global::dbSprinf());
  if ( nullptr == db.get() ) {
    setresult( false, QObject::tr("Нет подключения к базе данных") );
    return;
  }
  auto query = db->queryptrByName(queryName);
  if(nullptr == query) {return;}
  //  QString q = meteo::global::kMongoQueriesNew["get_country"];
  //  NosqlQuery query;
  if ( true == req->has_priority() ) {
    query->arg( "priority", req->priority() );
  }
  else {
  }
  if ( false == query->exec() ) {
    setresult( false, QObject::tr("Не удалось выполнить запрос '%1'")
               .arg( query->query() ) );
    return;
  }
  if ( false == query->initIterator() ) {
    setresult( false, QObject::tr("Не удалось получить курсор")
               .arg( query->query() ) );
    return;
  }
  while ( true == query->next() ) {
    const DbiEntry& doc = query->entry();
    meteo::sprinf::Country* c = resp->add_country();
    countryFromNoSql( doc, c );
  }

  resp->set_result(true);
}

void HandlerConnection::UpdateStation(const Station *request, ReportStationsAdded *response)
{
  auto errFunc = [response](QString err)
  {
    error_log << err;
    response->set_result(false);
    response->set_comment(err.toStdString());
    return;
  };

  response->set_result(false);

  QString queryName = "update_or_insert_station";

  std::unique_ptr<Dbi> db(meteo::global::dbSprinf());
  if ( nullptr == db.get() ) {
    return errFunc(QObject::tr("Не удается выполнить запрос в базу данных"));
  }
  auto query = db->queryptrByName(queryName);
  if(nullptr == query) {return errFunc("");}

  //  NosqlQuery query;
  //  query.setQuery(q);

  if ( true == request->has_id() &&
       "" != request->id() ) {
    query->argOid( "id", request->id());
  }
  //FIXME
  //  else { // TODO константа для обозначения отсутствия ключа?
  //    query->argJson( "id", "{\"$exists\":false}"); // хардкод - плохой подход
  //  }

  if ( true == request->has_index() ) {
    query->arg( "stationindex", request->index());
    query->arg( "station", QString::number(request->index())); // поле "station"
  }
  // предупреждает возможную ошибку, если в запросе вдруг окажутся и index, и cccc - в этом случае cccc игнорируется
  else if ( true == request->has_cccc() ) {
    query->arg( "cccc", request->cccc());
    query->arg( "station", QString::fromStdString(request->cccc())); // поле "station"
  }
  // редко используемый третий вариант - если нет ни index, ни cccc - в "station" записывается name.international
  else if ( true == request->has_name() and
            true == request->name().has_international() ) {
    query->arg( "station", QString::fromStdString(request->name().international())); // поле "station"
  }

  if ( true == request->has_name() ) {
    auto stName = request->name();
    if ( true == stName.has_rus() ) {
      query->arg( "nameru", stName.rus() );
    }
    if ( true == stName.has_short_() ) {
      query->arg( "nameshort", stName.short_());
    }
    if ( true == stName.has_international() ) {
      query->arg( "nameen", stName.international());
    }
  }

  if ( true == request->has_country() ) {
    query->arg( "country", request->country().number());
  }

  if ( true == request->has_type() ) {
    query->arg( "stationtype", request->type());
  }

  if ( true == request->has_position() ) {
    if ( true == request->position().has_lon_radian() &&
         true == request->position().has_lat_radian() &&
         true == request->position().has_height_meters() ) {
      meteo::GeoPoint gp( request->position().lat_radian(),
                          request->position().lon_radian(),
                          request->position().height_meters());
      query->arg("location", gp);
      query->arg("alt",request->position().height_meters() );
    }
  }

  if ( true == request->has_airstrip_direction() ) {
    query->arg( "direction", request->airstrip_direction());
  }

  if ( true == request->has_hydro_levels() ) {
    if ( true == request->hydro_levels().has_floodplain() ) {
      query->arg( "poima", request->hydro_levels().floodplain() );
    }
    if ( true == request->hydro_levels().has_flooding() ) {
      query->arg( "zatop", request->hydro_levels().flooding());
    }
    if ( true == request->hydro_levels().has_danger_level() ) {
      query->arg( "dangerlevel", request->hydro_levels().danger_level());
    }
  }

  // last_update
  QString currentDT_ISO = QDateTime::currentDateTime().toString(Qt::ISODate);
  query->argDt( "lastupdate", currentDT_ISO);

  if ( false == query->exec() ) {
    return errFunc(QObject::tr("Не удается выполнить запрос в базу данных"));
  }

  // ! comment в случае успешной записи станции содержит ее _id
  const DbiEntry& result = query->result();
  if ( result.hasField("update_station") ) {
    response->set_comment(result.valueOid("update_station").toStdString());
  }
  if ( result.hasField("value._id") ) {
    response->set_comment(result.valueOid("value._id").toStdString());
  }
  response->set_result(true);
}

void HandlerConnection::GetTlgStatistic (const ::meteo::sprinf::TlgStatisticRequest* request,
                                         ::meteo::sprinf::TlgStatisticResponse* response)
{
  auto errFunc = [response](QString err)
  {
    error_log << err;
    response->set_result(false);
    response->set_comment(err.toStdString());
    return;
  };

  response->set_result(false);
  QString queryName = "get_statistic_src";
  switch (request->type()) {
    case TlgStatisticRequestType::kDestination:
      queryName = "get_statistic_dest";
    break;
    case TlgStatisticRequestType::kSource:
    default:
      queryName = "get_statistic_src";
    break;
  }
  std::unique_ptr<Dbi> db(meteo::global::dbTelegram());
  if ( nullptr == db.get() ) {
    return errFunc(QObject::tr("Не удается выполнить запрос в базу данных"));
  }
  auto query = db->queryptrByName( queryName );
  if(nullptr == query) {return errFunc("");}
  query->argDt( "dt_start", request->date_begin());
  query->argDt( "dt_end", request->date_end());
  QString err;
  if ( false == query->execInit(&err) ){
    response->set_result(false);
    return errFunc(QObject::tr("Не удается выполнить запрос в базу данных"));
  }

  while ( true == query->next() ) {
    const DbiEntry& doc = query->entry();
    if ( true == doc.hasField("count") ) {
      ::meteo::sprinf::TlgStatistic *stat = response->add_stats();
      stat->set_date(doc.valueString("dt").toStdString());
      if(doc.hasField("src")){
        stat->set_src_address(doc.valueString("src").toStdString());
      }
      if(doc.hasField("dest")){
        stat->set_dest_address(doc.valueString("dest").toStdString());
      }
      stat->set_t1(doc.valueString("t1").toStdString());
      stat->set_t2(doc.valueString("t2").toStdString());
      stat->set_count(doc.valueInt32("count"));
    }
  }
  response->set_comment(QObject::tr("Ok").toStdString());
  response->set_result(true);
}

void HandlerConnection::GetCities( const ::meteo::sprinf::Dummy* request, ::meteo::map::proto::Cities* response )
{
  Q_UNUSED(request);
  QMap<int, meteo::map::proto::CitySetting > cities;
  if ( false == map::loadCitiesMap( &cities, global::kCitySettingsFileName ) ) {
    response->set_result(false);
    response->set_comment( QObject::tr("Не удалось загрузить города из файла %1")
                           .arg( global::kCitySettingsFileName ).toStdString() );
    return;
  }
  for ( auto it = cities.begin(), end = cities.end(); it != end; ++it ) {
    response->add_city_setting()->CopyFrom( it.value() );
  }
  response->set_result(true);
}

void HandlerConnection::GetCityDisplayConfig( const ::meteo::sprinf::Dummy* request, ::meteo::map::proto::CityDisplayConfig* response )
{
  Q_UNUSED(request);
  QMap< int, meteo::map::proto::CitySetting > cities;
  bool res = meteo::map::loadCitiesMap( &cities, meteo::global::kCitySettingsFileName );
  if ( false == res ) {
    response->set_result(false);
    response->set_comment( QObject::tr("Не удалось загрузить города из файла %1")
                           .arg( global::kCitySettingsFileName ).toStdString() );
    return;
  }

  QMap< int, meteo::map::proto::DisplaySetting > cityconfig;
  QMap< meteo::map::proto::CityMode, meteo::map::proto::DisplaySetting > citymodeconfig;
  meteo::map::proto::DisplaySetting defaultconfig;

  res = meteo::map::loadDisplaySettings( &cityconfig, &citymodeconfig, &defaultconfig, meteo::global::kDisplaySettingsFileName );
  if ( false == res ) {
    response->set_result(false);
    response->set_comment( QObject::tr("Не удалось загрузить параметры отображения городов из файла %1")
                           .arg( global::kDisplaySettingsFileName ).toStdString() );
    return;
  }
  for ( auto it = cityconfig.begin(), end = cityconfig.end(); it != end; ++it ) {
    int index = it.key();
    if ( true == cities.contains(index) ) {
      cities[index].mutable_display()->CopyFrom( it.value() );
    }
  }
  for ( auto it = citymodeconfig.begin(), end = citymodeconfig.end(); it != end; ++it ) {
    response->add_display_mode()->CopyFrom( it.value() );
  }
  for ( auto it = cities.begin(), end = cities.end(); it != end; ++it ) {
    response->mutable_cities()->add_city_setting()->CopyFrom( it.value() );
  }
  response->mutable_default_display()->CopyFrom(defaultconfig);
  response->set_result(true);
}

void HandlerConnection::DeleteStation(const Station *request, ReportStationsAdded *response)
{
  // удаляет _одну_ станцию, руководствуясь исключительно полем "id" поступившего запроса

  auto errFunc = [response](QString err)
  {
    error_log << err;
    response->set_result(false);
    response->set_comment(err.toStdString());
    return;
  };

  response->set_result(false);

  if ( ( false == request->has_id() ) ||
       ( true == request->has_id() &&
         "" == request->id() ) ) {
    return errFunc(QObject::tr("В запросе отсутствует id удаляемой станции"));
  }

  QString queryName = "delete_station";
  //  if ( false == meteo::global::kMongoQueriesNew.contains(queryName)) {
  //    return errFunc(QObject::tr("Не найдена функция для удаления данных = '%1'")
  //                   .arg(queryName));
  //  }
  //  QString q = meteo::global::kMongoQueriesNew[queryName];
  //
  //  NosqlQuery query;
  //  query.setQuery(q);

  std::unique_ptr<Dbi> db(meteo::global::dbSprinf());
  if ( nullptr == db.get() ) {
    return errFunc(QObject::tr("Не удается выполнить запрос в базу данных"));
  }
  auto query = db->queryptrByName( queryName );
  if(nullptr == query) {return errFunc("");}
  query->argOid( "id", request->id());
  if ( false == query->exec() ) {
    return errFunc(QObject::tr("Не удается выполнить запрос в базу данных"));
  }

  const DbiEntry& qResult = query->result();

  debug_log << "result:" << qResult.jsonExtendedString(); //TODO REMOVE
  debug_log << "ok:" << qResult.valueDouble("ok"); //TODO REMOVE
  debug_log << "n:" << qResult.valueInt32("n"); //TODO REMOVE
  debug_log << "RESULT =" << qResult.valueDouble("ok") << MnMath::isEqual( 1.0, qResult.valueDouble("ok") );

  if ( !MnMath::isEqual( 1.0, qResult.valueDouble("ok") ) ) {
    if ( true == qResult.hasField("writeErrors") ) {
      Document errDoc = qResult.valueDocument("writeErrors");
      return errFunc(errDoc.jsonExtendedString());
    }
    return;
  }

  response->set_result(true);
}

void HandlerConnection::slotFutureFinished()
{
  if(true == deferredcalls_.isEmpty()) {
    critical_log << QObject::tr("Непредвиденная ситуация. Завершился вызов. Но список вызовов пуст");
    return;
  }
  delete deferredcalls_.takeFirst();

  if(true == deferredcalls_.isEmpty()) {
    if(nullptr == channel_)
      this->deleteLater();
  }
  else {
    if(false == deferredcalls_.isEmpty())
      deferredcalls_[0]->Run();
  }
}

void HandlerConnection::slotClientDisconnect()
{
  handler_->removeConnection(channel_);
  channel_ = nullptr;
  this->deleteLater();
}

}
}
