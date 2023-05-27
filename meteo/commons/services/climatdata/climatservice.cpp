#include "climatservice.h"

#include <qmap.h>
#include <qprocess.h>
#include <qstring.h>
#include <qcoreapplication.h>

#include <cross-commons/debug/tlog.h>
#include <meteo/commons/global/global.h>
#include <sql/nosql/nosqlquery.h>
#include <sql/dbi/dbi.h>
#include <sql/dbi/dbiquery.h>


using namespace meteo;

climat::ClimatService::ClimatService(AppStatusThread* status)
  :status_(status)
{
  if(nullptr != status_){
    status_->setTitle(kRequestsCount, tr("Получено запросов"));
    status_->setTitle(kSuccessResponseCount, tr("Отправлено ответов"));
    status_->setTitle(kErorrResponseCount, tr("Ошибок при обработке запросов"));
    status_->setParam(kRequestsCount, 0,app::OperationState_NONE);
    status_->setParam(kSuccessResponseCount, 0,app::OperationState_NONE);
    status_->setParam(kErorrResponseCount, 0,app::OperationState_NONE);
    QObject::connect( this, &meteo::climat::ClimatService::signalRequestReceive,
                      this,&meteo::climat::ClimatService::slotRequestReceive );
  }

}

climat::ClimatService::~ClimatService()
{
}

void climat::ClimatService::GetAvailable( google::protobuf::RpcController* c,
            const meteo::climat::Dummy* req,
            meteo::climat::DataSeriesDate* res,
            google::protobuf::Closure* d )
{
  emit signalRequestReceive();
  QCoreApplication::processEvents();
  Q_UNUSED(c);
  Q_UNUSED(req);
  if (!getAvailable(res)&&!res->has_error()) {
    res->set_error("Запрос задан некорректно или ошибка выполнения запроса");
  }
  d->Run();
}

void climat::ClimatService::slotRequestReceive(){
  if(nullptr != status_){
    int count_req = status_->getParamValue(kRequestsCount).toInt();
    status_->setParam(kRequestsCount, count_req+1,app::OperationState_NORM);
  }
}

void climat::ClimatService::slotAnsverSendedError(){
  if(nullptr != status_){
    int count_err = status_->getParamValue(kErorrResponseCount).toInt();
    status_->setParam(kErorrResponseCount, count_err+1,app::OperationState_ERROR);
  }
}

void climat::ClimatService::slotAnsverSendedOK(){
  if(nullptr != status_){
    int count_ok = status_->getParamValue(kSuccessResponseCount).toInt();
    status_->setParam(kSuccessResponseCount,count_ok+1,app::OperationState_NORM);
  }
}
bool climat::ClimatService::getAvailable(meteo::climat::DataSeriesDate* res)
{
  bool ok = getAvailable("aero", meteo::sprinf::kStationAero,  res);
  ok |= getAvailable("surf_hour", meteo::sprinf::kStationSynop, res);
  if (!ok) {
    res->set_error(QObject::tr("Ошибка выполнения запроса").toStdString());
  }

  return ok;
}

bool climat::ClimatService::getAvailable(const QString& collection,
           meteo::sprinf::MeteostationType cltype,
           meteo::climat::DataSeriesDate* res)
{
  if (nullptr == res) {
    return false;
  }

  std::unique_ptr<Dbi> db(meteo::global::dbClimat());
  if ( nullptr == db.get() ) {
    res->set_error(QObject::tr("Не удается выполнить запрос в базу климатических данных").toStdString());
    error_log << QObject::tr("Не удается выполнить запрос в базу климатических данных");
    return false;
  }

  std::unique_ptr<DbiQuery> query = db->queryptrByName("cl_get_available");
  if ( !query ) {
    res->set_error(QObject::tr("Не найден запрос 'cl_get_available'").toStdString());
    error_log << QObject::tr("Не найден запрос 'cl_get_available'");
    return false;
  }

  query->arg("tablename", collection);

  //var(query->query());
  
  QString error;
  if ( !query->execInit(&error) ) {
    res->set_error(QObject::tr("Ошибка выполнения запроса").toStdString());
    error_log << QObject::tr("Ошибка выполнения запроса '%1'").arg(query->query());
    return false;
  }

  while (query->next()) {
    const DbiEntry& doc = query->entry();
    res->add_station(doc.valueDocument("_id").valueString("station").toStdString());
    res->add_type(doc.valueDocument("_id").valueInt32("station_type"));
    res->add_date_start(doc.valueDt("first").date().toString(Qt::ISODate).toStdString());
    res->add_date_end(doc.valueDt("last").date().toString(Qt::ISODate).toStdString());
    res->add_cltype(cltype);

  }

  return true;
}

