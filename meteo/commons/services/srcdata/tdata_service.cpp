#include "tdata_service.h"

#include <qmap.h>
#include <qprocess.h>
#include <qcoreapplication.h>
#include <qthreadpool.h>
#include <qelapsedtimer.h>
#include <qjsondocument.h>
#include <qjsonarray.h>
#include <qjsonobject.h>
#include <qjsonvalue.h>

#include <cross-commons/debug/tlog.h>

//#include <sql/nspgbase/tsqlquery.h>
#include <sql/dbi/dbiquery.h>
#include <sql/dbi/dbi.h>


#include <sql/nosql/nosqlquery.h>

#include <commons/meteo_data/meteo_data.h>
#include <commons/meteo_data/tmeteodescr.h>
#include <commons/textproto/pbcache.h>
#include <commons/textproto/pbtools.h>

#include <meteo/commons/global/global.h>

#include "meteodata_service.h"

namespace meteo {
namespace surf {

static const int kLoaderQueueLimit = 1000;

TDataServiceObj::TDataServiceObj(): mutex_(QMutex::Recursive) { }

TDataServiceObj::~TDataServiceObj() { }

void TDataServiceObj::appendThread(TBaseDataService *ts, QThread *th)
{
  mutex_.lock();
  pool.insert(ts,th);
  mutex_.unlock();
}

void TDataServiceObj::finished(TBaseDataService *obj)
{
  auto castedObj = qobject_cast<TGribDataService*>(obj);
  if ( nullptr != castedObj ) {
    if ( nullptr != castedObj->getGribDataReply() ) {
      //  debug_log << castedObj->getGribDataReply()->info().Utf8DebugString();
    }
  }


  mutex_.lock();
  google::protobuf::Closure* d = obj->GetClosure();
  if ( nullptr != d ) {
    d->Run();
  }
  QThread *th = pool.value(obj);
  th->quit();
  delete obj;
  mutex_.unlock();
}

void TDataService::moveToThread(TBaseDataService *obj)
{
  // Создание потока
  QThread* thread = new QThread;
  // Передаем права владения "рабочим" классом, классу QThread.
  obj->moveToThread(thread);
  dobj.appendThread(obj,thread);
  QObject::connect(thread, &QThread::started, obj, &TBaseDataService::run);
  QObject::connect(obj, &TBaseDataService::finished, &dobj, &TDataServiceObj::finished);
  QObject::connect(thread, &QThread::finished, thread, &QThread::deleteLater);

  thread->start();
}

TDataService::TDataService(::meteo::AppStatusThread* status,int serviceType, QObject* parent)
  : QObject(parent),
    status_(status)
{
  sendingDataMulti_ = false;
  sendingCurState_  = false;
  if(nullptr != status_){
    status_->setTitle(kRequestsCount, tr("Получено запросов"));
    status_->setTitle(kSuccessResponseCount, tr("Отправлено ответов"));
    status_->setTitle(kErorrResponseCount, tr("Ошибок при обработке запросов"));
    status_->setParam(kRequestsCount, 0,app::OperationState_NONE);
    status_->setParam(kSuccessResponseCount, 0,app::OperationState_NONE);
    status_->setParam(kErorrResponseCount, 0,app::OperationState_NONE);
    QObject::connect( this, &meteo::surf::TDataService::signalRequestReceive,
                      this,&meteo::surf::TDataService::slotRequestReceive );
  }

  if(settings::proto::kSrcData == serviceType || settings::proto::kSrcDataLocal == serviceType) {
    serviceType_ = serviceType;
  }
  else {
    error_log << QObject::tr("Указан не верный тип сервиса.");
    serviceType_ = settings::proto::kSrcData;
  }

  // Инициализация строки подключения к mongodb
  //auto conf = meteo::Global::instance()->mongodbConfMeteo();
  //dbname_ = conf.name();

  QTimer* timer = new QTimer(this);
  // connect(timer, &QTimer::timeout, this, &TDataService::slotRunCurState);
  connect(timer, &QTimer::timeout, this, &TDataService::slotRunDataMulti);
  timer->setInterval(1000);
  timer->start();

}

void TDataService::slotRequestReceive(){
  if(nullptr != status_){
    int count_req = status_->getParamValue(kRequestsCount).toInt();
    status_->setParam(kRequestsCount, count_req+1,app::OperationState_NORM);
  }
}

void TDataService::slotAnsverSendedError(){
  if(nullptr != status_){
    int count_err = status_->getParamValue(kErorrResponseCount).toInt();
    status_->setParam(kErorrResponseCount, count_err+1,app::OperationState_ERROR);
  }
}

void TDataService::slotAnsverSendedOK(){
  if(nullptr != status_){
    int count_ok = status_->getParamValue(kSuccessResponseCount).toInt();
    status_->setParam(kSuccessResponseCount,count_ok+1,app::OperationState_NORM);
  }
}

//!< Запрос на получение полноту данных (отношение количества станций с пришедшими данными к работающим станциям)
void TDataService::GetDataCover(RpcController* c, const DataRequest* req,
                                DataReply* resp, google::protobuf::Closure* d)
{
  Q_UNUSED(c);
  emit signalRequestReceive();
  MeteoDataService *mds = new MeteoDataService(req, resp, d, rGetDataCover, serviceType_);
  moveToThread(mds);
}


void TDataService::SaveMsg(google::protobuf::RpcController* controller,
                           const tlg::MessageNew* request,
                           SaveMsgReply* response,
                           google::protobuf::Closure* done)
{
  emit signalRequestReceive();
  Q_UNUSED( controller );
  if ( nullptr == msgQueue_ ) {
    response->set_error(tr("Сохранение сообщений не реализовано.").toStdString());
    done->Run();
    return;
  }

  MsgQueueItem item = msgQueue_->append(*request, MsgStage::kNone);
  if ( !item.isValid() ) {
    response->set_error(tr("Очередь переполнена.").toStdString());
    done->Run();
    return;
  }

  saveMsgCards_[item.id()] = SaveMsgCard(controller, request, response, done);

  item.appendStage(MsgStage::kReceive);
  item.appendStage(MsgStage::kRoute);
  item.nextStage();
}

void TDataService::SetDecode(google::protobuf::RpcController* controller,
                             const msgcenter::DecodeRequest* request,
                             msgcenter::Dummy* response,
                             google::protobuf::Closure* done)
{
  emit signalRequestReceive();
  Q_UNUSED( controller );
  Q_UNUSED( response );

  if ( nullptr == decodeQueue_ ) { return; }

  decodeQueue_->append(*request);

  // отправляем ответ не дожидаясь обновления документов в БД
  done->Run();
}



void TDataService::GetDataCount(RpcController* c, const DataRequest*  req, CountDataReply* resp, google::protobuf::Closure* d)
{
  emit signalRequestReceive();
  Q_UNUSED(c);
  MeteoDataService *mds = new MeteoDataService(req, resp, d, rGetDataCount, serviceType_);
  moveToThread(mds);
}


void TDataService::GetAvailableSigwx(RpcController* c, const DataRequest* req, SigwxDescResponse* res, Closure* d)
{
  emit signalRequestReceive();
  Q_UNUSED(c);
  MeteoDataService *mds = new MeteoDataService(req, res, d, serviceType_);
  moveToThread(mds);
}

void TDataService::GetSigwx(RpcController* c, const SigwxDesc* req, meteo::sigwx::SigWx* res, Closure* d)
{
  emit signalRequestReceive();
  Q_UNUSED(c);
  MeteoDataService* zds = new MeteoDataService(req, res, d, serviceType_);
  moveToThread(zds);
}

void TDataService::UpdateMeteoData(RpcController* c, const UpdateDataRequest* req, Value* res, Closure* d)
{
  emit signalRequestReceive();
  Q_UNUSED(c);
  Q_UNUSED(req);
  Q_UNUSED(res);
  Q_UNUSED(d);
  res->set_result(false);
  res->set_comment( QObject::tr("Вызов UpdateMeteoData не реализован").toStdString() );
  d->Run();
}

void TDataService::GetGmiSenderStations(RpcController* c, const DataRequest* req, StationsDataReply* res, Closure* d)
{
  emit signalRequestReceive();
  Q_UNUSED(c);
  Q_UNUSED(req);
  Q_UNUSED(res);
  Q_UNUSED(d);
  res->set_comment( QObject::tr("Вызов GetGmiSenderStations не реализован").toStdString() );
  res->set_result(true);
  d->Run();
}

//! Станции с данными по дате и типу данных
void TDataService::GetAvailableStations(RpcController* c, const DataRequest* req, StationsDataReply* res, Closure* d)
{
  emit signalRequestReceive();
  Q_UNUSED(c);
  MeteoDataService* mds = new MeteoDataService(req, res, d, serviceType_);
  moveToThread(mds);
}

void TDataService::RemoveGribDataAnalysed(RpcController* c, const SimpleDataRequest* req, SimpleDataReply* res, Closure* d)
{
  emit signalRequestReceive();
  Q_UNUSED(c);
  TGribDataService *gds = new TGribDataService(d);
  gds->setReplyType(rRemoveGribDataAnalysed);
  gds->setSimpleDataRequest(req);
  gds->setSimpleDataReply(res);
  moveToThread(gds);
}

void TDataService::GetNotAnalysedGribData(RpcController* c, const EmptyDataRequest* req, GribDataReply* res, Closure* d)
{
  emit signalRequestReceive();
  Q_UNUSED(req);
  Q_UNUSED(c);
  TGribDataService *gds = new TGribDataService(d);
  gds->setReplyType(rGetNotAnalysedGribData);
  gds->setGribDataReply(res);
  moveToThread(gds);
}

void TDataService::GetGribData(RpcController *c, const GribDataRequest *req, GribDataReply *res, Closure *d)
{
  emit signalRequestReceive();
  Q_UNUSED(c);
  TGribDataService *gds = new TGribDataService(d);
  gds->setReplyType(rGetGribData);
  gds->setGribDataRequest(req);
  gds->setGribDataReply(res);
  moveToThread(gds);
}

//! Доступные грибы по дате и типу уровня
void TDataService::GetAvailableGrib(RpcController* c, const DataRequest* req, GribDataAvailableReply* res, Closure* d)
{
  emit signalRequestReceive();
  Q_UNUSED(c);
  TGribDataService *gds = new TGribDataService(d);
  gds->setReplyType(rGetGribAvailable);
  gds->setDataRequest(req);
  gds->setDataReply(res);
  moveToThread(gds);
}

void TDataService::GetMrlDataOnAllStation(RpcController* c, const DataRequest* req, ManyMrlValueReply* res, Closure* d)
{
  emit signalRequestReceive();
  QString error;
  rpc::ClientHandler* handler = rpc::Channel::handlerFromController(c);
  if ( nullptr == handler ) {
    error = QObject::tr("Не удалось получить обработчик из контроллера");
    error_log << error;
    res->set_comment( error.toStdString() );
    res->set_result(false);
    d->Run();
    return;
  }
  handler->runMethod( c, &rds, &TRadarDataService::getAllMrlData, req, res );
}

void TDataService::GetAvailableZond(RpcController* c, const DataRequest* req, StationsDataReply* res, Closure* d)
{
  emit signalRequestReceive();
  Q_UNUSED(c);

  TZondDataService *zds = new TZondDataService(req, d);
  zds->setReplyType(rGetZondAvailable);
  zds->setZondStationsReply(res);
  moveToThread(zds);
}

void TDataService::GetTZondOnStation(RpcController* c, const DataRequest* req, TZondValueReply* res, Closure* d)
{
  emit signalRequestReceive();
  Q_UNUSED(c);
  QElapsedTimer ttt; ttt.start();
  if ( 0 == req->type_size() ) {
    error_log << QObject::tr("Запрос заполнен не полностью");
    res->set_result(false);
    res->set_comment( QObject::tr("Запрос заполнен не полностью").toStdString() );
    d->Run();
    return;
  }
  int datatype = req->type(0);

  if( datatype >= kAeroType && datatype <= kEndAeroType ) { //req->type() == 2
    TZondDataService *zds = new TZondDataService(req,d);
    // если казан флаг as_proto  - то вызываем функцию для расчета
    // пармаетров АД и возвращаем не в бинарном виде, а в виде массива уровней
    if ( req->has_as_proto() && req->as_proto() ){
      zds->setReplyType(rGetValueWithCalc);
    }else{
      zds->setReplyType(rGetTZondValue);
    }
    // zds->setZondValueReply(res);
    zds->setDataReply(res);
    moveToThread(zds);
  }
  else if (datatype == kSatAero) {
    TZondDataService *zds = new TZondDataService(req, d);
    zds->setReplyType(rGetTZondSatValue);
    zds->setDataReply(res);
    moveToThread(zds);
  }
  else {
    res->set_result(false);
    res->set_comment(QObject::tr("Запрос задан некорректно или ошибка выполнения запроса").toUtf8().constData());
    d->Run();
  }
}

void TDataService::GetZondDates(RpcController* c, const DataRequest* req, DateReply* res, Closure* d)
{
  emit signalRequestReceive();
  Q_UNUSED(c);
  QElapsedTimer ttt; ttt.start();
  if ( 0 == req->type_size() ) {
    error_log << QObject::tr("Запрос заполнен не полностью");
    d->Run();
    return;
  }
  int datatype = req->type(0);
  if( datatype >= kAeroType && datatype <= kEndAeroType ) { //req->type() == 2
    TZondDataService *zds = new TZondDataService(req,d);
    zds->setReplyType(rGetDates);
    zds->setDateReply(res);
    moveToThread(zds);
  }
  else {
    d->Run();
  }
}

void TDataService::GetZondDataOnStation(RpcController* c, const DataRequest* req, ZondValueReply* res, Closure* d)
{
  emit signalRequestReceive();
  Q_UNUSED(c);
  if ( 0 == req->type_size() ) {
    error_log << QObject::tr("Запрос заполнен не полностью");
    res->set_result(false);
    res->set_comment( QObject::tr("Запрос заполнен не полностью").toStdString() );
    d->Run();
    return;
  }
  int datatype = req->type(0);
  if ( datatype >= kAeroType && datatype <= kEndAeroType) { //req->type() == 2
    TZondDataService *zds = new TZondDataService(req,d);
    zds->setReplyType(rGetValue);
    zds->setZondValueReply(res);
    moveToThread(zds);
  }
  else {
    res->set_result(false);
    res->set_comment(QObject::tr("Запрос задан некорректно или ошибка выполнения запроса").toUtf8().constData());
    d->Run();
  }
}

void TDataService::GetValueOnStation(RpcController* c, const DataRequest* req, ValueReply* res, Closure* d)
{
  emit signalRequestReceive();
  Q_UNUSED(c);
  if ( 0 == req->type_size() ) {
    error_log << QObject::tr("Запрос заполнен не полностью");
    res->set_result(false);
    res->set_comment( QObject::tr("Запрос заполнен не полностью").toStdString() );
    d->Run();
    return;
  }
  int datatype = req->type(0);
  if ( datatype >= kOceanType && datatype <= kEndOceanType) {
    OceanDataService *ods = new OceanDataService(req, d);
    ods->setValueReply(res);
    ods->setReplyType(rGetManyOceanDataField);
    moveToThread(ods);
  }
  else {
    TSurfaceDataService *sds = new TSurfaceDataService(req,res,d);
    moveToThread(sds);
    return;
  }
}

//Возвращает значение  в виде TMeteoData
////по индексу, времени и дескриптору
void TDataService::GetMeteoDataOnStation(RpcController* c, const DataRequest* req, DataReply* res, Closure* d)
{
  emit signalRequestReceive();
  Q_UNUSED(c);
  // запрос в вышестоящий сервис
  if ( true == req->use_parent() ) {
    if ( loaderQueue_->size() > kLoaderQueueLimit ) {
      res->set_result(false);
      res->set_comment(tr("Очередь запросов переполнена").toStdString());
      debug_log << tr("Очередь запросов переполнена.");
    }
    else {
      QString reqId = QString("GetMeteoDataOnStation.%1.%2").arg(QDateTime::currentDateTime().toMSecsSinceEpoch()*0.001).arg(++requestCounter_);
      loaderQueue_->append(LoaderCard(reqId,kGetMeteoDataOnStation,*req));
      res->set_result(true);
      res->set_request_id(reqId.toStdString());
      debug_log << tr("Запрос на получение данных добавлен в очередь (id: %1)").arg(reqId);
    }
    d->Run();
    return;
  }
  if ( req->has_request_id() ) {
    res->set_request_id(req->request_id());
  }

  MeteoDataService *zds = new MeteoDataService(req, res, d, rRunService, serviceType_);
  moveToThread(zds);
}

void TDataService::GetMeteoData(RpcController* c, const DataRequest* req, DataReply* res, Closure* d)
{
  emit signalRequestReceive();
  Q_UNUSED(c);
  // запрос в вышестоящий сервис
  if ( true == req->use_parent() ) {
    if ( loaderQueue_->size() > kLoaderQueueLimit ) {
      res->set_result(false);
      res->set_comment(tr("Очередь запросов переполнена").toStdString());
      debug_log << tr("Очередь запросов переполнена.");
    }
    else {
      QString reqId = QString("GetMeteoData.%1.%2").arg(QDateTime::currentDateTime().toMSecsSinceEpoch()*0.001).arg(++requestCounter_);
      loaderQueue_->append(LoaderCard(reqId,kGetMeteoData,*req));
      res->set_result(true);
      res->set_request_id(reqId.toStdString());
      debug_log << tr("Запрос на получение данных добавлен в очередь (id: %1)").arg(reqId);
    }
    d->Run();
    return;
  }
  if ( req->has_request_id() ) {
    res->set_request_id(req->request_id());
  }

  MeteoDataService *zds = new MeteoDataService(req, res, d, rGetMeteoData, serviceType_);
  moveToThread(zds);
}

void TDataService::GetMeteoDataFromGrib(RpcController* c, const GribDataRequest* req, DataReply* res, Closure* d)
{
  emit signalRequestReceive();
  Q_UNUSED(c);
  TGribDataService *gds = new TGribDataService(d);
  gds->setReplyType(rGetGribMeteoData);
  gds->setGribDataRequest(req);
  gds->setDataReply(res);
  moveToThread(gds);
}




void TDataService::GetMeteoDataByDay(RpcController* c, const DataRequest* req, DataReply* res, Closure* d)
{
  emit signalRequestReceive();
  Q_UNUSED(c);
  // запрос в вышестоящий сервис
  if ( true == req->use_parent() ) {
    if ( loaderQueue_->size() > kLoaderQueueLimit ) {
      res->set_result(false);
      res->set_comment(tr("Очередь запросов переполнена").toStdString());
      debug_log << tr("Очередь запросов переполнена.");
    }
    else {
      QString reqId = QString("GetMeteoDataByDay.%1.%2").arg(QDateTime::currentDateTime().toMSecsSinceEpoch()*0.001).arg(++requestCounter_);
      loaderQueue_->append(LoaderCard(reqId,kGetMeteoDataByDay,*req));
      res->set_result(true);
      res->set_request_id(reqId.toStdString());
      debug_log << tr("Запрос на получение данных добавлен в очередь (id: %1)").arg(reqId);
    }
    d->Run();
    return;
  }
  if ( req->has_request_id() ) {
    res->set_request_id(req->request_id());
  }

  MeteoDataService *zds = new MeteoDataService(req, res, d, rGetMeteoData, serviceType_);
  moveToThread(zds);
}

void TDataService::GetMeteoDataTree(RpcController* c, const DataRequest* req, DataReply* res, Closure* d )
{
  emit signalRequestReceive();
  Q_UNUSED(c);
  MeteoDataService *zds = new MeteoDataService(req, res, d, rGetMeteoDataTree, serviceType_);
  moveToThread(zds);
}

void TDataService::GetAvailableRadar(RpcController* c, const DataRequest* req, DataDescResponse* res, Closure* d)
{
  emit signalRequestReceive();
  Q_UNUSED(c);
  QElapsedTimer ttt; ttt.start();

  if( true == rds.getAvailableRadar(req,res)) {
    int cur =  ttt.elapsed();
    if(1000 < cur) {
      warning_log << QString::fromUtf8("Время обработки запроса GetAvailableRadar: %1 мсек").arg(cur);
    }
    res->set_result(true);
  }
  else {
    res->set_result(false);
    res->set_comment(QObject::tr("Запрос задан некорректно или ошибка выполнения запроса").toUtf8().constData());
  }
  d->Run();
}

void TDataService::GetOceanDataOnPoint(RpcController* c, const DataRequest* req, DataReply* res, Closure* d)
{
  emit signalRequestReceive();
  Q_UNUSED(c);
  if ( 0 == req->type_size() ) {
    error_log << QObject::tr("Запрос заполнен не полностью");
    res->set_result(false);
    res->set_comment( QObject::tr("Запрос заполнен не полностью").toStdString() );
    d->Run();
    return;
  }

}

void TDataService::GetOceanByDt(RpcController* c, const DataRequest* req, DataReply* res, Closure* d)
{
  emit signalRequestReceive();
  Q_UNUSED(c);
  if ( 0 == req->type_size() ) {
    error_log << QObject::tr("Запрос заполнен не полностью");
    res->set_result(false);
    res->set_comment( QObject::tr("Запрос заполнен не полностью").toStdString() );
    d->Run();
    return;
  }
}

void TDataService::GetAvailableOcean(RpcController* c, const DataRequest* req, OceanAvailableReply* res, Closure* d)
{
  emit signalRequestReceive();
  Q_UNUSED(c);
  if ( 0 == req->type_size() ) {
    error_log << QObject::tr("Запрос заполнен не полностью");
    res->set_result(false);
    res->set_comment( QObject::tr("Запрос заполнен не полностью").toStdString() );
    d->Run();
    return;
  }
  int datatype = req->type(0);
  if( datatype >= kOceanType && datatype <= kEndOceanType) {
    OceanDataService *ods = new OceanDataService(req,d);
    ods->setDataReply(res);
    ods->setReplyType(rGetAvailableOcean);
    moveToThread(ods);
  }
  else {
    res->set_result(false);
    res->set_comment(QObject::tr("Запрос задан некорректно или ошибка выполнения запроса").toUtf8().constData());
    d->Run();
  }
}

void TDataService::GetOceanById(RpcController* c, const SimpleDataRequest* req, DataReply* res, Closure* d)
{
  emit signalRequestReceive();
  Q_UNUSED(c);
  OceanDataService *ods = new OceanDataService(req,d);
  ods->setDataReply(res);
  ods->setReplyType(rGetOceanById);
  moveToThread(ods);
}

void TDataService::GetAvailableSynMobSea(TDataService::RpcController *c, const DataRequest *req, OceanAvailableReply *res, TDataService::Closure *d)
{
  emit signalRequestReceive();
  Q_UNUSED(c);
  OceanDataService* ods = new OceanDataService(req, d);
  ods->setDataReply(res);
  ods->setReplyType(rGetAvailableSynMobSea);
  moveToThread(ods);
}



//! Доступные геофизические карты по дате/времени и типу
void TDataService::GetAvailableGeophysMap(RpcController* c, const DataRequest* req, GeophysMapAvailableReply* res, Closure* d)
{
  emit signalRequestReceive();
  Q_UNUSED(c);
  if ( 0 == req->type_size() ) {
    error_log << QObject::tr("Запрос заполнен не полностью");
    res->set_result(false);
    res->set_comment( QObject::tr("Запрос заполнен не полностью").toStdString() );
    d->Run();
    return;
  }
  int datatype = req->type(0);
  if ( datatype >= kGeophIonex && datatype <= kGeophGtex) {
    GeophysDataService *gds = new GeophysDataService(req, d);
    gds->setDataReply(res);
    gds->setReplyType(rGetAvailableGeophysMap);
    moveToThread(gds);
  }
  else {
    res->set_result(false);
    res->set_comment(QObject::tr("Запрос задан некорректно или ошибка выполнения запроса").toUtf8().constData());
    d->Run();
  }
}

//! Геофизическая карта по id
void TDataService::GetGeophysById(RpcController* c, const SimpleDataRequest* req, DataReply* res, Closure* d)
{
  emit signalRequestReceive();
  Q_UNUSED(c);
  GeophysDataService *gds = new GeophysDataService(req, d);
  gds->setDataReply(res);
  gds->setReplyType(rGetGeophysMapById);
  moveToThread(gds);
}


void TDataService::AddMeteoData(RpcController* c, const DataReply* req, SimpleDataReply* res, Closure* d)
{
  emit signalRequestReceive();
  Q_UNUSED(c);
  Q_UNUSED(req);
  Q_UNUSED(res);
  Q_UNUSED(d);

  res->set_result(false);
  res->set_comment( QObject::tr("Вызов GetAvailableSigwx не реализован").toStdString() );
  d->Run();
}

void TDataService::GetMsgMulti(google::protobuf::RpcController* controller,
                               const msgcenter::DistributeMsgRequest* request,
                               tlg::MessageNew* response,
                               google::protobuf::Closure* done)
{
  emit signalRequestReceive();
  emit subscribeDistributeMsg( ::meteo::DistributeMsgCard(controller,request,response,done) );
}

void TDataService::GetDataMulti(RpcController *c, const DataRequest *req, DataReply *res, Closure *d)
{
  emit signalRequestReceive();
  DataMultiCard card;
  card.request  = req;
  card.response = res;
  card.done = d;
  card.controller = c;

  dataMultiCards_[subs_.key(d)] = card;

  debug_log << QObject::tr("Оформлена подписка на получение метеоданных, всего подписчиков %1").arg(dataMultiCards_.size());
}

void TDataService::GetCurStateMulti(RpcController* c, const CurStateRequest* req, DataReply* res, Closure* d)
{
  emit signalRequestReceive();
  CurStateCard card;
  card.request = req;
  card.response = res;
  card.done = d;
  card.controller = c;

  curStateCards_[subs_.key(d)] = card;

  debug_log << QObject::tr("Оформлена подписка на получение текущих метеоданных, всего подписчиков %1").arg(curStateCards_.size());
}

void TDataService::GetDtsForPeriod(TDataService::RpcController *c, const DataRequest *req, DataReply *res, TDataService::Closure *d)
{
  emit signalRequestReceive();
  Q_UNUSED(c);

  // запрос в вышестоящий сервис
  if ( true == req->use_parent() ) {
    if ( loaderQueue_->size() > kLoaderQueueLimit ) {
      res->set_result(false);
      res->set_comment(tr("Очередь запросов переполнена").toStdString());
      debug_log << tr("Очередь запросов переполнена.");
    }
    else {
      QString reqId = QString("GetDtsForPeriod.%1.%2").arg(QDateTime::currentDateTime().toMSecsSinceEpoch()*0.001).arg(++requestCounter_);
      loaderQueue_->append(LoaderCard(reqId,kGetDts,*req));
      res->set_result(true);
      res->set_request_id(reqId.toStdString());
      debug_log << tr("Запрос на получение данных добавлен в очередь (id: %1)").arg(reqId);
    }
    d->Run();
    return;
  }
  if ( req->has_request_id() ) {
    res->set_request_id(req->request_id());
  }

  MeteoDataService *zds = new MeteoDataService(req, res, d, rGetDts, serviceType_);
  moveToThread(zds);
}

void TDataService::slotClientSubscribed(rpc::Controller* call  )
{
  rpc::Channel* ch = call->channel();
  google::protobuf::Closure* d = call->closure();
  debug_log << tr("Подключился клиент '%1'.").arg(ch->address());
  subs_[ch] = d;
}
//
//void TDataService::slotClientDisconnected(rpc::Channel* ch)
//{
//  if(subs_.contains(ch)) {
//    debug_log << tr("Клиент '%1' отключился.").arg(ch->address());
//    subs_.remove(ch);
//    dataMultiCards_.remove(ch);
//    curStateCards_.remove(ch);
//  }
//}

void TDataService::slotClientUnsubscribed( rpc::Controller* call )
{
  rpc::Channel* ch = call->channel();
  if(subs_.contains(ch)) {
    debug_log << tr("Клиент '%1' отключился.").arg(ch->address());
    subs_.remove(ch);
    dataMultiCards_.remove(ch);
    curStateCards_.remove(ch);
  }
}

void TDataService::slotRunDataMulti()
{
  if(dataMultiCards_.isEmpty()) { return; }
  if(sendingDataMulti_) { return; }
  sendingDataMulti_ = true;

  QElapsedTimer t;
  t.start();

  QDateTime cur = QDateTime::currentDateTimeUtc();
  cur.setTime(QTime(0,0,0));
  std::string curDt = cur.date().toString(Qt::ISODate).toStdString();

  for(DataMultiCard& card : dataMultiCards_.values()) {
    DataRequest req = *card.request;
    req.set_date_start(curDt);
    MeteoDataService srv(&req, card.response, card.done,rGetMeteoData,serviceType_);
    srv.run();
    card.done->Run();
  }

  sendingDataMulti_ = false;

  info_log_if(t.elapsed() > 500) << QObject::tr("Время рассылки метеоданных подписчикам %1 мсек.").arg(t.elapsed());
}

void TDataService::slotMsgProcessed(int id)
{
  if(!saveMsgCards_.contains(id)) { return; }

  SaveMsgCard card = saveMsgCards_.take(id);
  card.done->Run();
}

void TDataService::slotMsgProcessed(int id, const QString& error, qint64 ptkppId, bool duplicate)
{
  if (!saveMsgCards_.contains(id)) { return; }

  SaveMsgCard card = saveMsgCards_.take(id);
  if ( !error.isEmpty() ) {
    card.response->set_error(error.toStdString());
  }
  card.response->set_id(ptkppId);
  card.response->set_duplicate(duplicate);
  card.done->Run();
}

}
}
