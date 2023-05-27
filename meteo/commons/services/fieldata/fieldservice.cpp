#include "fieldservice.h"
#include "methodwrap.h"

#include <qelapsedtimer.h>
#include <qcoreapplication.h>
#include <qdir.h>
#include <qmap.h>
#include <qprocess.h>
#include <qstring.h>
#include <qvector.h>
#include <qmath.h>

#include <cross-commons/debug/tlog.h>

#include <commons/geobasis/geopoint.h>
#include <commons/meteo_data/meteo_data.h>
#include <commons/textproto/pbtools.h>

#include <sql/dbi/gridfs.h>

#include <meteo/commons/global/global.h>
#include <meteo/commons/global/log.h>
#include <meteo/commons/zond/zond.h>
#include <meteo/commons/services/obanal/tobanal.h>

namespace meteo {
namespace field {

TFieldDataService::TFieldDataService(AppStatusThread* status)
  :FieldService(),
    status_(status)
{
  if(nullptr != status_){
    status_->setTitle(kRequestsCount, QObject::tr("Получено запросов"));
    status_->setTitle(kSuccessResponseCount, QObject::tr("Отправлено ответов"));
    status_->setTitle(kErorrResponseCount, QObject::tr("Ошибок при обработке запросов"));
    status_->setParam(kRequestsCount, 0,app::OperationState_NONE);
    status_->setParam(kSuccessResponseCount, 0,app::OperationState_NONE);
    status_->setParam(kErorrResponseCount, 0,app::OperationState_NONE);
    QObject::connect( this, &meteo::field::TFieldDataService::signalRequestReceive,
                      this,&meteo::field::TFieldDataService::slotRequestReceive );
  }
  method_ = new MethodWrap(this);
}

TFieldDataService::~TFieldDataService()
{
}

void TFieldDataService::slotRequestReceive(){
  if(nullptr != status_){
    int count_req = status_->getParamValue(kRequestsCount).toInt();
    status_->setParam(kRequestsCount, count_req+1,app::OperationState_NORM);
  }
}

void TFieldDataService::slotAnsverSendedError(){
  if(nullptr != status_){
    int count_err = status_->getParamValue(kErorrResponseCount).toInt();
    status_->setParam(kErorrResponseCount, count_err+1,app::OperationState_ERROR);
  }
}

void TFieldDataService::slotAnsverSendedOK(){
  if(nullptr != status_){
    int count_ok = status_->getParamValue(kSuccessResponseCount).toInt();
    status_->setParam(kSuccessResponseCount,count_ok+1,app::OperationState_NORM);
  }
}

void TFieldDataService::GetFieldData(RpcController* c, const DataRequest* req, DataReply* resp, Closure* d)
{
  emit signalRequestReceive();
  rpc::ClientHandler* handler = rpc::Channel::handlerFromController( c );
  if ( nullptr == handler) {
    QString error = QObject::tr("Не удалось получить обработчик из контроллера");
    error_log << error;
    resp->set_comment(error.toStdString());
    resp->set_result(false);
    d->Run();
    return;
  }
  handler->runMethod( c, method_, &MethodWrap::GetFieldData, req, resp);
}

void TFieldDataService::GetGeoTiff(RpcController* c, const DataRequest* req, DataReply* resp, Closure* d)
{
  emit signalRequestReceive();
  rpc::ClientHandler* handler = rpc::Channel::handlerFromController( c );
  if ( nullptr == handler) {
    QString error = QObject::tr("Не удалось получить обработчик из контроллера");
    error_log << error;
    resp->set_comment(error.toStdString());
    resp->set_result(false);
    d->Run();
    return;
  }
  handler->runMethod( c, method_, &MethodWrap::GetGeoTiff, req, resp);
}

void TFieldDataService::GetIsoLinesShp(RpcController* c, const DataRequest* req, IsoLinesShp* resp, Closure* d)
{
  emit signalRequestReceive();
  rpc::ClientHandler* handler = rpc::Channel::handlerFromController( c );
  if ( nullptr == handler) {
    QString error = QObject::tr("Не удалось получить обработчик из контроллера");
    error_log << error;
    resp->set_comment(error.toStdString());
    resp->set_result(false);
    d->Run();
    return;
  }
  handler->runMethod( c, method_, &MethodWrap::GetIsoLinesShp, req, resp);
}


void TFieldDataService::GetFieldsData(RpcController* c, const DataRequest* req, ManyDataReply* resp, Closure* d)
{
  emit signalRequestReceive();
  rpc::ClientHandler* handler = rpc::Channel::handlerFromController( c );
  if ( nullptr == handler) {
    QString error = QObject::tr("Не удалось получить обработчик из контроллера");
    error_log << error;
    resp->set_comment(error.toStdString());
    resp->set_result(false);
    d->Run();
    return;
  }
  handler->runMethod( c, method_, &MethodWrap::GetFieldsData, req, resp);
}

void TFieldDataService::GetFieldsMData(RpcController* c, const DataRequest* req, meteo::surf::DataReply* resp, Closure* d)
{
  emit signalRequestReceive();
  rpc::ClientHandler* handler = rpc::Channel::handlerFromController( c );
  if ( nullptr == handler) {
    QString error = QObject::tr("Не удалось получить обработчик из контроллера");
    error_log << error;
    resp->set_comment(error.toStdString());
    resp->set_result(false);
    d->Run();
    return;
  }
  handler->runMethod( c, method_, &MethodWrap::GetFieldsMData, req, resp);
}

void TFieldDataService::GetFieldsDataWithoutForecast(RpcController* c, const DataRequest* req, ManyDataReply* resp, Closure* d)
{
  emit signalRequestReceive();
  rpc::ClientHandler* handler = rpc::Channel::handlerFromController( c );
  if ( nullptr == handler) {
    QString error = QObject::tr("Не удалось получить обработчик из контроллера");
    error_log << error;
    resp->set_comment(error.toStdString());
    resp->set_result(false);
    d->Run();
    return;
  }
  handler->runMethod( c, method_, &MethodWrap::GetFieldsDataWithoutForecast, req, resp);
}

void TFieldDataService::GetFieldDataPoID(RpcController* c, const SimpleDataRequest* req, DataReply* resp, Closure* d)
{
  emit signalRequestReceive();
  rpc::ClientHandler* handler = rpc::Channel::handlerFromController( c );
  if ( nullptr == handler) {
    QString error = QObject::tr("Не удалось получить обработчик из контроллера");
    error_log << error;
    resp->set_comment(error.toStdString());
    resp->set_result(false);
    d->Run();
    return;
  }
  handler->runMethod( c, method_, &MethodWrap::GetFieldDataPoID, req, resp);
}

void TFieldDataService::GetFieldDataForMeteosummary(TFieldDataService::RpcController* c, const DataRequest *req, ValueDataReply *resp, TFieldDataService::Closure *d)
{
  emit signalRequestReceive();
  rpc::ClientHandler* handler = rpc::Channel::handlerFromController( c );
  if ( nullptr == handler) {
    QString error = QObject::tr("Не удалось получить обработчик из контроллера");
    error_log << error;
    resp->set_comment(error.toStdString());
    resp->set_result(false);
    d->Run();
    return;
  }
  handler->runMethod( c, method_, &MethodWrap::GetFieldDataForMeteosummary, req, resp);
}

void TFieldDataService::GetAvailableCenters(RpcController* c, const DataRequest* req, CentersResponse* resp, Closure* d)
{
  emit signalRequestReceive();
  rpc::ClientHandler* handler = rpc::Channel::handlerFromController( c );
  if ( nullptr == handler) {
    QString error = QObject::tr("Не удалось получить обработчик из контроллера");
    error_log << error;
    resp->set_comment(error.toStdString());
    resp->set_result(false);
    d->Run();
    return;
  }
  handler->runMethod( c, method_, &MethodWrap::GetAvailableCenters, req, resp);
}

void TFieldDataService::GetAvailableCentersForecast(RpcController* c, const DataRequest* req, CentersResponse* resp, Closure* d)
{
  emit signalRequestReceive();
  rpc::ClientHandler* handler = rpc::Channel::handlerFromController( c );
  if ( nullptr == handler) {
    QString error = QObject::tr("Не удалось получить обработчик из контроллера");
    error_log << error;
    resp->set_comment(error.toStdString());
    resp->set_result(false);
    d->Run();
    return;
  }
  handler->runMethod( c, method_, &MethodWrap::GetAvailableCentersForecast, req, resp);
}

void TFieldDataService::GetAvailableCentersAD(RpcController *c, const DataRequest *req, CentersResponse *resp, Closure *d)
{
  emit signalRequestReceive();
  rpc::ClientHandler* handler = rpc::Channel::handlerFromController( c );
  if ( nullptr == handler) {
    QString error = QObject::tr("Не удалось получить обработчик из контроллера");
    error_log << error;
    resp->set_comment(error.toStdString());
    resp->set_result(false);
    d->Run();
    return;
  }
  handler->runMethod( c, method_, &MethodWrap::GetAvailableCentersAD, req, resp);
}

void TFieldDataService::GetAvailableHours(RpcController* c, const DataRequest* req, HourResponse* resp, Closure* d)
{
  emit signalRequestReceive();
  rpc::ClientHandler* handler = rpc::Channel::handlerFromController( c );
  if ( nullptr == handler) {
    QString error = QObject::tr("Не удалось получить обработчик из контроллера");
    error_log << error;
    resp->set_comment(error.toStdString());
    resp->set_result(false);
    d->Run();
    return;
  }
  handler->runMethod( c, method_, &MethodWrap::GetAvailableHours, req, resp);
 }

void TFieldDataService::GetForecastValues(RpcController* c, const DataRequest* req, ValueDataReply* resp, Closure* d)
{
  emit signalRequestReceive();
  rpc::ClientHandler* handler = rpc::Channel::handlerFromController( c );
  if ( nullptr == handler) {
    QString error = QObject::tr("Не удалось получить обработчик из контроллера");
    error_log << error;
    resp->set_comment(error.toStdString());
    resp->set_result(false);
    d->Run();
    return;
  }
  handler->runMethod( c, method_, &MethodWrap::GetForecastValues, req, resp);
}

void TFieldDataService::GetAvailableData(RpcController* c, const DataRequest* req, DataDescResponse* resp, Closure* d)
{
  emit signalRequestReceive();
  rpc::ClientHandler* handler = rpc::Channel::handlerFromController( c );
  if ( nullptr == handler) {
    QString error = QObject::tr("Не удалось получить обработчик из контроллера");
    error_log << error;
    resp->set_comment(error.toStdString());
    resp->set_result(false);
    d->Run();
    return;
  }
  handler->runMethod( c, method_, &MethodWrap::GetAvailableData, req, resp);
}

void TFieldDataService::GetADZond(RpcController *c, const DataRequest *req, surf::TZondValueReply *resp, Closure *d)
{
  emit signalRequestReceive();
  rpc::ClientHandler* handler = rpc::Channel::handlerFromController( c );
  if ( nullptr == handler) {
    QString error = QObject::tr("Не удалось получить обработчик из контроллера");
    error_log << error;
    resp->set_comment(error.toStdString());
    resp->set_result(false);
    d->Run();
    return;
  }
  handler->runMethod( c, method_, &MethodWrap::GetADZond, req, resp);
}

void TFieldDataService::GetLastDate(RpcController* c, const DataRequest* req, SimpleDataReply* resp, Closure* d)
{
  emit signalRequestReceive();
  rpc::ClientHandler* handler = rpc::Channel::handlerFromController( c );
  if ( nullptr == handler) {
    QString error = QObject::tr("Не удалось получить обработчик из контроллера");
    error_log << error;
    resp->set_comment(error.toStdString());
    resp->set_result(false);
    d->Run();
    return;
  }
  handler->runMethod( c, method_, &MethodWrap::GetLastDate, req, resp);
}

void TFieldDataService::GetFieldDates(RpcController* c, const DataRequest *req, DateReply *resp, Closure *d)
{
  emit signalRequestReceive();
  rpc::ClientHandler* handler = rpc::Channel::handlerFromController( c );
  if ( nullptr == handler) {
    QString error = QObject::tr("Не удалось получить обработчик из контроллера");
    error_log << error;
    resp->set_comment(error.toStdString());
    resp->set_result(false);
    d->Run();
    return;
  }
  handler->runMethod( c, method_, &MethodWrap::GetFieldDates, req, resp);
}

void TFieldDataService::GetNearDate(RpcController* c, const DataRequest *req, DateReply* resp, Closure* d)
{
  emit signalRequestReceive();
  rpc::ClientHandler* handler = rpc::Channel::handlerFromController( c );
  if ( nullptr == handler) {
    QString error = QObject::tr("Не удалось получить обработчик из контроллера");
    error_log << error;
    resp->set_comment(error.toStdString());
    resp->set_result(false);
    d->Run();
    return;
  }
  handler->runMethod( c, method_, &MethodWrap::GetNearDate, req, resp);
}

void TFieldDataService::GetFieldsForDate(RpcController* c, const DataRequest *req, ManyFieldsForDatesResponse *resp, Closure *d)
{
  emit signalRequestReceive();
  rpc::ClientHandler* handler = rpc::Channel::handlerFromController( c );
  if ( nullptr == handler) {
    QString error = QObject::tr("Не удалось получить обработчик из контроллера");
    error_log << error;
    resp->set_comment(error.toStdString());
    resp->set_result(false);
    d->Run();
    return;
  }
  handler->runMethod( c, method_, &MethodWrap::GetFieldsForDate, req, resp);
}

void TFieldDataService::GetIsoLines(::google::protobuf::RpcController* c,
                                    const ::meteo::field::DataRequest* req,
                                    ::meteo::IsoLines* resp,
                                    ::google::protobuf::Closure* d)
{
  emit signalRequestReceive();
  rpc::ClientHandler* handler = rpc::Channel::handlerFromController(c);
  if ( nullptr == handler) {
    QString error = QObject::tr("Не удалось получить обработчик из контроллера");
    error_log << error;
    resp->set_comment(error.toStdString());
    resp->set_result(false);
    d->Run();
    return;
  }
  handler->runMethod( c, method_, &MethodWrap::GetIsoLines, req, resp);
}

void TFieldDataService::GetFieldProto(::google::protobuf::RpcController* c,
                                      const ::meteo::field::DataRequest* req,
                                      ::meteo::field::FieldReply* resp,
                                      ::google::protobuf::Closure* d)
{
  emit signalRequestReceive();
  rpc::ClientHandler* handler = rpc::Channel::handlerFromController(c);
  if ( nullptr == handler) {
    QString error = QObject::tr("Не удалось получить обработчик из контроллера");
    error_log << error;
    resp->set_comment(error.toStdString());
    resp->set_result(false);
    d->Run();
    return;
  }
  handler->runMethod( c, method_, &MethodWrap::GetFieldProto, req, resp);
}


}
}