void climat::ClimatService::GetAvailableStations( google::protobuf::RpcController* c,
              const meteo::climat::Dummy* req,
              meteo::climat::ClimatSaveProp* res,
              google::protobuf::Closure* d )
{
  emit signalRequestReceive();
  Q_UNUSED(c);
  Q_UNUSED(req);
  QCoreApplication::processEvents();

  if (!getAvailableStations(res) && !res->has_error()) {
    res->set_error("Запрос задан некорректно или ошибка выполнения запроса");
  }
  d->Run();
}

bool climat::ClimatService::getAvailableStations(meteo::climat::ClimatSaveProp* res)
{
  if (nullptr == res) return false;

  std::unique_ptr<Dbi> db(meteo::global::dbClimat());
  if ( nullptr == db.get() ) {
    res->set_error(QObject::tr("Не удается выполнить запрос в базу климатических данных").toStdString());
    error_log << QObject::tr("Не удается выполнить запрос в базу климатических данных");
    return false;
  }

  std::unique_ptr<DbiQuery> query = db->queryptrByName("cl_get_available_stations");
  if ( !query ) {
    res->set_error(QObject::tr("Не найден запрос 'cl_get_available_stations'").toStdString());
    error_log << QObject::tr("Не найден запрос 'cl_get_available_stations'");
    return false;
  }

  //var(query->query());

  QString error;
  if ( !query->execInit(&error) ) {
    res->set_error(QObject::tr("Ошибка выполнения запроса").toStdString());
    error_log << QObject::tr("Ошибка выполнения запроса '%1'").arg(query->query());
    return false;
  }

  while (query->next()) {
    ClimatStation* clstation = res->add_stations();
    const DbiEntry& doc = query->entry();
    sprinf::Station* station = clstation->mutable_station();
    //station->set_id(doc.valueString("_id").toStdString());
    station->set_station(doc.valueString("station").toStdString());
    station->set_type(doc.valueInt32("type"));
    // station->mutable_name()->set_international(doc.valueDocument("name").valueString("en").toStdString());
    // station->mutable_name()->set_rus(doc.valueDocument("name").valueString("ru").toStdString());
    station->mutable_name()->set_international(doc.valueString("name_en").toStdString());
    station->mutable_name()->set_rus(doc.valueString("name_ru").toStdString());

    station->mutable_country()->set_number(doc.valueInt32("country"));
    GeoPoint pnt = doc.valueGeo("location");
    meteo::GeoPointPb*  pos = station->mutable_position();
    pos->set_lat_deg(pnt.latDeg());
    pos->set_lon_deg(pnt.lonDeg());
    pos->set_lat_radian(pnt.lat());
    pos->set_lon_radian(pnt.lon());
    pos->set_height_meters(pnt.alt());
    clstation->set_utc(doc.valueDouble("utc"));
    clstation->set_cltype(meteo::sprinf::MeteostationType(doc.valueInt32("cltype")));
    clstation->set_dtbeg(doc.valueDt("dtBegin").date().toString(Qt::ISODate).toStdString());
    clstation->set_dtend(doc.valueDt("dtEnd").date().toString(Qt::ISODate).toStdString());
  }

  return true;
}


