#include "obanalmaker.h"

#include <cross-commons/debug/tlog.h>
#include <commons/meteo_data/tmeteodescr.h>
#include <commons/textproto/tprototext.h>
#include <meteo/commons/rpc/rpc.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/global/log.h>
#include <sql/nosql/nosqlquery.h>
#include <commons/obanal/haosmain.h>
#include <commons/geobasis/geodata.h>
namespace meteo {
namespace obanal {

ObanalMaker::ObanalMaker( )
  : service_(new meteo::TObanalService())
{
}

ObanalMaker::~ObanalMaker()
{
  delete service_ ;
  service_ = nullptr;
}


bool ObanalMaker::makeWindObanal( const meteo::surf::DataRequest& request ){
  meteo::surf::DataReply* reply = getMeteoData(request);
  if ( nullptr == reply ) {
    error_log << QObject::tr("При попытке выполнить анализ данных ответ от сервиса данных не получен");
    return false;
  }

  bool ok = service_->haveValueRespWind(request, reply );
  delete reply;
  reply = nullptr;
  return ok;
}

bool ObanalMaker::makeObanal( const meteo::surf::DataRequest& request ) {
  meteo::surf::DataReply* reply = getMeteoData(request);
  if ( nullptr == reply ) {
      error_log << QObject::tr("При попытке выполнить анализ данных ответ от сервиса данных не получен");
      return false;
    }
  bool ok = service_->haveValueResp(request, reply );
  delete reply; reply = nullptr;
  return ok;
}


bool ObanalMaker::makeMinMaxTempr(const meteo::surf::DataRequest& request )
{
  if ( nullptr == service_ ) {
    error_log << meteo::msglog::kServiceConnectFailedSimple;
  }
  meteo::rpc::Channel* ctrl = meteo::global::serviceChannel( meteo::settings::proto::kSrcData );
  if(nullptr == ctrl) {
    error_log << meteo::msglog::kServiceConnectFailedSimple
                 .arg(meteo::global::serviceAddress( meteo::settings::proto::kSrcData ));
    return false;
  }
  meteo::surf::DataRequest req;
  req.CopyFrom(request);
  QDateTime dt = NosqlQuery::datetimeFromString( request.date_start() );
  dt.setTime( QTime(0,0,0) );
  req.set_date_start( dt.toString( Qt::ISODate ).toStdString() );
  meteo::surf::DataReply* reply = ctrl->remoteCall( &meteo::surf::SurfaceService::GetMeteoDataByDay, req, 3000000 );
  delete ctrl;
  ctrl = nullptr;
  if ( nullptr == reply ) {
    error_log << QObject::tr("При попытке выполнить анализ данных за сутки ответ от сервиса данных не получен");
    return false;
  }
  bool ok = service_->haveValueResp(req, reply );
  delete reply;
  reply = nullptr;
  return ok;
}

meteo::surf::DataReply* ObanalMaker::getMeteoData( const meteo::surf::DataRequest& request)
{
  if ( nullptr == service_ ) {
      error_log << meteo::msglog::kServiceConnectFailedSimple;
      return nullptr;
    }
  meteo::rpc::Channel* ctrl = meteo::global::serviceChannel( meteo::settings::proto::kSrcData );
  if ( nullptr == ctrl ) {
      error_log << meteo::msglog::kServiceConnectFailedSimple
                   .arg(meteo::global::serviceAddress( meteo::settings::proto::kSrcData ));
      return nullptr;
    }
  meteo::surf::DataReply* reply = ctrl->remoteCall( &meteo::surf::SurfaceService::GetMeteoData, request, 300000 );
  delete ctrl; ctrl = nullptr;
  return reply;
}

bool ObanalMaker::obanalPoDescrs(const surf::DataRequest &rdt){

  meteo::surf::DataRequest request;
  request.CopyFrom(rdt);
  request.set_max_srok(0);//FIXME
  bool ret_val = false;
  for ( int d = 0, descsz = rdt.meteo_descr_size(); d < descsz; ++d ) {
      int descr = rdt.meteo_descr(d);
      request.clear_meteo_descr();
      request.clear_meteo_descrname();
      request.add_meteo_descr(descr);
      request.add_meteo_descrname(TMeteoDescriptor::instance()->name(descr).toStdString()); //в сервисе используется имя
      if(rdt.has_type_level()){
          request.set_type_level(rdt.type_level());
        }
      if(rdt.has_level_p()){
          request.set_level_p(rdt.level_p());
        }
      request.mutable_type()->CopyFrom(rdt.type());
      switch(descr){
        case 11001:
          request.clear_meteo_descrname();
          request.add_meteo_descrname("dd");
          request.add_meteo_descrname("ff");
          ret_val = makeWindObanal( request );
          break;
        case 12111: //Максимальная температура
          request.set_query_type(meteo::surf::kMaximumValue);
          ret_val = makeMinMaxTempr( request );
          break;
        case 12112: //Минимальная температура:
          request.set_query_type(meteo::surf::kMinimumValue);
          ret_val = makeMinMaxTempr( request );
          break;
        case 13013: //Высота снежного покрова
          request.set_query_type(meteo::surf::kMaximumValue);
          ret_val = makeMinMaxTempr( request );
          break;
        case 13023: //Количество осадков за 24 часа:
          request.set_query_type(meteo::surf::kMaximumValue);
          ret_val = makeMinMaxTempr( request );
          break;
        default:
          ret_val = makeObanal( request );
          break;
        }
    }
  return ret_val;
}

bool ObanalMaker::obanalPoGeoData(meteo::GeoData *gd,const meteo::map::proto::WeatherLayer &info){

  return service_->haveValueResp(gd, info );
}




}
}
