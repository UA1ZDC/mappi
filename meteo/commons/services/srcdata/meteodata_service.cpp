#include "meteodata_service.h"

#include <functional>

#include <qstring.h>
#include <qmap.h>
#include <QElapsedTimer>
#include <qjsondocument.h>
#include <qjsonarray.h>
#include <qjsonobject.h>

#include <cross-commons/debug/tlog.h>

#include <sql/dbi/gridfs.h>
#include <sql/nosql/nosqlquery.h>

#include <commons/mathtools/mnmath.h>
#include <commons/textproto/pbtools.h>
#include <commons/meteo_data/meteo_data.h>
#include <commons/meteo_data/complexmeteo.h>
#include <commons/meteo_data/tmeteodescr.h>
#include <meteo/commons/global/weatherloader.h>
#include <meteo/commons/ui/map/puanson.h>

#include <meteo/commons/global/global.h>

const std::string kDbNotReady = QObject::tr("База данных с результатами объективного анализа недоступна").toUtf8().constData();
const std::string kNoData = QObject::tr("Данных, удовлетворяющих условию, не найдено").toUtf8().constData();
static const QString msgIncorrectRequest = QObject::tr("Запрос составлен некорректно: часть обязательных параметров отcутствует");

static const std::string msgQueryExecutionError = "Ошибка при выполнении запроса в базу данных";