void climat::ClimatService::GetSurfMonth( google::protobuf::RpcController* c,
            const meteo::climat::DataRequest* req,
            meteo::climat::SurfMonthReply* res,
            google::protobuf::Closure* d )
{
  emit signalRequestReceive();
  Q_UNUSED(c);
  QCoreApplication::processEvents();
  if(!surf_.getMonthValue(req, res) && !res->has_error()) {
    res->set_error("Запрос задан некорректно или ошибка выполнения запроса");
  }
  d->Run();
}

void climat::ClimatService::GetSurfMonthAvg( google::protobuf::RpcController* c,
               const meteo::climat::DataRequest* req,
               meteo::climat::SurfMonthAvgReply* res,
               google::protobuf::Closure* d )
{
  emit signalRequestReceive();
  Q_UNUSED(c);
  if (!surf_.getAvgMonthValue(req, res) && !res->has_error()) {
    res->set_error("Запрос задан некорректно или ошибка выполнения запроса");
  }
  d->Run();
}

void climat::ClimatService::GetSurfYear( google::protobuf::RpcController* c,
           const meteo::climat::DataRequest* req,
           meteo::climat::SurfYearReply* res,
           google::protobuf::Closure* d )
{
  emit signalRequestReceive();
  QCoreApplication::processEvents();
  Q_UNUSED(c);
  if (!surf_.getYearValue(req, res) && !res->has_error()) {
    res->set_error("Запрос задан некорректно или ошибка выполнения запроса");
  }
  d->Run();
}

void  climat::ClimatService::GetSurfDay( google::protobuf::RpcController* c,
           const meteo::climat::DataRequest* req,
           meteo::climat::SurfDayReply* res,
           google::protobuf::Closure* d )
{
  emit signalRequestReceive();
  QCoreApplication::processEvents();
  Q_UNUSED(c);
  if (!surf_.getDayValue(req, res) && !res->has_error()) {
    res->set_error("Запрос задан некорректно или ошибка выполнения запроса");
  }
  d->Run();
}

void climat::ClimatService::GetSurfDecade( google::protobuf::RpcController* c,
             const meteo::climat::DataRequest* req,
             meteo::climat::SurfDecadeReply* res,
             google::protobuf::Closure* d )
{
  emit signalRequestReceive();
  QCoreApplication::processEvents();
  Q_UNUSED(c);
  if (! surf_.getDecadeValue(req, res) && !res->has_error()) {
    res->set_error("Запрос задан некорректно или ошибка выполнения запроса");
  }
  d->Run();
}

void climat::ClimatService::GetSurfAllList( google::protobuf::RpcController* c,
              const meteo::climat::DataRequest* req,
              meteo::climat::SurfAllListReply* res,
              google::protobuf::Closure* d )
{
  emit signalRequestReceive();
  QCoreApplication::processEvents();
  Q_UNUSED(c);
  if (!surf_.getAllValueList(req, res) && !res->has_error()) {
    res->set_error("Запрос задан некорректно или ошибка выполнения запроса");
  }
  d->Run();
}

void climat::ClimatService::GetSurfDate( google::protobuf::RpcController* c,
           const meteo::climat::DataRequest* req,
           meteo::climat::SurfDateReply* res,
           google::protobuf::Closure* d)
{
  emit signalRequestReceive();
  QCoreApplication::processEvents();
  Q_UNUSED(c);
  if (!surf_.getDateValue(req, res) && !res->has_error()) {
    res->set_error("Запрос задан некорректно или ошибка выполнения запроса");
  }
  d->Run();
}

void climat::ClimatService::GetAeroMonthList( google::protobuf::RpcController* c,
                const meteo::climat::DataRequest* req,
                meteo::climat::AeroLayerListReply* res,
                google::protobuf::Closure* d )
{
  emit signalRequestReceive();
  Q_UNUSED(c);
  QCoreApplication::processEvents();
  if (!aero_.getMonthValue(req, res) && !res->has_error()) {
    res->set_error("Запрос задан некорректно или ошибка выполнения запроса");
  }
  d->Run();
}
