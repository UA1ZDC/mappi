#include "sprinfservice.h"
#include "clienthandler.h"

#include <cross-commons/debug/tlog.h>

#include <commons/geobasis/projection.h>
#include <commons/meteo_data/meteo_data.h>
#include <commons/meteo_data/tmeteodescr.h>
#include <commons/textproto/pbtools.h>

#include <meteo/commons/rpc/rpc.h>
#include <meteo/commons/global/global.h>

#include <sql/dbi/dbi.h>
#include <sql/dbi/dbiquery.h>

#include <commons/mathtools/mnmath.h>

#include <qset.h>
#include <qscopedpointer.h>
#include <qstring.h>
#include <qdatetime.h>
#include <QtConcurrent/QtConcurrent>

namespace meteo {
namespace sprinf {

TSprinfService::TSprinfService(AppStatusThread* status)
  :SprinfService(),
   handler_(new ClientHandler(this)),
   lockEditDb_(false),
   status_(status)
  {
  if(nullptr != status_){
    status_->setTitle(kRequestsCount, QObject::tr("Получено запросов"));
    status_->setTitle(kSuccessResponseCount, QObject::tr("Отправлено ответов"));
    status_->setTitle(kErorrResponseCount, QObject::tr("Ошибок при обработке запросов"));
    status_->setParam(kRequestsCount, 0,app::OperationState_NONE);
    status_->setParam(kSuccessResponseCount, 0,app::OperationState_NONE);
    status_->setParam(kErorrResponseCount, 0,app::OperationState_NONE);
    QObject::connect( this, &meteo::sprinf::TSprinfService::signalRequestReceive,
                      this,&meteo::sprinf::TSprinfService::slotRequestReceive );
  }
}
TSprinfService::~TSprinfService() {
  delete handler_; handler_ = nullptr;
}

void TSprinfService::slotRequestReceive(){
  if(nullptr != status_){
    int count_req = status_->getParamValue(kRequestsCount).toInt();
    status_->setParam(kRequestsCount, count_req+1,app::OperationState_NORM);
  }
}

void TSprinfService::slotAnsverSendedError(){
  if(nullptr != status_){
    int count_err = status_->getParamValue(kErorrResponseCount).toInt();
    status_->setParam(kErorrResponseCount, count_err+1,app::OperationState_ERROR);
  }
}

void TSprinfService::slotAnsverSendedOK(){
  if(nullptr != status_){
    int count_ok = status_->getParamValue(kSuccessResponseCount).toInt();
    status_->setParam(kSuccessResponseCount,count_ok+1,app::OperationState_NORM);
  }
}

void TSprinfService::GetStationTypes(google::protobuf::RpcController* controller,
                                     const TypesRequest* request,
                                     StationTypes* response,
                                     google::protobuf::Closure* done)
{
  emit signalRequestReceive();
  if (request == 0 || response == 0 || done == 0) {
    error_log << QString::fromUtf8("Ошибка. Переданы некорректные параметры");
    if (done != 0) {
      done->Run();
    }
    return;
  }
  rpc::Controller* tctrl = static_cast<rpc::Controller*>(controller);
  if(nullptr == tctrl->channel()) {
    QString err = QObject::tr("Не найден канал связи для выполнения удаленного вызова");
    error_log << err;
    response->set_result(false);
    response->set_error_message(err.toStdString());
    done->Run();
    return;
  }
  if(false == handler_->contains(tctrl->channel())) {
    QString err = QObject::tr("Не найден обработчик соединения при создании канала");
    error_log << err;
    response->set_result(false);
    response->set_error_message(err.toStdString());
    done->Run();
    return;
  }
  if(false == handler_->runMethod(tctrl->channel(), &HandlerConnection::GetStationTypes, request, response, done)) {
    QString err = QObject::tr("Не удалось вызвать метод обработчика");
    error_log << err;
    response->set_result(false);
    response->set_error_message(err.toStdString());
    done->Run();
    return;
  }
}

void TSprinfService::GetGmiTypes(google::protobuf::RpcController* controller,
                                 const TypesRequest* request,
                                 GmiTypes* response,
                                 google::protobuf::Closure* done)
{
  emit signalRequestReceive();
  if (request == 0 || response == 0 || done == 0) {
    error_log << QString::fromUtf8("Ошибка. Переданы некорректные параметры");
    if (done != 0) {
      done->Run();
    }
    return;
  }
  rpc::Controller* tctrl = static_cast<rpc::Controller*>(controller);
  if(nullptr == tctrl->channel()) {
    QString err = QObject::tr("Не найден канал связи для выполнения удаленного вызова");
    error_log << err;
    response->set_result(false);
    response->set_error_message(err.toStdString());
    done->Run();
    return;
  }
  if(false == handler_->contains(tctrl->channel())) {
    QString err = QObject::tr("Не найден обработчик соединения при создании канала");
    error_log << err;
    response->set_result(false);
    response->set_error_message(err.toStdString());
    done->Run();
    return;
  }
  if(false == handler_->runMethod(tctrl->channel(), &HandlerConnection::GetGmiTypes, request, response, done)) {
    QString err = QObject::tr("Не удалось вызвать метод обработчика");
    error_log << err;
    response->set_result(false);
    response->set_error_message(err.toStdString());
    done->Run();
    return;
  }
}

void TSprinfService::GetLevelTypes(google::protobuf::RpcController* controller,
                                   const TypesRequest* request,
                                   LevelTypes* response,
                                   google::protobuf::Closure* done)
{
  emit signalRequestReceive();
  if (request == 0 || response == 0 || done == 0) {
    error_log << QString::fromUtf8("Ошибка. Переданы некорректные параметры");
    if (done != 0) {
      done->Run();
    }
    return;
  }
  rpc::Controller* tctrl = static_cast<rpc::Controller*>(controller);
  if(nullptr == tctrl->channel()) {
    QString err = QObject::tr("Не найден канал связи для выполнения удаленного вызова");
    error_log << err;
    response->set_result(false);
    response->set_error_message(err.toStdString());
    done->Run();
    return;
  }
  if(false == handler_->contains(tctrl->channel())) {
    QString err = QObject::tr("Не найден обработчик соединения при создании канала");
    error_log << err;
    response->set_result(false);
    response->set_error_message(err.toStdString());
    done->Run();
    return;
  }
  if(false == handler_->runMethod(tctrl->channel(), &HandlerConnection::GetLevelTypes, request, response, done)) {
    QString err = QObject::tr("Не удалось вызвать метод обработчика");
    error_log << err;
    response->set_result(false);
    response->set_error_message(err.toStdString());
    done->Run();
    return;
  }
}

void TSprinfService::GetMilitaryDistrict(google::protobuf::RpcController* controller,
                 const StatementRequest* request,
                 Regions* response,
                 google::protobuf::Closure* done)
{
  emit signalRequestReceive();
  if ( ( nullptr == request )
       || ( nullptr == response )
       || ( nullptr == done ) ) {
    error_log << QString::fromUtf8("Ошибка. Переданы некорректные параметры");
    if (done != nullptr) {
      done->Run();
    }
    return;
  }
  rpc::Controller* tctrl = static_cast<rpc::Controller*>(controller);
  if(nullptr == tctrl->channel()) {
    QString err = QObject::tr("Не найден канал связи для выполнения удаленного вызова");
    error_log << err;
    response->set_result(false);
    response->set_error_message(err.toStdString());
    done->Run();
    return;
  }
  if(false == handler_->contains(tctrl->channel())) {
    QString err = QObject::tr("Не найден обработчик соединения при создании канала");
    error_log << err;
    response->set_result(false);
    response->set_error_message(err.toStdString());
    done->Run();
    return;
  }
  if(false == handler_->runMethod(tctrl->channel(), &HandlerConnection::GetMilitaryDistrict, request, response, done)) {
    QString err = QObject::tr("Не удалось вызвать метод обработчика");
    error_log << err;
    response->set_result(false);
    response->set_error_message(err.toStdString());
    done->Run();
    return;
  }
}



void TSprinfService::GetRegions(google::protobuf::RpcController* controller,
                                 const MultiStatementRequest* request,
                                 Regions* response,
                                 google::protobuf::Closure* done)
{
  emit signalRequestReceive();
  if ( ( nullptr == request )
       || ( nullptr == response )
       || ( nullptr == done ) ) {
    error_log << QString::fromUtf8("Ошибка. Переданы некорректные параметры");
    if (done != nullptr) {
      done->Run();
    }
    return;
  }
  rpc::Controller* tctrl = static_cast<rpc::Controller*>(controller);
  if(nullptr == tctrl->channel()) {
    QString err = QObject::tr("Не найден канал связи для выполнения удаленного вызова");
    error_log << err;
    response->set_result(false);
    response->set_error_message(err.toStdString());
    done->Run();
    return;
  }
  if(false == handler_->contains(tctrl->channel())) {
    QString err = QObject::tr("Не найден обработчик соединения при создании канала");
    error_log << err;
    response->set_result(false);
    response->set_error_message(err.toStdString());
    done->Run();
    return;
  }
  if(false == handler_->runMethod(tctrl->channel(), &HandlerConnection::GetRegions, request, response, done)) {
    QString err = QObject::tr("Не удалось вызвать метод обработчика");
    error_log << err;
    response->set_result(false);
    response->set_error_message(err.toStdString());
    done->Run();
    return;
  }
}



/**
 * @brief  Получаем группы регионов из базы. либо все, либо по айди
 * 
 * @param c 
 * @param request 
 * @param response 
 * @param done 
 */
void TSprinfService::GetRegionGroups( google::protobuf::RpcController* controller,
                                          const RegionGroupsRequest* request,
                                          RegionGroupsReply* response,
                                          google::protobuf::Closure* done ){

  emit signalRequestReceive();
  if ( ( nullptr == request )
       || ( nullptr == response )
       || ( nullptr == done ) ) {
    error_log << QString::fromUtf8("Ошибка. Переданы некорректные параметры GetRegionGroups");
    if (done != nullptr) {
      done->Run();
    }
    return;
  }
  rpc::Controller* tctrl = static_cast<rpc::Controller*>(controller);
  if(nullptr == tctrl->channel()) {
    QString err = QObject::tr("Не найден канал связи для выполнения удаленного вызова GetRegionGroups");
    error_log << err;
    response->set_result(false);
    response->set_comment(err.toStdString());
    done->Run();
    return;
  }
  if(false == handler_->contains(tctrl->channel())) {
    QString err = QObject::tr("Не найден обработчик соединения при создании канала GetRegionGroups");
    error_log << err;
    response->set_result(false);
    response->set_comment(err.toStdString());
    done->Run();
    return;
  }
  if(false == handler_->runMethod(tctrl->channel(), &HandlerConnection::GetRegionGroups, request, response, done)) {
    QString err = QObject::tr("Не удалось вызвать метод обработчика GetRegionGroups");
    error_log << err;
    response->set_result(false);
    response->set_comment(err.toStdString());
    done->Run();
    return;
  }
}


/**
 * @brief  Сохраняем/создаем новую группу регионов/станций в базе
 * 
 * @param c 
 * @param request 
 * @param response 
 * @param done 
 */
void TSprinfService::SaveRegionGroups( google::protobuf::RpcController* controller,
                                          const RegionGroupsRequest* request,
                                          RegionGroupsReply* response,
                                          google::protobuf::Closure* done ){
  emit signalRequestReceive();
  if ( ( nullptr == request )
       || ( nullptr == response )
       || ( nullptr == done ) ) {
    error_log << QString::fromUtf8("Ошибка. Переданы некорректные параметры SaveRegionGroups");
    if (done != nullptr) {
      done->Run();
    }
    return;
  }
  rpc::Controller* tctrl = static_cast<rpc::Controller*>(controller);
  if(nullptr == tctrl->channel()) {
    QString err = QObject::tr("Не найден канал связи для выполнения удаленного вызова SaveRegionGroups");
    error_log << err;
    response->set_result(false);
    response->set_comment(err.toStdString());
    done->Run();
    return;
  }
  if(false == handler_->contains(tctrl->channel())) {
    QString err = QObject::tr("Не найден обработчик соединения при создании канала SaveRegionGroups");
    error_log << err;
    response->set_result(false);
    response->set_comment(err.toStdString());
    done->Run();
    return;
  }
  if(false == handler_->runMethod(tctrl->channel(), &HandlerConnection::SaveRegionGroups, request, response, done)) {
    QString err = QObject::tr("Не удалось вызвать метод обработчика SaveRegionGroups");
    error_log << err;
    response->set_result(false);
    response->set_comment(err.toStdString());
    done->Run();
    return;
  }
  
}


/**
 * @brief Удаляем группу регионов из базы по айди
 * 
 * @param c 
 * @param request 
 * @param response 
 * @param done 
 */
void TSprinfService::DeleteRegionGroups( google::protobuf::RpcController* controller,
                                          const RegionGroupsRequest* request,
                                          RegionGroupsReply* response,
                                          google::protobuf::Closure* done ){

  
  emit signalRequestReceive();
  if ( ( nullptr == request )
       || ( nullptr == response )
       || ( nullptr == done ) ) {
    error_log << QString::fromUtf8("Ошибка. Переданы некорректные параметры DeleteRegionGroups");
    if (done != nullptr) {
      done->Run();
    }
    return;
  }
  rpc::Controller* tctrl = static_cast<rpc::Controller*>(controller);
  if(nullptr == tctrl->channel()) {
    QString err = QObject::tr("Не найден канал связи для выполнения удаленного вызова DeleteRegionGroups");
    error_log << err;
    response->set_result(false);
    response->set_comment(err.toStdString());
    done->Run();
    return;
  }
  if(false == handler_->contains(tctrl->channel())) {
    QString err = QObject::tr("Не найден обработчик соединения при создании канала DeleteRegionGroups");
    error_log << err;
    response->set_result(false);
    response->set_comment(err.toStdString());
    done->Run();
    return;
  }
  if(false == handler_->runMethod(tctrl->channel(), &HandlerConnection::DeleteRegionGroups, request, response, done)) {
    QString err = QObject::tr("Не удалось вызвать метод обработчика DeleteRegionGroups");
    error_log << err;
    response->set_result(false);
    response->set_comment(err.toStdString());
    done->Run();
    return;
  }
  
}




void TSprinfService::GetStations(google::protobuf::RpcController* controller,
                                 const MultiStatementRequest* request,
                                 Stations* response,
                                 google::protobuf::Closure* done)
{
  emit signalRequestReceive();
  if ( ( nullptr == request )
       || ( nullptr == response )
       || ( nullptr == done ) ) {
    error_log << QString::fromUtf8("Ошибка. Переданы некорректные параметры");
    if (done != nullptr) {
      done->Run();
    }
    return;
  }
  rpc::Controller* tctrl = static_cast<rpc::Controller*>(controller);
  if(nullptr == tctrl->channel()) {
    QString err = QObject::tr("Не найден канал связи для выполнения удаленного вызова");
    error_log << err;
    response->set_result(false);
    response->set_error_message(err.toStdString());
    done->Run();
    return;
  }
  if(false == handler_->contains(tctrl->channel())) {
    QString err = QObject::tr("Не найден обработчик соединения при создании канала");
    error_log << err;
    response->set_result(false);
    response->set_error_message(err.toStdString());
    done->Run();
    return;
  }
  if(false == handler_->runMethod(tctrl->channel(), &HandlerConnection::GetStations, request, response, done)) {
    QString err = QObject::tr("Не удалось вызвать метод обработчика");
    error_log << err;
    response->set_result(false);
    response->set_error_message(err.toStdString());
    done->Run();
    return;
  }
}

void TSprinfService::GetStationsByPosition(google::protobuf::RpcController* controller,
                                           const CircleRegionRequest* request,
                                           StationsWithDistance* response,
                                           google::protobuf::Closure* done)
{
  emit signalRequestReceive();
  if (request == 0 || response == 0 || done == 0) {
    error_log << QString::fromUtf8("Ошибка. Переданы некорректные параметры");
    if (done != 0) {
      done->Run();
    }
    return;
  }
  rpc::Controller* tctrl = static_cast<rpc::Controller*>(controller);
  if(nullptr == tctrl->channel()) {
    QString err = QObject::tr("Не найден канал связи для выполнения удаленного вызова");
    error_log << err;
    response->set_result(false);
    response->set_error_message(err.toStdString());
    done->Run();
    return;
  }
  if(false == handler_->contains(tctrl->channel())) {
    QString err = QObject::tr("Не найден обработчик соединения при создании канала");
    error_log << err;
    response->set_result(false);
    response->set_error_message(err.toStdString());
    done->Run();
    return;
  }
  if(false == handler_->runMethod(tctrl->channel(), &HandlerConnection::GetStationsByPosition, request, response, done)) {
    QString err = QObject::tr("Не удалось вызвать метод обработчика");
    error_log << err;
    response->set_result(false);
    response->set_error_message(err.toStdString());
    done->Run();
    return;
  }
}

void TSprinfService::GetMeteoCenters(google::protobuf::RpcController* controller,
                                     const MeteoCenterRequest* request,
                                     MeteoCenters* response,
                                     google::protobuf::Closure* done)
{
  emit signalRequestReceive();
  if (request == 0 || response == 0 || done == 0) {
    error_log << QString::fromUtf8("Ошибка. Переданы некорректные параметры");
    if (done != 0) {
      done->Run();
    }
    return;
  }
  rpc::Controller* tctrl = static_cast<rpc::Controller*>(controller);
  if(nullptr == tctrl->channel()) {
    QString err = QObject::tr("Не найден канал связи для выполнения удаленного вызова");
    error_log << err;
    response->set_result(false);
    response->set_error_message(err.toStdString());
    done->Run();
    return;
  }
  if(false == handler_->contains(tctrl->channel())) {
    QString err = QObject::tr("Не найден обработчик соединения при создании канала");
    error_log << err;
    response->set_result(false);
    response->set_error_message(err.toStdString());
    done->Run();
    return;
  }
  if(false == handler_->runMethod(tctrl->channel(), &HandlerConnection::GetMeteoCenters, request, response, done)) {
    QString err = QObject::tr("Не удалось вызвать метод обработчика");
    error_log << err;
    response->set_result(false);
    response->set_error_message(err.toStdString());
    done->Run();
    return;
  }
}

void TSprinfService::GetMeteoParametersByBufr(google::protobuf::RpcController* controller,
                                              const MeteoParameterRequest* request,
                                              MeteoParameters* response,
                                              google::protobuf::Closure* done)
{
  emit signalRequestReceive();
  if (request == 0 || response == 0 || done == 0) {
    error_log << QString::fromUtf8("Ошибка. Переданы некорректные параметры");
    if (done != 0) {
      done->Run();
    }
    return;
  }
  rpc::Controller* tctrl = static_cast<rpc::Controller*>(controller);
  if(nullptr == tctrl->channel()) {
    QString err = QObject::tr("Не найден канал связи для выполнения удаленного вызова");
    error_log << err;
    response->set_result(false);
    response->set_error_message(err.toStdString());
    done->Run();
    return;
  }
  if(false == handler_->contains(tctrl->channel())) {
    QString err = QObject::tr("Не найден обработчик соединения при создании канала");
    error_log << err;
    response->set_result(false);
    response->set_error_message(err.toStdString());
    done->Run();
    return;
  }
  if(false == handler_->runMethod(tctrl->channel(), &HandlerConnection::GetMeteoParametersByBufr, request, response, done)) {
    QString err = QObject::tr("Не удалось вызвать метод обработчика");
    error_log << err;
    response->set_result(false);
    response->set_error_message(err.toStdString());
    done->Run();
    return;
  }
}


void TSprinfService::GetBufrParametersTables(google::protobuf::RpcController* controller,
                                             const BufrParametersTableRequest* request,
                                             BufrParametersTables* response,
                                             google::protobuf::Closure* done)
{
  emit signalRequestReceive();
  if (request == 0 || response == 0 || done == 0) {
    error_log << QString::fromUtf8("Ошибка. Переданы некорректные параметры");
    if (done != 0) {
      done->Run();
    }
    return;
  }
  rpc::Controller* tctrl = static_cast<rpc::Controller*>(controller);
  if(nullptr == tctrl->channel()) {
    QString err = QObject::tr("Не найден канал связи для выполнения удаленного вызова");
    error_log << err;
    response->set_result(false);
    response->set_error_message(err.toStdString());
    done->Run();
    return;
  }
  if(false == handler_->contains(tctrl->channel())) {
    QString err = QObject::tr("Не найден обработчик соединения при создании канала");
    error_log << err;
    response->set_result(false);
    response->set_error_message(err.toStdString());
    done->Run();
    return;
  }
  if(false == handler_->runMethod(tctrl->channel(), &HandlerConnection::GetBufrParametersTables, request, response, done)) {
    QString err = QObject::tr("Не удалось вызвать метод обработчика");
    error_log << err;
    response->set_result(false);
    response->set_error_message(err.toStdString());
    done->Run();
    return;
  }

}

void TSprinfService::GetCountry(::google::protobuf::RpcController* controller,
                                const ::meteo::sprinf::CountryRequest* request,
                                ::meteo::sprinf::Countries* response,
                                ::google::protobuf::Closure* done)
{
  emit signalRequestReceive();
  if (request == 0 || response == 0 || done == 0) {
    error_log << QString::fromUtf8("Ошибка. Переданы некорректные параметры");
    if (done != 0) {
      done->Run();
    }
    return;
  }
  rpc::Controller* tctrl = static_cast<rpc::Controller*>(controller);
  if(nullptr == tctrl->channel()) {
    QString err = QObject::tr("Не найден канал связи для выполнения удаленного вызова");
    error_log << err;
    response->set_result(false);
    response->set_comment(err.toStdString());
    done->Run();
    return;
  }
  if(false == handler_->contains(tctrl->channel())) {
    QString err = QObject::tr("Не найден обработчик соединения при создании канала");
    error_log << err;
    response->set_result(false);
    response->set_comment(err.toStdString());
    done->Run();
    return;
  }
  if(false == handler_->runMethod(tctrl->channel(), &HandlerConnection::GetCountry, request, response, done)) {
    QString err = QObject::tr("Не удалось вызвать метод обработчика");
    error_log << err;
    response->set_result(false);
    response->set_comment(err.toStdString());
    done->Run();
    return;
  }
}

void TSprinfService::UpdateStation(google::protobuf::RpcController *controller, const Station *request, ReportStationsAdded *response, google::protobuf::Closure *done)
{
  emit signalRequestReceive();
  if (request == nullptr || response == nullptr || done == nullptr) {
    error_log << QString::fromUtf8("Ошибка. Переданы некорректные параметры");
    if (done != nullptr) {
      done->Run();
    }
    return;
  }
  rpc::Controller* tctrl = static_cast<rpc::Controller*>(controller);
  if(nullptr == tctrl->channel()) {
    QString err = QObject::tr("Не найден канал связи для выполнения удаленного вызова");
    error_log << err;
    response->set_result(false);
    response->set_comment(err.toStdString());
    done->Run();
    return;
  }
  if(false == handler_->contains(tctrl->channel())) {
    QString err = QObject::tr("Не найден обработчик соединения при создании канала");
    error_log << err;
    response->set_result(false);
    response->set_comment(err.toStdString());
    done->Run();
    return;
  }
  if(false == handler_->runMethod(tctrl->channel(), &HandlerConnection::UpdateStation, request, response, done)) {
    QString err = QObject::tr("Не удалось вызвать метод обработчика");
    error_log << err;
    response->set_result(false);
    response->set_comment(err.toStdString());
    done->Run();
    return;
  }
}

void TSprinfService::DeleteStation(google::protobuf::RpcController *controller,
                                   const Station *request,
                                   ReportStationsAdded *response,
                                   google::protobuf::Closure *done)
{
  emit signalRequestReceive();
  if (request == nullptr || response == nullptr || done == nullptr) {
    error_log << QString::fromUtf8("Ошибка. Переданы некорректные параметры");
    if (done != nullptr) {
      done->Run();
    }
    return;
  }
  rpc::Controller* tctrl = static_cast<rpc::Controller*>(controller);
  if(nullptr == tctrl->channel()) {
    QString err = QObject::tr("Не найден канал связи для выполнения удаленного вызова");
    error_log << err;
    response->set_result(false);
    response->set_comment(err.toStdString());
    done->Run();
    return;
  }
  if(false == handler_->contains(tctrl->channel())) {
    QString err = QObject::tr("Не найден обработчик соединения при создании канала");
    error_log << err;
    response->set_result(false);
    response->set_comment(err.toStdString());
    done->Run();
    return;
  }
  if(false == handler_->runMethod(tctrl->channel(), &HandlerConnection::DeleteStation,
                                  request,
                                  response,
                                  done)) {
    QString err = QObject::tr("Не удалось вызвать метод обработчика");
    error_log << err;
    response->set_result(false);
    response->set_comment(err.toStdString());
    done->Run();
    return;
  }
}

void TSprinfService::GetTlgStatistic(::google::protobuf::RpcController* controller,
                                     const ::meteo::sprinf::TlgStatisticRequest* request,
                                     TlgStatisticResponse *response,
                                     ::google::protobuf::Closure* done)
{
  emit signalRequestReceive();
  if (request == nullptr || response == nullptr || done == nullptr) {
    error_log << QString::fromUtf8("Ошибка. Переданы некорректные параметры");
    if (done != nullptr) {
      done->Run();
    }
    return;
  }
  rpc::Controller* tctrl = static_cast<rpc::Controller*>(controller);
  if(nullptr == tctrl->channel()) {
    QString err = QObject::tr("Не найден канал связи для выполнения удаленного вызова");
    error_log << err;
    response->set_result(false);
    response->set_comment(err.toStdString());
    done->Run();
    return;
  }
  if(false == handler_->contains(tctrl->channel())) {
    QString err = QObject::tr("Не найден обработчик соединения при создании канала");
    error_log << err;
    response->set_result(false);
    response->set_comment(err.toStdString());
    done->Run();
    return;
  }
  if(false == handler_->runMethod(tctrl->channel(), &HandlerConnection::GetTlgStatistic,
                                  request,
                                  response,
                                  done)) {
    QString err = QObject::tr("Не удалось вызвать метод обработчика");
    error_log << err;
    response->set_result(false);
    response->set_comment(err.toStdString());
    done->Run();
    return;
  }


}

void TSprinfService::GetCities(::google::protobuf::RpcController* controller,
                       const ::meteo::sprinf::Dummy* request,
                       ::meteo::map::proto::Cities* response,
                       ::google::protobuf::Closure* done)
{
  emit signalRequestReceive();
  if (request == nullptr || response == nullptr || done == nullptr) {
    error_log << QString::fromUtf8("Ошибка. Переданы некорректные параметры");
    if (done != nullptr) {
      done->Run();
    }
    return;
  }
  rpc::Controller* tctrl = static_cast<rpc::Controller*>(controller);
  if(nullptr == tctrl->channel()) {
    QString err = QObject::tr("Не найден канал связи для выполнения удаленного вызова");
    error_log << err;
    response->set_result(false);
    response->set_comment(err.toStdString());
    done->Run();
    return;
  }
  if(false == handler_->contains(tctrl->channel())) {
    QString err = QObject::tr("Не найден обработчик соединения при создании канала");
    error_log << err;
    response->set_result(false);
    response->set_comment(err.toStdString());
    done->Run();
    return;
  }
  if(false == handler_->runMethod(tctrl->channel(), &HandlerConnection::GetCities,
                                  request,
                                  response,
                                  done)) {
    QString err = QObject::tr("Не удалось вызвать метод обработчика GetCities");
    error_log << err;
    response->set_result(false);
    response->set_comment(err.toStdString());
    done->Run();
    return;
  }
}

void TSprinfService::GetCityDisplayConfig(::google::protobuf::RpcController* controller,
                       const ::meteo::sprinf::Dummy* request,
                       ::meteo::map::proto::CityDisplayConfig* response,
                       ::google::protobuf::Closure* done)
{
  emit signalRequestReceive();
  if (request == nullptr || response == nullptr || done == nullptr) {
    error_log << QString::fromUtf8("Ошибка. Переданы некорректные параметры");
    if (done != nullptr) {
      done->Run();
    }
    return;
  }
  rpc::Controller* tctrl = static_cast<rpc::Controller*>(controller);
  if(nullptr == tctrl->channel()) {
    QString err = QObject::tr("Не найден канал связи для выполнения удаленного вызова");
    error_log << err;
    response->set_result(false);
    response->set_comment(err.toStdString());
    done->Run();
    return;
  }
  if(false == handler_->contains(tctrl->channel())) {
    QString err = QObject::tr("Не найден обработчик соединения при создании канала");
    error_log << err;
    response->set_result(false);
    response->set_comment(err.toStdString());
    done->Run();
    return;
  }
  if(false == handler_->runMethod(tctrl->channel(), &HandlerConnection::GetCityDisplayConfig,
                                  request,
                                  response,
                                  done)) {
    QString err = QObject::tr("Не удалось вызвать метод обработчика GetCityDisplayConfig");
    error_log << err;
    response->set_result(false);
    response->set_comment(err.toStdString());
    done->Run();
    return;
  }
}

}
}