namespace meteo {
namespace surf {

static const QString kPrefix = "flight_wx";

struct Info {
  double lat;
  double lon;
  double alt;
  QDateTime dt;
  QString station;
  QString error;
};

struct Descr {
  uint64_t descr;
  QString descrname;
  double value;
  int qual;
  QString code;
};


MeteoDataService::MeteoDataService(const DataRequest* r, FullValue* dr, google::protobuf::Closure* done, kMeteoDataReqType rt, int serviceType)
  : TBaseDataService(done,serviceType),
    data_request_(r),
    data_reply_(nullptr),
    value_reply_(dr),
    sigwx_request_(nullptr),
    sigwx_resp_(nullptr),
    req_type_(rt)
{
}

MeteoDataService::MeteoDataService(const DataRequest* r, DataReply* dr, google::protobuf::Closure* done, kMeteoDataReqType rt, int serviceType)
  : TBaseDataService(done,serviceType),
    data_request_(r),
    data_reply_(dr),
    value_reply_(nullptr),
    sigwx_request_(nullptr),
    sigwx_resp_(nullptr),
    req_type_(rt)
{
}

MeteoDataService::MeteoDataService(const DataRequest* r, CountDataReply* dr, google::protobuf::Closure* done, kMeteoDataReqType rt, int serviceType)
  : TBaseDataService(done,serviceType),
    data_request_(r),
    data_reply_(nullptr),
    value_reply_(nullptr),
    sigwx_request_(nullptr),
    sigwx_resp_(nullptr),
    count_resp_(dr),
    req_type_(rt)
{
}

MeteoDataService::MeteoDataService(const SigwxDesc* r,meteo::sigwx::SigWx* dr, google::protobuf::Closure* done, int serviceType)
  : TBaseDataService(done,serviceType),
    data_request_(nullptr),
    data_reply_(nullptr),
    value_reply_(nullptr),
    sigwx_request_(r),
    sigwx_resp_(dr),
    req_type_(rGetSigwx)
{
}

MeteoDataService::MeteoDataService(const DataRequest* r,SigwxDescResponse* dr, google::protobuf::Closure* done, int serviceType)
  : TBaseDataService(done,serviceType),
    data_request_(r),
    sigwxDescRes_(dr),
    req_type_(rAvGetSigwx)
{
}


MeteoDataService::MeteoDataService(const DataRequest* r,StationsDataReply* dr, google::protobuf::Closure* done, int serviceType)
  : TBaseDataService(done,serviceType),
    data_request_(r),
    statReply_(dr),
    req_type_(rAvStations)
{
}



MeteoDataService::~MeteoDataService(){
  // debug_log<<"MeteoDataService поток "<< this <<" завершил работу(destructor)";
}

void MeteoDataService::run() {

  //debug_log<<"MeteoDataService поток "<< this <<" начал работу(run)";
  QElapsedTimer ttt; ttt.start();
  bool ret_val=false;
  switch(req_type_){
    case   rRunService:
      if(0 != data_reply_ && 0!=data_request_) {
        ret_val = runService(data_request_,data_reply_);
      }
    break;
    case   rGetMeteoData:
      if(0 != data_reply_ && 0!=data_request_) {
        ret_val = getMeteoData(data_request_,data_reply_);
      }
    break;
    case   rGetDataCount:
      if ( nullptr != count_resp_ && nullptr != data_request_ ) {
        ret_val = getDataCount(data_request_, count_resp_ );
      }
    break;
    case   rGetMeteoDataTree:
      //      debug_log << "get meteo_data tree" << this;
      if(0 != data_reply_ && 0!=data_request_) {
        ret_val = getMeteoDataTree(data_request_,data_reply_);
      }
    break;
    case rGetSigwx:
      if ( 0 != sigwx_resp_ && 0 != sigwx_request_ ) {
        ret_val = getSigwx( sigwx_request_, sigwx_resp_ );
      }
    break;
    case rAvGetSigwx:
      if ( 0 != data_request_ && 0 != sigwxDescRes_ ) {
        ret_val = getAvailableSigwx( data_request_, sigwxDescRes_ );
      }
    break;
    case rAvStations:
      if ( 0 != data_request_ && 0 != statReply_ ) {
        ret_val = getAvailableStations( data_request_, statReply_ );
      }
    break;
    case rGetDts:
      if(0 != data_reply_ && 0!=data_request_) {
        ret_val = getDtsForPeriod(data_request_,data_reply_);
      }
    break;
    case rGetDataCover:
      if(0 != data_reply_ && 0!=data_request_) {
        ret_val = getDataCover(data_request_,data_reply_);
      }
    break;
    default:
    break;
  }

  if(!ret_val){
    if(0 != data_reply_ ) {
      if ( !data_reply_->has_comment() ) {
        data_reply_->set_comment(tr("Запрос задан некорректно или ошибка выполнения запроса").toUtf8().constData());
      }
      data_reply_->set_result(false);
    }
  }
  int cur =  ttt.elapsed();
  if(1000<cur){
    debug_log << QString::fromUtf8("Время обработки запроса GetValueOnStation: %1 мсек").arg(cur)<< "req_type_"<<req_type_;
  }
  //  debug_log<<"MeteoDataService поток "<< this <<" завершил работу(run)";
  //  debug_log << "EMIT FINISHED" << this;
  emit finished(this);
}

bool MeteoDataService::getDataCover(const DataRequest* req, DataReply* resp)
{
  QString error;

  auto return_func = [&error, resp]( bool fl ) {
    resp->set_result(fl);
    resp->set_comment( error.toStdString() );
    if ( false == fl ) {
      error_log << error;
    }
    return fl;
  };

  if ( false == req->has_date_start() ||
       (0 == req->type_size() && 0 == req->station_type_size() )) {
    error = QObject::tr("Запрос не заполнен");
    return return_func(false);
  }
  std::unique_ptr<Dbi> db(meteo::global::dbMeteo());
  if ( nullptr == db.get() ) { return_func(false); }
  auto query = db->queryptrByName("get_data_cover");
  if(nullptr == query) {return return_func(false);}
  query->argDt("dt", req->date_start());
  if(0 < req->station_type_size()){
    query->arg("station_type", req->station_type(0));
  }
  if(0 < req->type_size()){
    query->arg("data_type", req->type(0));
  }
  if ( false == query->exec() ) {
    error = QObject::tr("Не удается выполнить запрос в базу данных");
    return return_func(false);
  }
  const meteo::DbiEntry& result = query->result();
  bool ok = true;
  double resval = result.valueDouble("result", &ok);
  if (false == ok){
    error = QObject::tr("Ошибка при выполнении запроса в базу данных");
    return return_func(false);
  }
  resp->set_value(resval);
  return return_func(true);
}

bool MeteoDataService::getDtsForPeriod(const DataRequest *req, DataReply *resp)
{
  QString error;

  auto return_func = [&error, resp]( bool fl ) {
    resp->set_result(fl);
    resp->set_comment( error.toStdString() );
    if ( false == fl ) {
      error_log << error;
    }
    return fl;
  };

  if ( 0 == req->station_size() || false == req->has_date_start() || false == req->has_date_end() ) {
    error = QObject::tr("Запрос не заполнен");
    return return_func(false);
  }

  std::unique_ptr<Dbi> db(meteo::global::dbMeteo());
  if ( nullptr == db.get() ) { return_func(false); }
  auto query = db->queryptrByName("get_dts_for_period");
  if(nullptr == query) {return return_func(false);}

  query->arg("station", req->station(0));
  query->argDt("start_dt", req->date_start());
  query->argDt("end_dt", req->date_end());
  if ( true == req->has_date_start() ) {
    resp->set_date_start( req->date_start() );
  }
  if ( true == req->has_date_end() ) {
    resp->set_date_end( req->date_end() );
  }


  if(false == query->execInit( &error)){
    return return_func(false);
  }

  QList<QDateTime> dtlist;
  while ( true == query->next()) {
    const DbiEntry& doc = query->entry();
    QDateTime dt = doc.valueDt("dt");
    if ( false == dtlist.contains(dt) ) {
      dtlist.append(dt);
    }
  }
  for ( auto dt : dtlist ) {
    auto mdp = resp->add_meteodata_proto();
    mdp->set_dt(dt.toString(Qt::ISODate).toStdString());
  }
  error = QObject::tr("Запрос количества данных выполнен успешно");
  return return_func(true);
}


bool MeteoDataService::getDataCount( const DataRequest* req, CountDataReply* resp )
{
  QString error;

  auto return_func = [&error, resp]( bool fl ) {
    resp->set_result(fl);
    resp->set_comment( error.toStdString() );
    if ( false == fl ) {
      error_log << error;
    }
    return fl;
  };
  QString queryname = "get_data_count";

  if ( true == req->has_query_type() ) {
    if ( kMaximumValue == req->query_type()
         || kMinimumValue == req->query_type()
         || kAverageValue == req->query_type()
         || kLastValue == req->query_type()) {
      queryname = "get_data_count_max_min";
    }
  }
  std::unique_ptr<Dbi> db(meteo::global::dbMeteo());
  if ( nullptr == db.get() ) { return return_func(false); }
  auto query = db->queryptrByName(queryname);
  if(nullptr == query) {return return_func(false);}
  if(req->has_date_start()){
    query->argDt("start_dt", req->date_start());
  }
  if(req->has_date_end()){
    query->argDt("end_dt", req->date_end());
  }
  if(0 < req->meteo_descrname_size()){
    query->arg("descrname", req->meteo_descrname());
  }
  if ( 0 != req->type_size() ) {
    query->arg("data_type", req->type() );
  }

  if(req->has_type_level()){
    query->arg("level_type", req->type_level() );
  }
  if(req->has_level_p()){
    query->arg("level", req->level_p() );
  }
  debug_log << query->query();

  if(false == query->execInit( &error)){return return_func(false);}

  while ( true == query->next()) {
    const DbiEntry& doc = query->entry();
    QDateTime dt = doc.valueDt("dt");
    CountData* data = resp->add_data();
    data->set_date( dt.toString(Qt::ISODate).toStdString() );
    data->set_count( doc.valueInt32("count") );
    data->set_cover( doc.valueDouble("cover") );
    data->set_level( doc.valueDouble("level") );
  }
  error = QObject::tr("Запрос количества данных выполнен успешно");
  return return_func(true);
}

bool MeteoDataService::runService(const DataRequest* req, DataReply* res)
{
  if ( !req->has_type_level() ) {
    error_log << QObject::tr("Запрос заполнен не полностью");
    res->set_result(false);
    res->set_comment( QObject::tr("Запрос заполнен не полностью").toStdString() );
    return false;
  }

  int typeLevel = ( req->has_type_level() ? req->type_level() : -1 );
  // только одно значение у земли - по индексу, времени и дескриптору
  if ( ( meteodescr::kSurfaceLevel == typeLevel)
       && ( 1 == req->station_size())
       && ( true == req->has_date_start() )
       && ( false == req->has_date_end() )
       && ( ( 1 == req->meteo_descr_size())
            || ( 1 == req->meteo_descrname_size() ) ) )
  {
    return getOneSurfaceData(req,res);
  }

  // несколько значений - по индексу, дескриптору и интервалу времени
  if ( meteodescr::kSurfaceLevel == typeLevel
       && (1 == req->station_size())
       && req->has_date_start()  && req->has_date_end()
       )
  {
    return getManySurfaceData(req,res);
  }
  return false;
}

bool MeteoDataService::meteoDataByCoordQuery(const DataRequest* req, DbiQuery* query)
{
  query->argDt("start_dt", req->date_start());// 1 аргумент
  if ( true == req->has_date_end() ) {
    query->argDt("end_dt", req->date_end());// 2 аргумент
  }
  else {
    query->argDt("end_dt", req->date_start());// 2 аргумент
  }

  if ( 0 != req->station_size() ) {
    query->arg("station", req->station() );// 3 аргумент
  }
  if ( 0 != req->station_type_size() ) {// 4 аргумент
    query->arg("station_type", req->station_type() );
  }
  if ( 0 < req->type_size()) {
    query->arg("data_type", req->type() ); // 5 аргумент
  }
  query->arg("level_type", req->type_level() );// 6 аргумент
  query->arg("level", req->level_p() );// 7 аргумент
  query->arg("descrname", req->meteo_descrname() );// 8 аргумент

  if ( true == req->has_region()){
    if ( 1 == req->region().point_size()) {
      const meteo::surf::Point& p = req->region().point(0);
      GeoPoint gp(p.fi(),p.la());
      query->arg("coords",gp);
      //query->arg("la", p.la()* meteo::RAD2DEG  );// 9 аргумент
      //query->arg("fi", p.fi()* meteo::RAD2DEG  );// 10 аргумент
    } else{
      GeoVector gv;
      for(auto p: req->region().point()){
        GeoPoint gp(p.fi(),p.la());
        gv << gp;
      }
      if(0< gv.size()){
        query->arg("coords",gv);
      }
    }
  }


  //  int max_rast = 100000; //100 km
  if(true == req->has_rast() ){
    int max_rast = req->rast();
    query->arg("max_rast",max_rast);
  }
  //debug_log<< query.query();
  return true;
}

bool MeteoDataService::meteoDataLastValueQuery(const DataRequest* req, DbiQuery* query)
{
  if ( true == req->has_date_end() ) {
    query->argDt("end_dt", req->date_end());
  }
  else {
    QDateTime dte = QDateTime::currentDateTimeUtc();
    dte.setTimeSpec( Qt::LocalTime );
    query->arg("end_dt",dte);
  }
  if ( true == req->has_date_start() ) {
    query->argDt("start_dt", req->date_start());
  }
  else {
    QDateTime dts = QDateTime::currentDateTimeUtc().addSecs(-3*60*60);
    dts.setTimeSpec( Qt::LocalTime );
    query->arg("start_dt",dts);
  }
  if ( true == req->has_country() ) {
    query->arg("country", req->country() );
  }
  if ( 0 != req->station_size() ) {
    query->arg("station",req->station() );
  }
  if ( 0 != req->station_type_size() ) {
    query->arg("station_type",req->station_type() );
  }
  if ( 0 != req->type_size() ) {
    query->arg("data_type", req->type() );
  }

  query->arg("level_type", req->type_level() );
  query->arg("level", req->level_p() );
  query->arg("descrname", req->meteo_descrname() );
  if ( 0 != req->region().point_size() ) {
    GeoVector gv;
    for ( auto pnt : req->region().point() ) {
      gv.append( GeoPoint::fromDegree( pnt.fi(), pnt.la()  ) );
    }
    query->argGeoWithin("location",gv);
  }

  return query;
}


bool MeteoDataService::meteoDataQuery(const DataRequest* req, DbiQuery* query)
{
  query->argDt("start_dt", req->date_start());
  if ( true == req->has_date_end() ) {
    query->argDt("end_dt", req->date_end());
  } else {
    query->argDt("end_dt", req->date_start());
  }
  
  if ( true == req->has_country() ) {
    query->arg("country", req->country() );
  }
  if ( 0 != req->station_size() ) {
    query->arg("station",req->station() );
  }
  if ( 0 != req->station_type_size() ) {
    query->arg("station_type",req->station_type() );
  }
  if ( 0 != req->type_size() ) {
    query->arg("data_type", req->type() );
  }
  query->arg("level_type", req->type_level() );
  if ( true == req->has_level_p() ) {
    query->arg("level", req->level_p() );
  }
  if ( 0 != req->meteo_descrname_size() ) {
    query->arg("descrname", req->meteo_descrname() );
  }
  if ( 0 != req->region().point_size() ) {
    GeoVector gv;
    for ( auto pnt : req->region().point() ) {
      gv.append( GeoPoint::fromDegree( pnt.fi(), pnt.la()  ) );
    }
    query->argGeoWithin("location",gv);
  }
  return true;
}


bool MeteoDataService::meteoDataMaxMinValueQuery(const DataRequest *req, DbiQuery* query)
{
  query->argDt("start_dt", req->date_start());
  if ( true == req->has_date_end() ) {
    query->argDt("end_dt", req->date_end());
  }
  if ( true == req->has_country() ) {
    query->arg("country", req->country() );
  }
  if ( 0 != req->station_size() ) {
    query->arg("station",req->station() );
  }
  if ( 0 != req->station_type_size() ) {
    query->arg("station_type",req->station_type() );
  }
  if ( 0 != req->type_size() ) {
    query->arg("data_type", req->type() );
  }
  query->arg("level_type", req->type_level() );
  if ( true == req->has_level_p() ) {
    query->arg("level", req->level_p() );
  }
  if ( 0 != req->meteo_descrname_size() ) {
    query->arg("descrname", req->meteo_descrname() );
  }

  if ( true == req->has_skip() ) {
    query->arg("skip", req->skip() );
  }
  if ( true == req->has_limit() ) {
    query->arg("limit", req->limit() );

  }
  return true;
}

bool MeteoDataService::meteoDataNearestTimeQuery(const DataRequest* req, DbiQuery* query)
{

  if ( false == req->has_term() ) {
    error_log << QObject::tr("Не указан срок данных");
    return false;
  }

  if ( false == req->has_max_secs() ) {
    error_log << QObject::tr("Не указан диапазон поиска в секундах");
    return false;
  }

  QDateTime term = NosqlQuery::datetimeFromString( req->term() );
  QDateTime dtbeg = term.addSecs( -req->max_secs() );
  QDateTime dtend = term.addSecs( +req->max_secs() );
  query->arg("start_dt", dtbeg);
  query->arg("end_dt", dtend);

  if ( true == req->has_country() ) {
    query->arg("country", req->country() );
  }
  if ( 0 != req->station_size() ) {
    query->arg("station",req->station() );
  }
  if ( 0 != req->station_type_size() ) {
    query->arg("station_type",req->station_type() );
  }
  if ( 0 != req->type_size() ) {
    query->arg("data_type", req->type() );
  }
  query->arg("level_type", req->type_level() );
  if ( true == req->has_level_p() ) {
    query->arg("level", req->level_p() );
  }
  if ( 0 != req->meteo_descrname_size() ) {
    query->arg("descrname", req->meteo_descrname() );
  }
  if ( 0 != req->region().point_size() ) {
    GeoVector gv;
    for ( auto pnt : req->region().point() ) {
      gv.append( GeoPoint::fromDegree( pnt.fi(), pnt.la()  ) );
    }
    query->argGeoWithin("location",gv);
  }
  query->arg("term",term);
  return true;
}

bool MeteoDataService::getMeteoData(const DataRequest* req, DataReply* resp)
{
  //QTime ttt; ttt.start();
  QString error;
  auto return_func = [&error, resp]( bool fl ) {
    resp->set_result(fl);
    resp->set_comment( error.toStdString() );
    if ( false == fl ) {
      error_log << error;
    }
    else if ( 0 != error.size() ) {
      error_log << error;
    }
    return fl;
  };
  //debug_log<<req->DebugString();
  if ( nullptr == req || nullptr == resp ) {
    error = QObject::tr("DataRequest или DataReply равен nullptr");
    return return_func(false);
  }
  std::unique_ptr<Dbi> db(meteo::global::dbMeteo());
  if ( nullptr == db.get() ) {
    error = QObject::tr("Не удается выполнить запрос в базу данных");
    return return_func(false);
  }
  meteo::surf::QueryType qtype = meteo::surf::kTermValue;
  if ( true == req->has_query_type() ) {
    qtype = req->query_type();
  }
  if ( true == req->has_date_end() ) {
    resp->set_date_end( req->date_end() );
  }
  else {
    auto str = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
    str.replace("Z","");
    resp->set_date_end( str.toStdString() );
  }
  if ( true == req->has_date_start() ) {
    resp->set_date_start( req->date_start() );
  }
  else if ( meteo::surf::kLastValue == qtype ) {
    QDateTime dts = QDateTime::fromString( QString::fromStdString(resp->date_start()), Qt::ISODate );
    dts = dts.addSecs(-3*60*60);
    auto str = dts.toString(Qt::ISODate);
    str.replace("Z","");
    resp->set_date_start( str.toStdString() );
  }

  bool result = false;
  std::unique_ptr<DbiQuery> query = nullptr;
  switch (qtype) {
    case meteo::surf::kNearPoCoord:
      query = db->queryptrByName("get_meteo_data_by_coords");
      if(nullptr == query){ return return_func(false); }
      result = meteoDataByCoordQuery(req, query.get());
    break;
    case meteo::surf::kLastValue:
    case meteo::surf::kMaximumValue:
    case meteo::surf::kMinimumValue:
      query = db->queryptrByName("get_meteo_data_by_day");
      if ( nullptr == query){
        return return_func(false);
      }
      result = meteoDataLastValueQuery( req, query.get() );
    break;
    case meteo::surf::kAverageValue:
      query = db->queryptrByName("get_meteo_data_avg_value");
      if(nullptr == query){ return return_func(false); }
      result = meteoDataMaxMinValueQuery( req, query.get());
    break;
    case meteo::surf::kNearestTermValue:
      query = db->queryptrByName("get_meteo_data_nearest_time");
      if(nullptr == query){ return return_func(false); }
      result = meteoDataNearestTimeQuery(req, query.get());
    break;
    default:
      query = db->queryptrByName("get_meteo_data");
      if(nullptr == query){ return return_func(false); }
      result = meteoDataQuery(req, query.get());
  }
  // debug_log << query->query();

  if ( false == result || false == query->execInit(&error) ){
    error="";
    return return_func(false);
  }

  while ( true == query->next()) {
    const DbiEntry& doc = query->entry();
    //debug_log<<doc.jsonExtendedString();
    if ( false == req->as_proto() ) {
      TMeteoData md;
      if ( true == global::json2meteodata( &doc, &md ) ) {
        // md.printData();
        QByteArray buf;
        md >> buf;
        resp->add_meteodata( buf.data(), buf.size() );
      }
    }
    else {
      surf::MeteoDataProto proto;
      if ( true == global::json2meteoproto( &doc, &proto, req->protoparam_as_tree() ) ) {
        resp->add_meteodata_proto()->CopyFrom(proto);
      }
    }
  }
  //debug_log << "Время выполнения: MeteoDataService::getMeteoData:" << ttt.elapsed();
  return return_func(true);
}


void fillDataReplyFromMeteoData(const QHash<QString, QPair<Info, TMeteoData>>& mds, const DataRequest *req, DataReply* resp)
{
  //debug_log << QString("Найдено станций: %1").arg(mds.size());
  if ( true == req->has_date_start() ) {
    resp->set_date_start( req->date_start() );
  }
  if ( true == req->has_date_end() ) {
    resp->set_date_end( req->date_end() );
  }
  foreach(const auto& md, mds)
  {
    if(false == req->as_proto()) {
      QByteArray buf;
      md.second >> buf;
      resp->add_meteodata(buf.data(), buf.size());
    }
    else {
      auto meteodata = resp->add_meteodata_proto();
      //      meteodata->set_index(md.first.station.toStdString());
      meteodata->mutable_station_info()->set_cccc(md.first.station.toStdString());
      meteodata->set_dt(md.first.dt.toString(Qt::ISODate).toStdString());
      meteodata->mutable_gp()->set_la(md.first.lat);
      meteodata->mutable_gp()->set_lo(md.first.lon);

      for(const auto& it: req->meteo_descrname()) {
        QString descrname = QString::fromStdString(it);
        const TMeteoParam& mp = md.second.meteoParam(descrname);
        int64_t descr;
        if(TMeteoDescriptor::instance()->isAdditional(descrname)) {
          auto add = TMeteoDescriptor::instance()->additional(descrname);
          descr = add.descr;
        }
        else {
          descr = TMeteoDescriptor::instance()->descriptor(descrname);
        }

        auto param = meteodata->add_param();
        param->set_code(mp.code().toStdString());
        param->set_value(mp.value());
        param->set_quality(mp.quality());
        param->set_descr(descr);
        param->set_descrname(descrname.toStdString());

        meteodescr::Property prop;
        if(TMeteoDescriptor::instance()->property(descrname, &prop)) {
          QString description = prop.description;
          if(false == prop.unitsRu.isEmpty() && "NO" != prop.unitsRu) {
            description += ", " + prop.unitsRu;
          }
          param->set_description(description.toStdString());
        }
      }
    }
  }
  resp->set_comment("All ok");
  resp->set_result(true);
}

bool MeteoDataService::getSigwx(const SigwxDesc* req, meteo::sigwx::SigWx* resp)
{
  auto return_func = [resp](QString err)
  {
    error_log << err;
    resp->set_result(false);
    resp->set_comment(err.toStdString());
    return false;
  };

  debug_log << "SIGWX REQ =" << req->Utf8DebugString();

  if ( false == req->has_date() ) {
    return return_func(QObject::tr("Запрос составлен некорректно (не указана дата)"));
  }
  std::unique_ptr<Dbi> db(meteo::global::dbMeteo());
  if ( nullptr == db.get() ) { return_func(""); }
  auto query = db->queryptrByName("get_sigwx");
  if(nullptr == query) {return return_func("");}

  QDateTime dt = NosqlQuery::datetimeFromString(req->date());
  if ( dt.isValid() ) {
    query->arg("dt",dt);
  }

  if ( true == req->has_forecast_beg() ) {
    QDateTime dt = NosqlQuery::datetimeFromString(req->forecast_beg());
    if ( dt.isValid() ) {
      query->arg("start_dt",dt);
    }
  }
  if ( true == req->has_forecast_end() ) {
    QDateTime dt = NosqlQuery::datetimeFromString(req->forecast_end());
    if ( dt.isValid() ) {
      query->arg("end_dt",dt);
    }
  }

  if ( true == req->has_center() ) {
    query->arg("center",req->center());
  }

  if ( true == req->has_level_lo() ) {
    query->arg("level_lo",req->level_lo());
  }

  if ( true == req->has_level_hi() ) {
    query->arg("level_hi",req->level_hi());
  }

  GridFs gfs;
  auto conf = global::mongodbConfMeteo();
  if ( false == gfs.connect(conf) ) {
    return return_func(gfs.lastError());
  }
  gfs.use( conf.name(), kPrefix);
  QString error;
  if(false == query->execInit( &error)){
    return return_func(error);
  }

  while ( true == query->next()) {
    const DbiEntry& doc = query->entry();
    bool ok = false;
    QString id = doc.valueOid("fileid", &ok);
    if ( !ok ) {
      error_log << QObject::tr("Не найден идентификатор файла в поле fileid");
      //debug_log << doc.jsonString();
      continue;
    }

    GridFile file = gfs.findOneById(id);
    if ( !file.hasFile() ) {
      error_log << QObject::tr("Файл '%1' не найден").arg(id);
      continue;
    }

    QByteArray arr = file.readAll(&ok);
    if ( !ok ) {
      error_log << QObject::tr("Не удалось загрузить файл %1.").arg(id) << file.lastError();
      continue;
    }
    meteo::sigwx::SigWx s;
    global::arr2protomsg( arr, &s );
    resp->MergeFrom(s);
  }
  resp->set_layer_name( layernameFromInfo(*req).toStdString() );
  resp->set_result(true);

//  auto sigstorm = resp->add_storm();
//  auto header = sigstorm->mutable_header();
//  header->set_center(1);
//  header->set_dt_beg("1");
//  header->set_dt_end("1");
//  header->set_descr(1);
//  header->set_value(1);
//  auto storm = sigstorm->add_storm();
//  auto pnt = storm->mutable_pnt();
//  pnt->set_lat_deg(60);
//  pnt->set_lon_deg(30);
//  storm->set_name("Тестовый");

  return true;
}



bool MeteoDataService::getMeteoDataTree(const DataRequest* req, DataReply* res)
{
  if (req->has_type_level() && 1 == req->station_size() && req->has_date_start()) {
    return getMeteoDataTreeOnStation(req, res);
  }

  return false;
}


bool MeteoDataService::getManySurfaceData(const DataRequest* req, DataReply* resp)
{
  auto return_func = [resp](QString err)
  {
    error_log << err;
    resp->set_result(false);
    resp->set_comment(err.toStdString());
    return false;
  };

  resp->set_result(false);
  if ( true == req->has_date_start() ) {
    resp->set_date_start( req->date_start() );
  }
  if ( true == req->has_date_end() ) {
    resp->set_date_end( req->date_end() );
  }

  QElapsedTimer workTimeCounter; workTimeCounter.start();
  if (req->station_size() <= 0 ||
      !req->has_date_start() ||
      !req->has_date_end() ||
      (req->meteo_descrname_size() == 1 && req->meteo_descr_size() == 1)){
    return return_func(msgIncorrectRequest);
  }

  QStringList stations;
  for (int i = 0; i < req->station_size(); ++i){
    stations << QString::fromStdString(req->station(i));
  }

  QDateTime dtStart = NosqlQuery::datetimeFromString(req->date_start());
  QDateTime dtEnd = NosqlQuery::datetimeFromString(req->date_end());
  QStringList descriptors;
  for (int i = 0; i < req->meteo_descrname_size(); ++i){
    QString mdescr = QString::fromStdString(req->meteo_descrname(i));
    descriptors << mdescr;
  }
  std::unique_ptr<Dbi> db(meteo::global::dbMeteo());
  if ( nullptr == db.get() ) { return_func(""); }
  auto query = db->queryptrByName("meteodata_get_one_surface_data_2");
  if(nullptr == query) {return return_func("");}


  query->arg("start_dt",dtStart);
  query->arg("end_dt",dtEnd);
  query->arg("station",stations);


  if ( 0 != req->station_type_size() ){
    query->arg("station_type",req->station_type());
  }
  if ( 0 != req->type_size() ) {
    query->arg("data_type",req->type());
  }
  if ( 0 != descriptors.size() ) {
    query->arg("descrname",descriptors);
  }
  QString error;
  if(false == query->execInit( &error) ) {
    return return_func(error);
  }
  QList<TMeteoData> mds;

  while ( true == query->next()) {
    const DbiEntry& doc = query->entry();
    if ( true == req->as_proto() ) {
      surf::MeteoDataProto proto;
      if ( true == global::json2meteoproto( &doc, &proto, req->protoparam_as_tree() ) ) {
        resp->add_meteodata_proto()->CopyFrom(proto);
      }
    }
    else {
      TMeteoData md;
      if ( true == global::json2meteodata( &doc, &md ) ) {
        mds << md;
      }
    }
  }
  resp->set_comment(msglog::kServerAnswerOK.toStdString());
  resp->set_result(true);

  if ( true == req->as_proto() ) {
    auto sortDataBydt = [](const surf::MeteoDataProto& a, const surf::MeteoDataProto& b)
    {
      QDateTime dta = QDateTime::fromString(QString::fromStdString(a.dt()), Qt::ISODate);
      QDateTime dtb = QDateTime::fromString(QString::fromStdString(b.dt()), Qt::ISODate);
      return dta < dtb;
    };
    auto datas = resp->mutable_meteodata_proto();
    std::sort(datas->begin(), datas->end(), sortDataBydt);
  }
  else {
    auto sortDataByDt = [](const TMeteoData& a, const TMeteoData& b)
    {
      QDateTime dta = TMeteoDescriptor::instance()->dateTime(a);
      QDateTime dtb = TMeteoDescriptor::instance()->dateTime(b);
      return dta < dtb;
    };
    std::sort(mds.begin(), mds.end(), sortDataByDt);
    for ( auto md : mds ) {
      QByteArray buf;
      md >> buf;
      resp->add_meteodata( buf.constData(), buf.size() );
    }
  }

  debug_log << "Время выполнения: MeteoDataServiceMongo::getManySurfaceData2:" << workTimeCounter.elapsed();
  return true;
}

bool MeteoDataService::getOneSurfaceData(const DataRequest* req, DataReply* resp)
{
  auto return_func = [resp](QString err)
  {
    error_log << err;
    resp->set_result(false);
    resp->set_comment(err.toStdString());
    return false;
  };

  if ( true == req->has_date_start() ) {
    resp->set_date_start( req->date_start() );
  }
  if ( true == req->has_date_end() ) {
    resp->set_date_end( req->date_end() );
  }
  resp->set_result(false);

  if ( req->meteo_descr_size() + req->meteo_descrname_size() != 1 ) {
    return return_func(tr("Запрос составлен некорректно: необходимо указать один дескриптор"));
  }

  QDateTime dt = NosqlQuery::datetimeFromString(req->date_start());
  QString station = QString::fromStdString(req->station(0));
  QString meteoDescriptorString = req->meteo_descrname_size() == 1? QString::fromStdString(req->meteo_descrname(0)) :
                                                                    TMeteoDescriptor::instance()->name(req->meteo_descr(0));
  int descriptor = TMeteoDescriptor::instance()->descriptor(meteoDescriptorString);
  std::unique_ptr<Dbi> db(meteo::global::dbMeteo());
  if ( nullptr == db.get() ) { return_func(""); }
  auto query = db->queryptrByName("meteodata_get_one_surface_data_1");
  if(nullptr == query) {return return_func("");}


  query->arg("start_dt",dt);
  query->arg("station",station);
  query->arg("descrname",meteoDescriptorString);
  QString error;
  if(false == query->execInit( &error)){
    return return_func(error);
  }

  TMeteoData md;
  md.setDateTime(dt);
  md.setStation(station);

  if (query->next()){
    const DbiEntry& doc = query->entry();
    QString code = doc.valueString("param.code");
    double value = doc.valueDouble("param.value");
    int quality = doc.valueInt32("param.quality");
    int station_type = doc.valueInt32("station_type");
    int level_type = doc.valueInt32("level_type");
    int dataType = doc.valueInt32("data_type");
    if ( true == req->as_proto() ) {
      surf::MeteoDataProto* mdProto = resp->add_meteodata_proto();
      QString station = doc.valueString("station");
      mdProto->mutable_station_info()->set_cccc(station.toStdString());
      if (true == doc.hasField("station_info")) {
        if (true == doc.hasField("station_info.country_ru")){
          QString countryRu = doc.valueString("station_info.country_ru");
          mdProto->mutable_station_info()->set_country_rus(countryRu.toStdString());
        }
        if (true == doc.hasField("station_info.country_en")){
          QString countryEn = doc.valueString("station_info.country_en");
          mdProto->mutable_station_info()->set_country_eng(countryEn.toStdString());
        }
        if (true == doc.hasField("station_info.name_ru")){
          QString nameRu = doc.valueString("station_info.name_ru");
          mdProto->mutable_station_info()->set_name_rus(nameRu.toStdString());
        }
        if (true == doc.hasField("station_info.name")){
          QString nameEng = doc.valueString("station_info.name");
          mdProto->mutable_station_info()->set_name_eng(nameEng.toStdString());
        }
        if (true == doc.hasField("station_info.country")){
          double countryCode = doc.valueDouble("station_info.country");
          mdProto->mutable_station_info()->set_country_code(countryCode);
        }
      }
      mdProto->set_dt( doc.valueDt("dt").toString(Qt::ISODate).toStdString() );
      GeoPoint coords = doc.valueGeo("location");
      mdProto->mutable_gp()->set_la( coords.lat() );
      mdProto->mutable_gp()->set_lo( coords.lon() );

      QString descr =  doc.valueString("param.descrname");
      QString dt_beg = doc.valueDt("param.dt_beg").toString(Qt::ISODate);
      QString dt_end = doc.valueDt("param.dt_end").toString(Qt::ISODate);
      surf::MeteoParamProto* param = mdProto->add_param();
      param->set_code( code.toStdString() );
      param->set_value( value );
      param->set_quality( quality );
      param->set_descrname( descr.toStdString() );
      if (( mdProto->dt() != dt_beg.toStdString() )
          || ( mdProto->dt() != dt_end.toStdString() )) {
        param->set_dt_beg( dt_beg.toStdString() );
        param->set_dt_end( dt_end.toStdString() );
      }
//      if ( true == req->duplication()) {
//        if ( true == doc.hasField("param.dubl")) {
//          Array dubls = doc.valueArray("param.dubl");
//          while (true == dubls.next()) {
//            surf::MeteoParamProto* dubl = param->add_dubl();
//            Document dublDoc;
//            dubls.valueDocument( &dublDoc );
//            double dvalue = dublDoc.valueDouble("value");
//            int dqual = dublDoc.valueInt32("quality");
//            QString dcode = dublDoc.valueString("code");
//            int dt_type = dublDoc.valueInt32("dt_type");
//            QString dt_beg_dubl = dublDoc.valueDt("dt_beg").toString(Qt::ISODate);
//            QString dt_end_dubl = dublDoc.valueDt("dt_end").toString(Qt::ISODate);
//            dubl->set_value(dvalue);
//            dubl->set_quality(dqual);
//            dubl->set_code(dcode.toStdString());
//            dubl->set_dt_type(dt_type);
//            if ( ( mdProto->dt() != dt_beg_dubl.toStdString() )
//                 || ( mdProto->dt() != dt_end_dubl.toStdString() )) {
//              dubl->set_dt_beg(dt_beg_dubl.toStdString());
//              dubl->set_dt_end(dt_end_dubl.toStdString());
//              //            dubl->set_dt_beg(dt_beg.toStdString());
//              //            dubl->set_dt_end(dt_end.toStdString());
//            }
//          }
//        }
//      }
    }
    else {
      md.add(descriptor, TMeteoParam(code, value , static_cast<control::QualityControl>(quality)), 0);
      md.set(TMeteoDescriptor::instance()->descriptor("station_type"),
             TMeteoParam(QString::number(station_type), station_type, control::RIGHT));
      GeoPoint gp = doc.valueGeo("location");
      md.setCoord(gp.fi(),gp.la(),gp.alt());
      md.set(TMeteoDescriptor::instance()->descriptor("level_type"),
             TMeteoParam(QString::number(level_type), level_type, control::RIGHT));
      md.set(TMeteoDescriptor::instance()->descriptor("category"),
             TMeteoParam(QString::number(dataType), dataType, control::RIGHT));
      QByteArray buf;
      md >> buf;
      resp->add_meteodata(buf.data(),buf.size());
    }
  }
  else {
    QString errStr = QString("MeteoDataServiceMongo::getOneSurfaceData1 в ответе отсутствует документ");
    return return_func( errStr );
  }
  resp->set_result(true);
  return true;
}


//! Чтение TMeteoData из файла
bool MeteoDataService::readData(const QString& path, TMeteoData* data, QString* err)
{
  QFile file(path);

  if (!file.open(QIODevice::ReadOnly)) {
    error_log << QObject::tr("Ошибка открытия файла '%1'").arg(path);
    *err = QObject::tr("Ошибка открытия файла '%1'").arg(path);
  }
  QByteArray ba = qUncompress(file.readAll());

  file.close();

  (*data) << ba;

  return true;
}

//! С восстановлением иерархии, по индексу, дате, типу данных (без учета уровней)
bool MeteoDataService::getMeteoDataTreeOnStation(const DataRequest* req, DataReply* resp)
{
  auto return_func = [resp](QString err)
  {
    error_log << err;
    resp->set_result(false);
    resp->set_comment(err.toStdString());
    return false;
  };

  if ( true == req->has_date_start() ) {
    resp->set_date_start( req->date_start() );
  }
  if ( true == req->has_date_end() ) {
    resp->set_date_end( req->date_end() );
  }
  resp->set_result(false);

  if (!req->has_date_start()   ||  !req->has_type_level() ||
      req->station_size() == 0 ||  req->station_type_size() == 0 ||
      req->meteo_descrname_size() == 0) {
    return return_func(QObject::tr("Запрос заполнен не полностью"));
  }

  QDateTime dt = NosqlQuery::datetimeFromString(req->date_start());
  //  QString station = QString::fromStdString(req->station(0));

  std::unique_ptr<Dbi> db(meteo::global::dbMeteo());
  if ( nullptr == db.get() ) { return_func(""); }
  auto query = db->queryptrByName("get_one_meteo_data");
  if(nullptr == query) {return return_func("");}

  query->arg("start_dt",dt);
  query->arg("station", req->station());
  query->arg("station_type",req->station_type());
  query->arg("level_type",req->type_level());
  query->arg("descrname",req->meteo_descrname());
  QString error;
  if(false == query->execInit( &error)){
    return return_func(error);
  }
  bool exist = false;
  ComplexMeteo md;
  md.setDtType(meteodescr::kNormalTime);
  md.setDt1(dt);
  md.setDt2(dt);

  while (query->next()){
    const DbiEntry& doc = query->entry();

    ComplexMeteo* cur = &md;
    //if (!doc.valueString("_id.uuid").isEmpty()) {
    cur = &md.addChild();
    descr_t complDescr = BAD_DESCRIPTOR_NUM;
    //}
    Array params = doc.valueArray("param");
    while (true == params.next()) {
      exist = true;
      Document pdoc;
      params.valueDocument( &pdoc );

      if (pdoc.hasField("dt_type")) {
        cur->setDtType(pdoc.valueInt32("dt_type"));
      }
      if (pdoc.hasField("dt_beg")) {
        cur->setDt1(pdoc.valueDt("dt_beg"));
      }
      if (pdoc.hasField("dt_end")) {
        cur->setDt2(pdoc.valueDt("dt_end"));
      }
      if (complDescr == BAD_DESCRIPTOR_NUM) {
        if (TMeteoDescriptor::instance()->isComponent(pdoc.valueString("descrname"), &complDescr)) {
          cur->setDescriptor(complDescr);
        }
      }
      control::QualityControl qual = control::QualityControl(pdoc.valueInt32("quality"));
      if (qual < control::DOUBTFUL) {
        cur->add(pdoc.valueString("descrname"), TMeteoParam(pdoc.valueString("code"),
                                                            pdoc.valueDouble("value"),
                                                            qual));
      }
    }
  }

  if (exist) {
    QByteArray buf;
    md >> buf;
    resp->add_meteodata(buf.data(),buf.size());
    resp->set_comment(msglog::kServerAnswerOK.toStdString());
  } else {
    resp->set_comment(QObject::tr("Нет данных").toStdString());
  }
  resp->set_result(true);
  return true;
}

bool MeteoDataService::getAvailableSigwx(const DataRequest* req, SigwxDescResponse* res)
{
  QString err;

  auto return_func = [&err, res](bool fl) {
    res->set_result(fl);
    res->set_comment(err.toStdString());
    if ( false == fl ) {
      error_log << err;
    }
    else if ( 0 != err.size() ) {
      info_log << err;
    }
    return fl;
  };

  if(false == req->has_date_start()) {
    err = QObject::tr("Не указан период отбора данных");
    return return_func(false);
  }
  auto dts = req->date_start();
  std::string dte;
  if ( false == req->has_date_end() ) {
    dte = req->date_start();
  }
  else {
    dte = req->date_end();
  }

  std::unique_ptr<Dbi> db(meteo::global::dbMeteo());
  if ( nullptr == db.get() ) { return_func(""); }
  auto query = db->queryptrByName("get_available_sigwx");
  if(nullptr == query) {return return_func("");}

  query->argDt("date_start",dts);
  query->argDt("date_end",dte);

  if(false == query->execInit( &err)){
    return return_func(false);
  }
  while( true == query->next() ) {
    const DbiEntry& doc = query->entry();
    surf::SigwxDesc* desc = res->add_sigwx();
    QDateTime dt = doc.valueDt("_id.dt");
    desc->set_date(dt.toString(Qt::ISODate).toStdString());
    dt = doc.valueDt("_id.dt_beg");
    desc->set_forecast_beg(dt.toString(Qt::ISODate).toStdString());
    dt = doc.valueDt("_id.dt_end");
    desc->set_forecast_end(dt.toString(Qt::ISODate).toStdString());
    desc->set_center( doc.valueInt32("_id.center") );
    desc->set_level_lo( doc.valueDouble("_id.level_lo") );
    desc->set_level_hi( doc.valueDouble("_id.level_hi") );
    desc->set_count( doc.valueInt32("count") );
    Array arr = doc.valueArray("fileid");
    while ( true == arr.next() ) {
      //      desc->add_filepath( arr.valueString().toStdString() );
      desc->add_file_id( arr.valueOid().toStdString() );
    }
  }
  return return_func(true);
}
//! Станции с данными по дате и типу данных
bool MeteoDataService::getAvailableStations(const DataRequest* req, StationsDataReply* res)
{
  auto return_func = [res](QString err)
  {
    error_log << err;
    res->set_result(false);
    res->set_comment(err.toStdString());
    return false;
  };


  if(false == req->has_date_start() || 0 == req->type_size()) {
    return return_func(QObject::tr("Запрос заполнен не полностью"));
  }

  std::string dtend;
  if (req->has_date_end()) {
    dtend = req->date_end();
  } else {
    dtend = req->date_start();
  }

  std::unique_ptr<Dbi> db(meteo::global::dbMeteo());
  if ( nullptr == db.get() ) { return_func(""); }
  auto query = db->queryptrByName("get_available_stations");
  if(nullptr == query) {return return_func("");}

  query->argDt("date_start",req->date_start());
  query->argDt("date_end",dtend);
  query->arg("data_type",req->type());

  QString error;
  if ( false == query->execInit( &error) ) {
    return return_func(error);
  }

  QStringList stations;
  meteo::sprinf::MeteostationType station_type = meteo::sprinf::kStationUnk;
  while ( true == query->next() ) {
    const DbiEntry& doc = query->entry();
    //  Document doc = entry.valueDocument("_id"); //FIXME
    meteo::sprinf::MeteostationType type = meteo::sprinf::MeteostationType(doc.valueInt32("station_type"));
    if (type > meteo::sprinf::kStationUnk) {
      station_type = type;
      Array arr = doc.valueArray("stations");
      while(arr.next()) {
        stations.append(arr.valueString());
      }
    }
  }

  meteo::sprinf::Stations stations_cache;
  meteo::sprinf::MultiStatementRequest reqStation;

  if( !meteo::global::loadStations(reqStation, &stations_cache) ) {
    return return_func(QObject::tr("Ошибка чтения кеша станций"));
  }

  QHash<QString,int> hashStations;
  hashStations.reserve(stations_cache.station_size());
  for(int j = 0, szs = stations_cache.station_size(); j < szs; ++j) {
    QString key = QString::number(stations_cache.station(j).type()) + "_" +
                  QString::fromStdString(stations_cache.station(j).station());
    hashStations.insert(key,j);
  }

  QStringList::iterator it;
  for(it = stations.begin(); it != stations.end(); ++it) {
    QString key = QString::number(station_type).append("_").append(*it);
    int j = hashStations.value(key);
    surf::Point* pnt = res->add_coords();
    pnt->set_index((*it).toStdString());
    pnt->set_eng_name(stations_cache.station(j).name().international());
    pnt->set_name(stations_cache.station(j).name().rus());
    pnt->set_fi(stations_cache.station(j).position().lat_radian());
    pnt->set_la(stations_cache.station(j).position().lon_radian());
  }
  res->set_result(true);
  return true;
}
}
}
