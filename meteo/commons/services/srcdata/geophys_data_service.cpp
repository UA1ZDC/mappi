#include "geophys_data_service.h"

#include <qfile.h>
#include <qdatastream.h>

#include <cross-commons/debug/tlog.h>

#include <commons/meteo_data/meteo_data.h>
#include <commons/meteo_data/tmeteodescr.h>
#include <meteo/commons/global/global.h>
//#include <sql/nspgbase/ns_pgbase.h>

using namespace meteo;
using namespace surf;

GeophysDataService::GeophysDataService(const DataRequest* req, google::protobuf::Closure* done)
  : TBaseDataService(done)
  , data_request_(req)
  , simpledata_request_(nullptr)
  , data_reply_(nullptr)
  , available_reply_(nullptr)
{ }

GeophysDataService::GeophysDataService(const SimpleDataRequest* req, google::protobuf::Closure* done)
  : TBaseDataService(done)
  , data_request_(nullptr)
  , simpledata_request_(req)
  , data_reply_(nullptr)
  , available_reply_(nullptr)
{ }

void GeophysDataService::run()
{
  QTime ttt; ttt.start();
  bool ret_val = false;

  switch(req_type_) {
  case rGetAvailableGeophysMap:
    if(nullptr != data_request_ && nullptr != available_reply_) {
      ret_val= getAvailable(data_request_, available_reply_);
    }
  break;
  case rGetGeophysMapById:
    if(nullptr != simpledata_request_ && nullptr != data_reply_) {
      ret_val= getById(simpledata_request_, data_reply_);
    }
  break;
  default:
  break;
  }
  if(false == ret_val) {
    //TODO
  }
  int cur =  ttt.elapsed();
  if(1000 < cur) {
    warning_log << QObject::tr("Время обработки запроса GetValueOnStation: %1 мсек").arg(cur);
  }
  emit finished(this);
}

bool GeophysDataService::getAvailable(const DataRequest* req, GeophysMapAvailableReply* res)
{
  Q_UNUSED(req);
  Q_UNUSED(res);
  //  if(false == connectToDb()) return false;
  //
  //  if(false == req->has_date_start() || false == req->has_type()) {
  //    return false;
  //  }
  //  QString dts = QString::fromStdString(req->date_start());
  //
  //  QString query = QString("SELECT * from get_available_geophysmap('%1', %2);").arg(dts).arg(req->type());
  //  debug_log << query;
  //
  //  if(false == db_->ExecQuery(query)) {
  //    error_log << "Error read" << db_->ErrorMessage();
  //    return false;
  //  }
  //
  //  //debug_log << "count Records" << db_->RecordCount();
  //
  //  if(0 == db_->RecordCount()) {
  //    res->set_error(QObject::tr("Нет данных").toUtf8().constData());
  //    return true;
  //  }
  //
  //  for (int idx = 0; idx < db_->RecordCount(); idx++) {
  //    GeophysMapInfo* info = res->add_info();
  //    info->set_id(db_->GetValue(idx, 0).toULong());
  //    info->set_dt(QString(db_->GetValue(idx, 1)).toStdString());
  //    info->set_station(QString(db_->GetValue(idx, 2)).toStdString());
  //    info->set_lat1(db_->GetValue(idx, 3).toFloat());
  //    info->set_lat2(db_->GetValue(idx, 4).toFloat());
  //    info->set_dlat(db_->GetValue(idx, 5).toFloat());
  //    info->set_lon1(db_->GetValue(idx, 6).toFloat());
  //    info->set_lon2(db_->GetValue(idx, 7).toFloat());
  //    info->set_dlon(db_->GetValue(idx, 8).toFloat());
  //    info->set_h1(db_->GetValue(idx, 9).toFloat());
  //    info->set_h2(db_->GetValue(idx, 10).toFloat());
  //    info->set_dh(db_->GetValue(idx, 11).toFloat());
  //  }
  //
  //  return true;
  return false;
}

bool GeophysDataService::getById(const SimpleDataRequest* req, DataReply* res)
{
  Q_UNUSED(req);
  Q_UNUSED(res);
  //  if(false == connectToDb()) return false;
  //
  //  if(req->id_size() != 1) {
  //    return false;
  //  }
  //
  //  QString query = QString("SELECT * from get_geophysmap(%1);").arg(QString::fromStdString(req->id(0)));
  //  debug_log << query;
  //
  //  if (!db_->ExecQuery(query)) {
  //    error_log<<"Error read" << db_->ErrorMessage();
  //    return false;
  //  }
  //
  //  if(0 == db_->RecordCount()) {
  //    res->set_error(QObject::tr("Нет данных").toUtf8().constData());
  //    return true;
  //  }
  //  QString fileName = QString(db_->GetValue(0, 0));
  //  var(fileName);
  //
  //  TMeteoData header;
  //  QByteArray data;
  //
  //  QFile file(fileName);
  //  file.open(QIODevice::ReadOnly);
  //  QDataStream ds(&file);
  //  ds >> header;
  //  ds >> data;
  //  file.close();
  //
  //  data = qUncompress(data);
  //
  //  //header.printData();
  //  //var(data);
  //
  //  QByteArray ba;
  //  header >> ba;
  //  res->add_meteodata(ba.data(), ba.size());
  //
  //
  //  return true;
  return false;
}

