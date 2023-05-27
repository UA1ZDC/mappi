#include "gribobanalmaker.h"

#include <QTime>
#include <meteo/commons/proto/surface.pb.h>
#include <meteo/commons/global/global.h>

GribObanalMaker::GribObanalMaker(): ctrl_(nullptr)
{

}

GribObanalMaker::~GribObanalMaker()
{
  if (nullptr != ctrl_) {
    delete ctrl_;
    ctrl_ = nullptr;
  }
}

void GribObanalMaker::init()
{
  ctrl_ =  meteo::global::serviceChannel( meteo::settings::proto::kSrcData );
}


int GribObanalMaker::makeAutoGribObanal(  meteo::TObanalService* service )
{
  int retryCnt = 100;
  while ( nullptr == ctrl_ || false == ctrl_->isConnected() ) {
//    QThread::sleep(1);
    if ( 0 == retryCnt ) {
      return AUTOOBANAL_CONNECTION_REFUSED;
    }
    if ( nullptr != ctrl_ ) {
      delete ctrl_;
      ctrl_ = nullptr;
    }
    init();
    --retryCnt;
  }

  QTime ttt; ttt.start();
  meteo::surf::EmptyDataRequest grib_request;
  int result = 1;
  meteo::surf::SimpleDataRequest req;

  QTime ttt_not_analysed; ttt_not_analysed.start();
  meteo::surf::GribDataReply* reply = ctrl_->remoteCall( &meteo::surf::SurfaceService::GetNotAnalysedGribData, grib_request, 10000);

  if ( nullptr == reply ) {
    error_log << QObject::tr("При попытке выполнить анализ данных в коде GRIB ответ от сервиса данных не получен");
    req.set_text("not implemented yet");
    delete reply;
    return AUTOOBANAL_NOREPLY;
  }
  else if ( ( true == reply->has_finished() ) && ( true == reply->finished() ) ){
    delete reply;
    return AUTOOBANAL_FINISHED;
  }
  else if ( true == reply->has_result() &&  false == reply->result()  ){
    error_log << QObject::tr("Произошла ошибка при выполнении запроса к сервису: %1")
                   .arg(QString::fromStdString(reply->comment()));
    return AUTOOBANAL_ERROR;
  }
  else if ( false == reply->has_info()) {
    req.set_text("not implemented yet");
    //debug_log << reply->Utf8DebugString();
    delete reply;
    return AUTOOBANAL_NOINFO;
  }
  else if (!service->haveValueRespGrib(reply)) {
    req.set_text("not implemented yet");
  }
  *req.mutable_id() = reply->ids();
  delete reply; reply = nullptr;
  if ( req.id_size() > 0) {
    QTime ttt_remove; ttt_remove.start();
    meteo::surf::SimpleDataReply* sreply = ctrl_->remoteCall( &meteo::surf::SurfaceService::RemoveGribDataAnalysed, req , 10000);
    if ( nullptr == sreply ) {
      error_log << QObject::tr("При попытке установить флаг анализа данных в коде GRIB ответ от сервиса данных не получен");
      result = -1;
    } 
    else {
      result = sreply->result();
      delete sreply;
    }
  }
  else {
    return AUTOOBANAL_NODATA;
  }
  return result;
}

int GribObanalMaker::makeCustomObanal(meteo::TObanalService *service, const meteo::surf::GribDataRequest& request)
{
  QTime ttt; ttt.start();
  int retryCnt = 100;
  while ( (nullptr == ctrl_ ) || false == ctrl_->isConnected() ) {
//    QThread::sleep(1);
    if ( 0 == retryCnt ) {
      return AUTOOBANAL_CONNECTION_REFUSED;
    }
    if (nullptr != ctrl_) {
      delete ctrl_;
      ctrl_ = nullptr;
    }
    init();
    --retryCnt;
  }
  int result = 1;
  meteo::surf::SimpleDataRequest req;
  QTime ttt_not_analysed; ttt_not_analysed.start();
  meteo::surf::GribDataReply* reply = ctrl_->remoteCall( &meteo::surf::SurfaceService::GetGribData, request, 10000);
  if ( nullptr == reply ) {
    error_log << QObject::tr("При попытке выполнить анализ данных в коде GRIB ответ от сервиса данных не получен");
    req.set_text("not implemented yet");
    return AUTOOBANAL_NOREPLY;
  }
  else if ( ( true == reply->has_finished() ) && ( true == reply->finished() ) ){
    delete reply;
    return AUTOOBANAL_FINISHED;
  }
  else if ( true == reply->has_result() &&  false == reply->result()  ){
    error_log << QObject::tr("Произошла ошибка при выполнении запроса к сервису: %1")
                   .arg(QString::fromStdString(reply->comment()));
    return AUTOOBANAL_ERROR;
  }
  else if ( false == reply->has_info()) {
    req.set_text("not implemented yet");
    debug_log << reply->Utf8DebugString();
    delete reply;
    return AUTOOBANAL_NOINFO;
  }
  else if ( false == service->haveValueRespGrib(reply) ) {
    req.set_text("not implemented yet");
  }
  *req.mutable_id() = reply->ids();
  delete reply; reply = nullptr;
  if ( req.id_size() > 0 ) {
    QTime ttt_remove; ttt_remove.start();
    meteo::surf::SimpleDataReply* sreply = ctrl_->remoteCall( &meteo::surf::SurfaceService::RemoveGribDataAnalysed, req , 10000);
    if ( nullptr == sreply ) {
      error_log << QObject::tr("При попытке установить флаг анализа данных в коде GRIB ответ от сервиса данных не получен");
      result = -1;
    }
    else {
      result = sreply->result();
      delete sreply;
    }
  }
  else {
    return AUTOOBANAL_NODATA;
  }
  return result;
}
