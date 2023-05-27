#include "tforecastservice.h"
#include "punktdb.h"
#include "methodbag.h"
#include "tgradaciidb.h"
#include <QElapsedTimer>

#include <commons/obanal/func_obanal.h>
#include <commons/meteo_data/tmeteodescr.h>

#include <meteo/commons/global/dbnames.h>

#include <meteo/commons/global/log.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/zond/zond.h>
#include <meteo/commons/services/sprinf/sprinfservice.h>
#include <meteo/commons/services/fieldata/fieldservice.h>

#include <sql/nosql/nosqlquery.h>


namespace meteo{
  namespace forecast{


TForecastService::TForecastService():
ctrl_field_(0),
fdb_(0),
methodbag_( new MethodBag(this) )
{
  fdb_ = new TForecastDb();
}

//static const QString databaseName = QString::fromStdString("meteodb");
static const QString collectionAccuracy = QString::fromStdString("forecast_accuracy");

TForecastService::~TForecastService()
{
  delete methodbag_; methodbag_ = nullptr;
  if(0 != fdb_) {delete fdb_;fdb_=0;}
  if(0 != ctrl_field_) {delete ctrl_field_;ctrl_field_=0;}
}


bool TForecastService::calcAccuracyForecast(const meteo::field::OnePointData* req){
  if(0 == fdb_) return false;
  return fdb_->savecAccuracyForecast(req);
}

void TForecastService::checkFizMethodsBag()
{
  if(nullptr == fizmethodbag_){
    fizmethodbag_ = new FizMethodBag(this);
  }
}


void TForecastService::CalcAccuracyForecast( google::protobuf::RpcController* c,
                   const meteo::field::OnePointData* req,
                   meteo::field::SimpleDataReply* res,
                   google::protobuf::Closure* d ){
  Q_UNUSED(c);
  if(fdb_->savecAccuracyForecast(req)){
      res->set_error("All ok");
      d->Run();
      return;
  }
  res->set_error((msglog::kServiceRequestFailedErr.toUtf8().constData()));
  d->Run();
}


/**
 * получаем точность прогнозов по пунктам
 * @param controller [description]
 * @param request    [description]
 * @param response   [description]
 * @param done       [description]
 */
void TForecastService::GetAccuracy(google::protobuf::RpcController* c,
                                             const AccuracyRequest* req,
                                                     AccuracyReply* resp,
                                         google::protobuf::Closure* done)
{
  QString error;
  rpc::ClientHandler* handler = rpc::Channel::handlerFromController(c);
  if ( nullptr == handler ) {
    error = QObject::tr("Не удалось получить обработчик из контроллера");
    error_log << error;
    // resp->set_comment( error.toStdString() );
    resp->set_result(false);
    done->Run();
    return;
  }
  handler->runMethod( c, methodbag_, &MethodBag::GetAccuracy, qMakePair( handler, req ), resp );
}

/**
 * запрашиваем данные оправдываемости по всем методам по одной станции
 * @param c    [description]
 * @param req  [description]
 * @param resp [description]
 * @param done [description]
 */
void TForecastService::GetManyAccuracy(google::protobuf::RpcController* c,
                                             const ManyAccuracyRequest* req,
                                                     ManyAccuracyReply* resp,
                                         google::protobuf::Closure* done)
{
  QString error;
  rpc::ClientHandler* handler = rpc::Channel::handlerFromController(c);
  if ( nullptr == handler ) {
    error = QObject::tr("Не удалось получить обработчик из контроллера");
    error_log << error;
    resp->set_result(false);
    done->Run();
    return;
  }
  handler->runMethod( c, methodbag_, &MethodBag::GetManyAccuracy, qMakePair( handler, req ), resp );
}


/**
 * Считаем оправдавшиеся прогнозы по станции
 * на вход координаты станции или индекс
 * на выходе - запись в базе forecast_opr с количеством оправдавшихся и неоправд
 * расчетных методов
 *
 * @param controller [description]
 * @param req        [description]
 * @param response   [description]
 * @param done       [description]
 */
void TForecastService::CalcForecastOprStation(google::protobuf::RpcController* c,
                   const meteo::forecast::AccuracyRequest* req,
                   meteo::field::SimpleDataReply* response,
                   google::protobuf::Closure* done){
  QString error;
  rpc::ClientHandler* handler = rpc::Channel::handlerFromController(c);
  if ( nullptr == handler ) {
    error = QObject::tr("Не удалось получить обработчик из контроллера");
    error_log << error;
    response->set_result(false);
    done->Run();
    return;
  }
  handler->runMethod( c, methodbag_, &MethodBag::CalcForecastOprStation, qMakePair( handler, req ), response );
}




void TForecastService::SaveManyForecasts(google::protobuf::RpcController *c,
                                         const ForecastManyPointsData *req,
                                         meteo::field::SimpleDataReply *res,
                                         google::protobuf::Closure *d) {
  Q_UNUSED(c);
  QElapsedTimer ttt; ttt.start();

  if (fdb_->saveManyForecasts(req)){
    res->set_error("All ok");;
    d->Run();
    debug_log << "Save many forecast finished without errors in:" << ttt.elapsed();
    return;
  }

  res->set_error((msglog::kServiceRequestFailedErr.toUtf8().constData()));
  d->Run();
  warning_log << "Save many forecast finished in: " << ttt.elapsed();
}


void TForecastService::SaveForecast( google::protobuf::RpcController* c,
                                     const meteo::forecast::ForecastPointData* req,
                   meteo::field::SimpleDataReply* res,
                   google::protobuf::Closure* d ){
  Q_UNUSED(c);
  QElapsedTimer ttt; ttt.start();

  if(fdb_->saveForecast(req)){
      res->set_error("All ok");
      d->Run();
      debug_log << "Save forecast finished in:" << ttt.elapsed();
      return;
  }

  res->set_error((msglog::kServiceRequestFailedErr.toUtf8().constData()));
  d->Run();
  warning_log << "Save forecast finished in: " << ttt.elapsed()<<"ms";
}


bool TForecastService::prognFromObanal(){

  if(0 == fdb_) return false;
  if(0 == ctrl_field_ ) {
    ctrl_field_ = meteo::global::serviceChannel(meteo::settings::proto::kField);
    if(0 == ctrl_field_ ) {
      error_log << msglog::kNoConnect.arg(meteo::settings::proto::kField);
      return false;
    }
  }

  ::meteo::field::DataRequest request;
  PunktResponce punkts;
  if(!PunktDB::getForecastPunkts(&punkts)|| 1 > punkts.punkts_size()){
    return false;
  }
  foreach (::meteo::forecast::PunktValue punkt, punkts.punkts()) {
      if(false == punkt.isactive()) continue;
      meteo::surf::Point* pnt = request.add_coords();
      pnt->set_index(punkt.stationid());
      pnt->set_name(punkt.name());
      pnt->set_fi(punkt.fi());
      pnt->set_la(punkt.la());
      pnt->set_height(punkt.height());
  }

  QDateTime dt_s = QDateTime::currentDateTimeUtc();
  QDateTime dt_e = dt_s.addDays(10);

  request.set_date_start(dt_s.toString(Qt::ISODate).toStdString());
  request.set_date_end(dt_e.toString(Qt::ISODate).toStdString());
  request.set_need_field_descr(true);

  field::ValueDataReply* reply = ctrl_field_->remoteCall(&meteo::field::FieldService::GetForecastValues, request, 120000);

  if ( 0 == reply ) {
    error_log << msglog::kServiceAnswerFailed.arg(meteo::settings::proto::kField);
    return false;
  }

  for(int i =0; i< reply->data_size(); ++i){
    meteo::forecast::ForecastPointData req;
    req.mutable_coord()->CopyFrom(request.coords(i));
    req.mutable_fdesc()->CopyFrom(reply->data(i).fdesc());
    if(reply->data(i).has_value()){
     continue;
    }
    req.set_value(reply->data(i).value());

    fdb_->saveForecast(&req);
  }
  return true;
}

void TForecastService::GetForecastResult(google::protobuf::RpcController* c,
                       const ForecastResultRequest* req,
                       ForecastResultReply* res,
                       google::protobuf::Closure* d){
  Q_UNUSED(c);

  if(fdb_->getForecastResult(req,res)){
    d->Run();
    return;
  }

  d->Run();

}

void TForecastService::GetMeteoData(google::protobuf::RpcController* controller,
                                    const surf::DataRequest* request,
                                    surf::DataReply* response,
                                    google::protobuf::Closure* done)
{
  Q_UNUSED(controller);
  Q_UNUSED (response);
  Q_UNUSED( request);
  Q_UNUSED(done);
  not_impl;
  done->Run();
  return;
//  auto client = NoSqlManager::instance().getClient();
//  if(nullptr == client) {
//    error_log << client->getLastError();
//    return;
//  }
//  QStringList filters;

//  if ( 0 != request->type_size() ) {
//    filters << util::matchString("type", request->type());
//  }
//  if ( request->has_level_p() ) {
//    filters << util::field("level_p", request->has_level_p());
//  }
//  if ( request->has_type_level() ) {
//    filters << util::field("type_level", request->type_level());
//  }
//  if (request->station_size() != 0){
//    filters << util::matchString("station",  request->station());
//  }
//  if (request->meteo_descr_size() != 0){
//    filters << util::matchString("meteo_descr", request->meteo_descr());
//  }
//  auto cursor = client->find(databaseName, Find(collectionAccuracy, filters));

//  QHash<QString,TMeteoData> surfData;
//  QHash<QString,zond::Zond> aeroData;

//  while (cursor->next()) {
//    QString document = cursor->getDocument();
//    QJsonObject obj = QJsonDocument::fromJson(document.toUtf8()).object();
//    QString station = obj["station"].toString();
//    double value = obj["value"].toDouble();
//    descr_t descr = (descr_t) obj["descr"].toDouble();
//    int lvl = obj["level"].toInt();
//    int lvlType = obj["level_type"].toInt();
//    if ( 0 == lvl && meteodescr::kSurfaceLevel == lvlType ) {
//      surfData[station].set(descr, "", value, control::NO_CONTROL);
//    }
//    else {
//      aeroData[station].setValue(lvlType, lvl, descr, value, control::NO_CONTROL);
//    }
//  }


//  QByteArray ba;

//  QDateTime dt = QDateTime::fromString(QString::fromStdString(request->date_start()), Qt::ISODate);
//  dt.setTimeSpec(Qt::UTC);

//  // приземные данные
//  bool isRequestArrContainsSynop = false;
//  bool isRequestArrContainsAero = false;
//  for (int dataType : request->type()){
//    if (dataType == surf::kSynopFix) isRequestArrContainsSynop = true;
//    if (dataType == surf::kAeroFix) isRequestArrContainsAero = true;
//  }
//  if ( 0 == request->type_size() || true == isRequestArrContainsSynop)    {
//    auto cur = surfData.begin();
//    auto end = surfData.end();
//    for ( ; cur!=end; ++cur ) {
//      cur.value().setDateTime(dt);
//      cur.value().set("category", "", surf::kSynopFix, control::RIGHT);
//      cur.value().setStation(cur.key().toInt());

//      cur.value() >> ba;
//      response->add_meteodata(ba.data(), ba.size());
//    }
//  }

//  // аэрологические данные
//  if ( 0 == request->type_size() || true == isRequestArrContainsAero)
//  {
//    auto cur = aeroData.begin();
//    auto end = aeroData.end();
//    for ( ; cur!=end; ++cur ) {
//      TMeteoData md = cur.value().toMeteoData();

//      md.setDateTime(dt);
//      md.set("category", "", surf::kAeroFix, control::RIGHT);
//      md.setStation(cur.key().toInt());

//      md >> ba;
//      response->add_meteodata(ba.data(), ba.size());
//    }
//  }
//  done->Run();
}

void TForecastService::GetForecastPunkts(google::protobuf::RpcController *controller,
                   const PunktRequest *request,
                   PunktResponce *response,
                   google::protobuf::Closure *done){
  Q_UNUSED(controller);
  bool result = false;

  switch (request->requestcode()) {
  case meteo::forecast::kPunktRemoveRequest:{
      result = PunktDB::removeForecastPunkts(request);
      break;
    }
  case meteo::forecast::kPunktAddRequest:{
      result = PunktDB::addForecastPunkts(request);
      break;
    }
  case meteo::forecast::kPunktEnabledChangeRequest:{
      result = PunktDB::changeActiveForecastPunkts(request);
      break;
    }
  case meteo::forecast::kPunktGetRequest:
  default:
    result = PunktDB::getForecastPunkts(response);
    break;

}

  response->set_result(result);
  done->Run();
}


void TForecastService::FillGradacii(google::protobuf::RpcController *controller,
                                    const Gradacii *request,
                                    Gradacii *response,
                                    google::protobuf::Closure *done){
  Q_UNUSED(controller);
  if (! request->has_level() ||
      ! request->has_type_level() ||
      (!request->has_descr() && !request->has_descrname()) ){
    error_log << QObject::tr("Отсуствует часть обязательных полей");
    done->Run();
    return;
  }

  int descr     = 0;

  // если задано поле descrname, то заюираем его
  if ( request->has_descrname() ){
    descr =  TMeteoDescriptor::instance()->descriptor( request->descrname() );
  }else{
    descr = request->descr();
  }

  int level     = request->level();
  int typeLevel = request->type_level();  
  int hour      = 24;
  if(request->has_fhour()){
    hour = request->fhour();
  }
  

  

  TGradaciiDB::getGradacii(level, typeLevel, descr,hour, response);
  done->Run();
}


void TForecastService::SaveForecastOpr(google::protobuf::RpcController *controller,
                                       const ForecastRequest *request,
                                       ForecastResultReply *response,
                                       google::protobuf::Closure *done){
  Q_UNUSED(controller);
  QStringList oprList;
  QStringList notOprList;
  for(int i = 0; i < request->data_size() ; ++i) {
    auto forecast = request->data(i);
    if (!forecast.has_id() || !forecast.has_opr()){
      response->set_result("Отсутствует часть обязательных полей");
      error_log << response->result();
      done->Run();
      return;
    }
    QString id = QString::fromStdString(forecast.id());
    bool opr = forecast.opr();
    if (opr)  {
      oprList.append(id);
    } else {
      notOprList.append(id);
    }
  }
  TForecastDb::setOpr(oprList, true);
  TForecastDb::setOpr(notOprList, false);
  done->Run();
}


void TForecastService::SaveForecastError(google::protobuf::RpcController *controller,
                                         const ForecastRequest *request,
                                         ForecastResultReply *response,
                                         google::protobuf::Closure *done){
  Q_UNUSED(controller);
  QStringList ids;
  for(int i = 0; i < request->data_size() ; ++i) {
    auto forecast = request->data(i);
    if (!forecast.has_id()){
      response->set_result("Отсутствует часть обязательных полей");
      error_log << response->result();
      done->Run();
      return;
    }
    QString id = QString::fromStdString(forecast.id());
    ids << id;
  }
  TForecastDb::setError(ids);
  done->Run();
}


void TForecastService::CalAccuracyForecastMany( google::protobuf::RpcController* c,
                                                const MsgCalAccuracyForecastMany* req,
                                                meteo::field::SimpleDataReply* res,
                                                google::protobuf::Closure* d ){
  Q_UNUSED(c);
  QElapsedTimer ttt; ttt.start();
  bool allOk = true;
  for (int i = 0; i < req->data_size(); ++i){
    allOk &= fdb_->savecAccuracyForecast(&req->data(i));
  }
  debug_log << QObject::tr("CalAccuracyForecastMany finished in %1 msec").arg(ttt.elapsed());
  if (allOk){
      res->set_error("All ok");
      d->Run();
  } else {
    res->set_error((msglog::kServiceRequestFailedErr.toUtf8().constData()));
    d->Run();
  }
}


/**
 * @brief Метод сохранения введенных вручную прогнозов погоды по региону или
 * по станции 
 * 
 * @param c 
 * @param request 
 * @param response 
 * @param done 
 */
void TForecastService::SaveManualForecast( google::protobuf::RpcController* c,
                                            const ForecastManualData* request,
                                            ForecastManualReply* response,
                                            google::protobuf::Closure* done ){
  QString error;
  rpc::ClientHandler* handler = rpc::Channel::handlerFromController(c);
  if ( nullptr == handler ) {
    error = QObject::tr("Не удалось получить обработчик из контроллера");
    error_log << error;
    // resp->set_comment( error.toStdString() );
    response->set_result(false);
    done->Run();
    return;
  }
  handler->runMethod( c, methodbag_, &MethodBag::SaveManualForecast, qMakePair( handler, request ), response );
}

/**
 * @brief Метод получения прогнозов введенных вручную прогнозов погоды по региону или
 * по станции 
 * 
 * @param c 
 * @param request 
 * @param response 
 * @param done 
 */
void TForecastService::GetManualForecast( google::protobuf::RpcController* c,
                                          const ForecastManualRequest* request,
                                          ForecastManualReply* response,
                                          google::protobuf::Closure* done ){

  QString error;
  rpc::ClientHandler* handler = rpc::Channel::handlerFromController(c);
  if ( nullptr == handler ) {
    error = QObject::tr("Не удалось получить обработчик из контроллера");
    error_log << error;
    // resp->set_comment( error.toStdString() );
    response->set_result(false);
    done->Run();
    return;
  }
  handler->runMethod( c, methodbag_, &MethodBag::GetManualForecastResult, qMakePair( handler, request ), response );
}



void TForecastService::getForecastAV12( google::protobuf::RpcController* c,
                                        const ForecastAv12Request* req,
                                        ForecastAv12Responce* res,
                                        google::protobuf::Closure* d )
{
  Q_UNUSED(c);
  QElapsedTimer ttt; ttt.start();
  fdb_->getForecastAV12(req, res);
  debug_log << QString::fromStdString("Запрос getForecastAV12 выполнен за %1 мсек")
              .arg(ttt.elapsed());
  d->Run();
}


void TForecastService::GetForecastMethodsList( google::protobuf::RpcController* c,
                   const Dummy* request,  ForecastMethodList* response,  google::protobuf::Closure* done){

  QString error;
  rpc::ClientHandler* handler = rpc::Channel::handlerFromController(c);
  if ( nullptr == handler ) {
    error = QObject::tr("Не удалось получить обработчик из контроллера");
    error_log << error;
    // resp->set_comment( error.toStdString() );
    response->set_result(false);
    done->Run();
    return;
  }
  checkFizMethodsBag();
  handler->runMethod( c, fizmethodbag_, &FizMethodBag::GetForecastMethodsList, request, response );
}

void TForecastService::GetForecastMethodsResultRun(google::protobuf::RpcController* c,
                                                   const ForecastResultRequest* request,
                                                   ForecastFizResult* response,  google::protobuf::Closure* done)
{
  QString error;
  rpc::ClientHandler* handler = rpc::Channel::handlerFromController(c);
  if ( nullptr == handler ) {
    error = QObject::tr("Не удалось получить обработчик из контроллера");
    error_log << error;
    // resp->set_comment( error.toStdString() );
    response->set_result(false);
    done->Run();
    return;
  }
  checkFizMethodsBag();
  handler->runMethod( c, fizmethodbag_, &FizMethodBag::GetForecastMethodsResultRun, request, response );

}



} // forecast
} // meteo
