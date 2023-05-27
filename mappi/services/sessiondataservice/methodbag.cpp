#include "methodbag.h"

#include <cross-commons/debug/tlog.h>
#include <meteo/commons/global/global.h>
#include <mappi/global/global.h>
#include <sql/nosql/nosqlquery.h>
#include <sql/dbi/dbi.h>
#include <sql/dbi/dbiquery.h>

#include <gdal/gdal_priv.h>
#include <gdal/gdal_alg.h>
// #include <gdal/ogr_spatialref.h>
// #include <gdal/ogr_geometry.h>
// #include <gdal/ogr_featurestyle.h>
// #include <gdal/gdalwarper.h>

TMap<int, QString> themeTypes = TMap<int, QString>()
<< QPair<int, QString>(0, "Неизвестный тип")
<< QPair<int, QString>(1, "Оттенки серого")
<< QPair<int, QString>(2, "NDVI")
<< QPair<int, QString>(3, "Псевдоцвета");

namespace mappi {

const QString sessionSavePath()      { return MnCommon::varPath("mappi") + "sessions/"; }
const QString pretreatmentSavePath() { return MnCommon::varPath("mappi") + "pretreatment/"; }
const QString thematicSavePath()     { return MnCommon::varPath("mappi") + "thematic/"; }

MethodBag::MethodBag(SessionDataService *s):
  QObject(),
  service_(s)
{
}

MethodBag::~MethodBag()
{
}

void MethodBag::SaveSession( QPair< meteo::rpc::ClientHandler*, const mappi::proto::SessionData*> param, mappi::proto::Response* resp )
{
  auto return_func = [resp](QString err) {
    error_log << err;
    resp->set_result(false);
    resp->set_comment(err.toStdString());
    // return false;
  };

  const mappi::proto::SessionData* req = param.second;

  std::unique_ptr<meteo::Dbi> db(meteo::global::dbMappi());
  if ( nullptr == db.get() ) {
    return return_func(tr("Ошибка подключения к базе данных mappidb"));
  }

  auto query = db->queryptrByName("session_insert");
  if (nullptr == query) {
    return return_func(tr("Ошибка загрузки запроса '%1'").arg("update_session"));
  }

  query->arg("satellite", req->sat_name());
  query->arg("site", req->site().name());
  query->argDt("date_start", req->date_start());
  query->argDt("date_end", req->date_end());
  query->arg("fpath", req->server_path());
  query->arg("tle", req->tle());

  //TODO заполнить параметры в rpc
  query->arg("date_start_offset", 0);
  query->arg("revol", 0);
  query->arg("direction", 0);
  query->arg("elevat_max", 0);

  QString error;
  if ( !query->execInit(&error) ) {
    return return_func(tr("Ошибка выполнения запроса update_session"));
  }

  if (false == query->next()) {
    return return_func(tr("Ошибка выполнения запроса update_session"));
  }

  const meteo::DbiEntry& doc = query->result();
  resp->set_index(doc.valueInt32("session_insert"));
  resp->set_result(true);
}

void MethodBag::SavePretreatment( QPair< meteo::rpc::ClientHandler*, const mappi::proto::Pretreatment*> param, mappi::proto::Response* resp )
{
  auto return_func = [resp](QString err) {
    error_log << err;
    resp->set_result(false);
    resp->set_comment(err.toStdString());
    // return false;
  };

  const mappi::proto::Pretreatment* req = param.second;

  //сохранение файла производится модулем, выполняющим предварительную обработку
  //(он выполняется на той же машине, что и данный сервис)

  if (!req->has_session_id()
      || 0 == req->channel_size()
      || !req->has_instrument_type()
      || !req->has_calibration()
      || !req->has_path()
      || !req->has_date_start()
      || !req->has_date_end()) {

    return_func (tr("Заданы не все необходимые параметры"));
    return;
  }

  std::unique_ptr<meteo::Dbi> db(meteo::global::dbMappi());
  if ( nullptr == db.get() ) {
    return return_func(tr("Ошибка подключения к базе данных mappidb"));
  }

  auto query = db->queryptrByName("pretreatment_insert");
  if (nullptr == query) {
    return return_func(tr("Не найден тип запроса: '%1'").arg("pretreatment_insert"));
  }

  query->arg("session", req->session_id()); //TODO oid
  query->arg("instrument_id", req->instrument_type());
  query->arg("channel_number", req->channel(0));
  query->argDt("date_start", req->date_start());
  query->argDt("date_end", req->date_end());
  query->arg("calibration", req->calibration());
  query->arg("fpath", req->path());

  QString error;
  if ( !query->execInit(&error) ) {
    return return_func(tr("Ошибка выполнения запроса pretreatment_insert: '%1'").arg(error));
  }

  resp->set_result(true);
}

void MethodBag::GetAvailableSessions( QPair< meteo::rpc::ClientHandler*, const mappi::proto::SessionData*> param,
                                      mappi::proto::SessionList* resp )
{
  auto return_func = [resp](QString err)
  {
    error_log << err;
    resp->set_result(false);
    resp->set_comment(err.toStdString());
    // return false;
  };

  const mappi::proto::SessionData* req = param.second;

  //var(req->Utf8DebugString());

  std::unique_ptr<meteo::Dbi> db(meteo::global::dbMappi());
  if ( nullptr == db.get() ) {
    return return_func(tr("Ошибка подключения к базе данных Mappi "));
  }

  auto query = db->queryptrByName("get_available_sessions");
  if (nullptr == query) {
    return return_func(tr("Ошибка загрузки запроса '%1'").arg("get_available_sessions"));
  }

  if (req->has_date_start()) {
    query->argDt("date_start",  req->date_start());
  }
  if (req->has_date_end()) {
    query->argDt("date_end",  req->date_end());
  }
  if (req->has_sat_name()) {
    query->arg("satellite_name", req->sat_name());
  }
  if (req->has_processed()) {
    query->arg("preprocessed", req->processed());
  }
  if (req->site().has_name()) {
    query->arg("site", req->site().name());
  }
  if (req->has_thematic_done()) {
    query->arg("thematic_done", req->thematic_done());
  }

  //    var(query->query());

  QString error;
  if ( !query->execInit(&error) ) {
    return return_func(tr("Ошибка выполнения запроса get_available_sessions"));
  }
  while (query->next()) {
    const meteo::DbiEntry& doc = query->entry();
    proto::SessionData session;
    if (false == sessionFromQuery(doc, &session)) {
      continue;
    }
    resp->add_sessions()->CopyFrom(session);
  }

  resp->set_result(true);

}

void MethodBag::GetSession( QPair< meteo::rpc::ClientHandler*, const mappi::proto::SessionData*> param,
                            mappi::proto::SessionList* resp )
{
  auto return_func = [resp](QString err)
  {
    error_log << err;
    resp->set_result(false);
    resp->set_comment(err.toStdString());
    // return false;
  };
  const mappi::proto::SessionData* req = param.second;

  if (!req->has_id()) {
    return return_func(tr("Запрос заполнен не корректно"));
  }

  std::unique_ptr<meteo::Dbi> db(meteo::global::dbMappi());
  if ( nullptr == db.get() ) {
    return return_func(tr("Ошибка подключения к базе данных mappidb"));
  }

  //    var(req->Utf8DebugString());

  auto query = db->queryptrByName("get_session");
  if (nullptr == query) {
    return return_func(tr("Ошибка загрузки запроса '%1'").arg("get_session"));
  }

  query->arg("index", req->id());

  QString error;
  if ( !query->execInit(&error) ) {
    return return_func(tr("Ошибка выполнения запроса get_session"));
  }

  var(query->query());

  if (false == query->next()) {
    return return_func(tr("Ошибка выполнения запроса get_session"));
  }

  const meteo::DbiEntry& doc = query->entry();
  proto::SessionData session;
  if (false == sessionFromQuery(doc, &session)) {
    return return_func(tr("Ошибка выполнения запроса get_session"));
  }
  resp->add_sessions()->CopyFrom(session);
  resp->set_result(true);

  var(resp->Utf8DebugString());
}


bool MethodBag::sessionFromQuery(const meteo::DbiEntry &result, proto::SessionData* session)
{
  session->set_sat_name(result.valueString("satellite").toStdString());
  session->set_id(result.valueInt64("id"));
  session->set_date_start(result.valueDt("date_start").toString( Qt::ISODate ).toStdString());
  session->set_date_end(  result.valueDt("date_end" ).toString( Qt::ISODate ).toStdString());
  mappi::conf::Site site;
  site.set_name(result.valueString("site" ).toStdString());
  session->mutable_site()->CopyFrom(site); //TODO ?
  //    session->set_processed(result.valueBool("preprocessed"));
  session->set_server_path( result.valueString("fpath").toStdString());
  if ( false == result.valueString("tle").isEmpty()) {
    session->set_tle(result.valueString("tle").toStdString());
  }

  return true;
}


void MethodBag::MarkSessionThematicDone( QPair< meteo::rpc::ClientHandler*, const mappi::proto::SessionData*> param,
                                         mappi::proto::Response* resp )
{
  auto return_func = [resp](QString err)
  {
    error_log << err;
    resp->set_result(false);
    resp->set_comment(err.toStdString());
    // return false;
  };

  const mappi::proto::SessionData* req = param.second;

  std::unique_ptr<meteo::Dbi> db(meteo::global::dbMappi());
  if ( nullptr == db.get() ) {
    return return_func(tr("Ошибка подключения к базе данных mappidb"));
  }

  if ( !req->has_id() ) {
    return_func(tr("Не задан идентификатор сессии"));
    return;
  }

  bool flag = false;
  if ( !req->has_thematic_done() ) {
    resp->set_comment(QObject::tr("Флаг не задан, будет установлено значение по умолчанию (FALSE)").toStdString());
  }
  else {
    flag = req->thematic_done();
  }

  auto query = db->queryptrByName("mark_session_thematic_done");
  if (nullptr == query) {
    return return_func(tr("Ошибка загрузки запроса '%1'").arg("mark_session_thematic_done"));
  }
  query->arg("thematic_done", flag);
  query->arg("index", req->id());

  QString error;
  if ( !query->execInit(&error) ) {
    return return_func(tr("Ошибка выполнения запроса mark_session_thematic_done"));
  }

  resp->set_result(true);
}

void MethodBag::GetAvailablePretreatment( QPair< meteo::rpc::ClientHandler*, const mappi::proto::Pretreatment*> param,
                                          mappi::proto::PretreatmentList* resp )
{
  auto return_func = [resp](QString err)
  {
    error_log << err;
    resp->set_result(false);
    resp->set_comment(err.toStdString());
    // return false;
  };

  const mappi::proto::Pretreatment* req = param.second;

  std::unique_ptr<meteo::Dbi> db(meteo::global::dbMappi());
  if ( nullptr == db.get() ) {
    return return_func(tr("Ошибка подключения к базе данных mappidb"));
  }

  auto query = db->queryptrByName("get_available_pretreatment");
  if (nullptr == query) {
    return return_func(tr("Ошибка загрузки запроса '%1'").arg("get_available_pretreatment"));
  }

  if (req->has_date_start()) {
    query->argDt("date_start",  req->date_start());
  }
  if (req->has_date_end()) {
    query->argDt("date_end",  req->date_end());
  }
  if (req->has_session_id()) {
    query->arg("session_id",  req->session_id());
  }
  if (req->has_sat_name()) {
    query->arg("satellite", req->sat_name());
  }
  if ( req->has_instrument_type() ) {
    query->arg("instrument_type", req->instrument_type());
    if ( req->channel_size() != 0 ) {
      query->arg("channel", req->channel());
    }
  }
  if ( req->channel_alias_size() != 0) {
    query->arg("channel_alias", req->channel_alias());
  }
  if ( req->has_calibration() ) {
    query->arg("calibration", req->calibration());
  }

  var(query->query());

  QString error;
  if ( !query->execInit(&error) ) {
    return return_func(tr("Ошибка выполнения запроса get_available_pretreatment"));
  }
  while (query->next()) {

    const meteo::DbiEntry& doc = query->entry();

    //var(doc.jsonExtendedString())    ;
    proto::Pretreatment pr;
    pr.set_date_start(doc.valueDt("date_start").toString( Qt::ISODate ).toStdString());
    pr.set_date_end(doc.valueDt("date_end" ).toString( Qt::ISODate ).toStdString());
    pr.set_calibration(doc.valueBool("calibration"));
    pr.set_path(doc.valueString("fpath").toStdString());
    pr.add_channel(doc.valueInt32("channel_number"));
    pr.add_channel_alias(doc.valueString("channel_alias").toStdString());
    pr.set_instrument_type(mappi::conf::InstrumentType(doc.valueInt32("instrument_id")));
    pr.set_session_id(doc.valueInt32("session_id")); //TODO oid?
    pr.set_sat_name(doc.valueString("satellite").toStdString());

    resp->add_pretreatments()->CopyFrom(pr);
  }

  //var(resp->Utf8DebugString());

  resp->set_result(true);
}


//new
void MethodBag::GetAvailableThematic( QPair< meteo::rpc::ClientHandler*, const mappi::proto::ThematicData*> param,
                                      mappi::proto::ThematicList* resp )
{
  auto return_func = [resp](QString err)
  {
    error_log << err;
    resp->set_result(false);
    resp->set_comment(err.toStdString());
    // return false;
  };

  const mappi::proto::ThematicData* req = param.second;

  //var(req->Utf8DebugString()) ;

  std::unique_ptr<meteo::Dbi> db(meteo::global::dbMappi());
  if ( nullptr == db.get() ) {
    return return_func(tr("Ошибка подключения к базе данных mappidb"));
  }

  auto query = db->queryptrByName("get_available_thematic");
  if (nullptr == query) {
    return return_func(tr("Ошибка загрузки запроса '%1'").arg("get_available_thematic"));
  }

  if (req->has_date_start()) {
    query->argDt("date_start",  req->date_start());
  }
  //важно, что в запросе указывается именно date_start
  //таким образом, при запросе доступных тематических обработок
  //date_end выступает не как дата окончания сессии, а как конечная точка интервала дат начала
  //искомых сессий
  if (req->has_date_end()) {
    query->argDt("date_end",  req->date_end());
  }
  if (req->has_satellite_name()) {
    query->arg("satellite_name", req->satellite_name());
  }
  if (req->has_type()) {
    query->arg("type", req->type());
  }
  if ( req->has_instrument_type() ) {
    query->arg("instrument_type", req->instrument_type());
  }
  // if ( req->has_channel_number() ) {
  //   query->arg("channel_number", req->channel_number());
  // }
  // if ( req->has_channel_alias() ) {
  //   query->arg("channel_alias", req->channel_alias());
  // }
  if ( req->has_session_id() ) {
    query->arg("session_id", req->session_id());
  }

  if ( req->has_projection() ) {
    query->arg("projection", req->projection());
  }
  if ( req->has_format() ) {
    query->arg("format", req->format());
  }

  //var(query->query());

  QString error;
  if ( !query->execInit(&error) ) {
    return return_func(tr("Ошибка выполнения запроса get_available_thematic"));
  }

  while (query->next()) {
    const meteo::DbiEntry& doc = query->entry();
    proto::ThematicData them;
    if (false == themFromQuery(doc, &them)) {
      continue;
    }
    resp->add_themes()->CopyFrom(them);
  }

  resp->set_result(true);
//  var(resp->Utf8DebugString());
}


void MethodBag::GetGeotiffInfo( QPair< meteo::rpc::ClientHandler*, const mappi::proto::ThematicData*> param,
                                mappi::proto::GeotifHeader* resp )
{
  auto return_func = [resp](QString err)
  {
    error_log << err;
    resp->set_result(false);
    resp->set_comment(err.toStdString());
  };

  const mappi::proto::ThematicData* req = param.second;

  if (! req->has_path()) { //TODO надо ли возможность запроса имени файла по параметрам?
    return return_func(tr("Не указано имя файла"));
  }

  GDALAllRegister();
  CPLPushErrorHandler(CPLQuietErrorHandler);


  GDALDataset* gds = static_cast<GDALDataset*>(GDALOpen(req->path().c_str(), GA_ReadOnly));
  if (nullptr == gds) {
    return return_func(tr("Ошибка чтения файла"));
  }

  double transform[6]{};
  CPLErr er = gds->GetGeoTransform(transform);
  if(0 != er) debug_log << "geo" << "return" << er << CPLGetLastErrorMsg();
  debug_log << transform[0] << transform[1] << transform[3] << transform[5];

  resp->set_xsize(gds->GetRasterXSize());
  resp->set_ysize(gds->GetRasterYSize());
  resp->set_bands(gds->GetRasterCount());
  resp->set_up_left_x(transform[0]);
  resp->set_x_step(transform[1]);
  resp->set_up_left_y(transform[3]);
  resp->set_y_step(transform[5]);


  GDALClose(gds);
  GDALDestroyDriverManager();

  resp->set_result(true);
}


//new
bool MethodBag::themFromQuery(const meteo::DbiEntry &doc, proto::ThematicData* them)
{
  them->set_session_id(doc.valueInt64("session_id"));
  them->set_type(static_cast<conf::ThemType>(doc.valueInt32("type")));
  them->set_name(doc.valueString("name").toStdString());
  them->set_date_start(doc.valueDt("date_start").toString( Qt::ISODate ).toStdString());
  them->set_date_end(doc.valueDt("date_end" ).toString( Qt::ISODate ).toStdString());
  them->set_path(doc.valueString("fpath").toStdString());
  them->set_satellite_name(doc.valueString("satellite").toStdString());
  them->set_instrument_type(static_cast<conf::InstrumentType>(doc.valueInt32("instrument")));
  them->set_instrument_name(doc.valueString("instr_name").toStdString());
  them->set_projection(doc.valueString("projection").toStdString());
  them->set_format(doc.valueString("format").toStdString());

  // them->set_channel_number(doc.valueInt32("channel"));
  // them->set_channel_alias(doc.valueString("alias").toStdString());
  return true;
}

//new
void MethodBag::SaveThematic( QPair< meteo::rpc::ClientHandler*, const mappi::proto::ThematicData*> param,
                              mappi::proto::Response* resp )
{
  auto return_func = [resp](QString err)
  {
    error_log << err;
    resp->set_result(false);
    resp->set_comment(err.toStdString());
    // return false;
  };

  const mappi::proto::ThematicData* req = param.second;

  var(req->Utf8DebugString());

  std::unique_ptr<meteo::Dbi> db(meteo::global::dbMappi());
  if ( nullptr == db.get() ) {
    return return_func(tr("Ошибка подключения к базе данных mappidb"));
  }

  auto query = db->queryptrByName("thematic_insert");
  if (nullptr == query) {
    return return_func(tr("Ошибка загрузки запроса '%1'").arg("thematic_insert"));
  }

  if (req->has_session_id()) {
    query->arg("session_id", req->session_id());
  }
  if (req->has_satellite_name()) {
    query->arg("satellite", req->satellite_name());
  }
  query->arg("type", req->type());
  query->arg("them_name", req->name());

  query->argDt("date_start", req->date_start());
  query->argDt("date_end",   req->date_end());
  query->arg("path", req->path());
  query->arg("instrument", req->instrument_type());
  query->arg("projection", req->projection());
  query->arg("format", req->format());

  // if (req->has_channel_number()) {
  //   query->arg("channel_number", req->channel_number());
  // }


  QString error;
  if ( !query->execInit(&error) ) {
    return return_func(tr("Ошибка выполнения запроса thematic_insert"));
  }

  resp->set_result(true);
}

void MethodBag::GetAvailableThematicTypes( QPair< meteo::rpc::ClientHandler*, const Dummy*> ,
                                           mappi::proto::ThematicList* resp )
{
  auto return_func = [resp](QString err)
  {
    error_log << err;
    resp->set_result(false);
    resp->set_comment(err.toStdString());
    // return false;
  };

  std::unique_ptr<meteo::Dbi> db(meteo::global::dbMappi());
  if ( nullptr == db.get() ) {
    return return_func(tr("Ошибка подключения к базе данных mappidb"));
  }

  auto query = db->queryptrByName("get_available_thematic_types");
  if (nullptr == query) {
    return return_func(tr("Ошибка загрузки запроса '%1'").arg("get_available_thematic_types"));
  }

  QString error;
  if ( !query->execInit(&error) ) {
    return return_func(tr("Ошибка выполнения запроса get_available_thematic_types"));
  }

  while (query->next()) {
    const meteo::DbiEntry& doc = query->entry();
    proto::ThematicData them;
    if (false == themFromQuery(doc, &them)) {
      continue;
    }
    resp->add_themes()->CopyFrom(them);
  }

  resp->set_result(true);
}

void MethodBag::GetGrayscaleChannels( QPair< meteo::rpc::ClientHandler*,  const Dummy*> ,
                                      mappi::proto::ThematicList* resp )
{
  auto return_func = [resp](QString err)
  {
    error_log << err;
    resp->set_result(false);
    resp->set_comment(err.toStdString());
    // return false;
  };

  std::unique_ptr<meteo::Dbi> db(meteo::global::dbMappi());
  if ( nullptr == db.get() ) {
    return return_func(tr("Ошибка подключения к базе данных mappidb"));
  }

  auto query = db->queryptrByName("get_grayscale_channels");
  if (nullptr == query) {
    return return_func(tr("Ошибка загрузки запроса '%1'").arg("get_grayscale_channels"));
  }

  QString error;
  if ( !query->execInit(&error) ) {
    return return_func(tr("Ошибка выполнения запроса get_grayscale_channels"));
  }

  while (query->next()) {
    const meteo::DbiEntry& doc = query->entry();
    proto::ThematicData them;
    if (false == themFromQuery(doc, &them)) {
      continue;
    }
    resp->add_themes()->CopyFrom(them);
  }


  resp->set_result(true);
}



}
