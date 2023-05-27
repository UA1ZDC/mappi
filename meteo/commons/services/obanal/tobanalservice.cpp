#include "tobanalservice.h"

#include <sql/psql/psqlquery.h>

#include <commons/obanal/func_obanal.h>
#include <commons/obanal/tfieldparams.h>
#include <commons/meteo_data/meteo_data.h>
#include <commons/meteo_data/tmeteodescr.h>

#include <meteo/commons/proto/tgribformat.pb.h>
#include <meteo/commons/primarydb/gribmongo.h>

#include "tobanal.h"

namespace meteo {

//constexpr NetType kNetType = STEP_0500x0500;
constexpr NetType kNetType = STEP_100x100;

TObanalService::TObanalService()
  : obanal_(new TObanal()) { }

TObanalService::~TObanalService() { }

bool TObanalService::haveValueRespGrib(meteo::surf::GribDataReply* resp)
{
  if(nullptr == obanal_) { return false; }

  if(false == resp->has_info()         ||
     0 == resp->info().param_size() ||
     false == resp->info().has_level() ||
     false == resp->info().has_hour()  ||
     0 == resp->data_size())
  { return false; }
//  if(resp->info().param() != 12101) { return true; }

  QDateTime dt = PsqlQuery::datetimeFromString(resp->info().date());
  if(false == dt.isValid()) {
    debug_log << "Некорректный формат даты:"<< resp->info().date();
    return false;
  }
  QVector<RegionParam> corners;
  meteo::GeoData gdata;
  GribMongoDb grib_decoder;
  
  //TODO в resp теперь заполнен netType, если надо. только там ::grib::NetType
  for(const meteo::surf::GribData& data: resp->data()) {
    if (true == grib_decoder.fillFromGrib(&gdata, /*&agrib_net_type,*/ data, true)) {
    addCorners(corners, data.grid_def());
    }
  }
  if(corners.empty()) {
    debug_log << "Нет информации о сетке";
    return false;
  }
  meteo::field::DataDesc fdescr;
  fdescr.set_center     (resp->info().center());
  fdescr.set_count_point(gdata.count());
  fdescr.set_date       (resp->info().date());
  //TODO заполнять dt1, dt2 из GribInfo
  QDateTime dt1 = PsqlQuery::datetimeFromString(resp->info().hour().dt1());
  if(false == dt.isValid() || false == dt1.isValid()) {
    debug_log << QObject::tr("Не удалось получить срок прогноза из GribInfo. dt = %1 dt1 = %2, dt2 = %3. Срок прогноза = 0")
      .arg(QString::fromStdString(resp->info().date()))
      .arg(QString::fromStdString(resp->info().hour().dt1()))
      .arg(QString::fromStdString(resp->info().hour().dt2()));
  }
  QDateTime dt2 = PsqlQuery::datetimeFromString(resp->info().hour().dt2());
  debug_log_if(false == dt1.isValid() || false == dt2.isValid()) << resp->info().Utf8DebugString();
  fdescr.set_hour      (dt.secsTo(dt1));
  fdescr.set_level     (resp->info().level());
  fdescr.set_level_type(resp->info().level_type());
  fdescr.set_meteodescr(resp->info().param(0));
  fdescr.set_model     (resp->info().model());
  //fdescr.set_net_type  (kNetType);
  fdescr.set_net_type  (resp->info().net_type());
  fdescr.set_dt1       (dt1.toString(Qt::ISODate).toStdString());
  fdescr.set_dt2       (dt2.toString(Qt::ISODate).toStdString());
  fdescr.set_time_range(resp->info().hour().time_range());
  return obanal_->calculateAndSaveGrib(&gdata, corners, &fdescr);
}



bool TObanalService::haveValueResp(GeoData* all_data,  const meteo::map::proto::WeatherLayer &info)
{
  if(nullptr == obanal_) {
    debug_log <<" pointer to obanal_ is NULL";
    return false;
  }
  if(0 == info.meteo_descr_size()) { return false; }

  int descr = info.meteo_descr(0);

  meteo::field::DataDesc fdescr;
  fdescr.set_center     (global::kCenterWeather );
  fdescr.set_count_point(all_data->count());
  fdescr.set_date       (info.datetime());
  fdescr.set_hour       (0);
  fdescr.set_level      (info.level());
  fdescr.set_meteodescr (descr);
  fdescr.set_level_type (info.type_level());
  fdescr.set_model      (global::kModelSurface );
  fdescr.set_net_type   (kNetType);
  fdescr.set_dt1        (fdescr.date());
  fdescr.set_dt2        (fdescr.date());
  fdescr.set_time_range (255);

  return obanal_->calculateAndSaveField(all_data,  &fdescr);
}

bool TObanalService::haveValueResp(const meteo::surf::DataRequest& req, meteo::surf::DataReply* resp)
{
  if(nullptr == obanal_) {
    debug_log <<" pointer to obanal_ is NULL";
    return false;
  }
  GeoData all_data;
  if(0 == req.meteo_descr_size()) { return false; }

  int descr = req.meteo_descr(0);
  for(const std::string& str: resp->meteodata()) {
    QByteArray barr(str.data(), str.size());
    TMeteoData md;
    md << barr;

    GeoPoint gpt;
    if(false == TMeteoDescriptor::instance()->getCoord(md, &gpt)) { continue; }

    const TMeteoParam& param = md.getParam(descr);
    const TMeteoParam& id = md.getParam(TMeteoDescriptor::instance()->descriptor("text"));
    if(control::SPECIAL_VALUE <= param.quality() ||
       false == checkParam(descr, param.value(), req.level_p(), req.type_level()) )
    { continue; }
    float v = param.value();
//    all_data.append(MData(gpt.fi(), gpt.la(), v, TRUE_DATA, id.code()));
    all_data.append(MData(gpt, v, TRUE_DATA, id.code()));

  }

  meteo::field::DataDesc fdescr;
  fdescr.set_center     (global::kCenterWeather );
  fdescr.set_count_point(all_data.count());
  fdescr.set_date       (req.date_start());
  fdescr.set_hour       (0);
  fdescr.set_level      (req.level_p());
  fdescr.set_meteodescr (descr);
  fdescr.set_level_type (req.type_level());
  fdescr.set_model      (global::kModelSurface );
  fdescr.set_net_type   (kNetType);
  fdescr.set_dt1        (fdescr.date());
  fdescr.set_dt2        (fdescr.date());
  fdescr.set_time_range (255);
  return obanal_->calculateAndSaveField(&all_data,  &fdescr);
}

bool TObanalService::haveValueRespWind(const meteo::surf::DataRequest& req,  meteo::surf::DataReply* resp)
{
  if(nullptr == obanal_) {
    debug_log <<" pointer to obanal_ is NULL";
    return false;
  }

  GeoData all_data_u;
  GeoData all_data_v;
  // debug_log << QString::fromStdString(resp->date());
  for(const std::string& str: resp->meteodata()) {
    QByteArray barr(str.data(), str.size());
    TMeteoData md;
    md << barr;
    GeoPoint gpt;
    if(!TMeteoDescriptor::instance()->getCoord(md, &gpt)) { continue; }
    const TMeteoParam& dd = md.getParam(11001);
    const TMeteoParam& ff = md.getParam(11002);
    const TMeteoParam& id = md.getParam(TMeteoDescriptor::instance()->descriptor("text"));
    if(control::SPECIAL_VALUE > dd.quality() && control::SPECIAL_VALUE > ff.quality()) {
        float u, v;
        MnMath::convertDFtoUV(dd.value(), ff.value(), &u, &v);
        //all_data_u.append(MData(gpt.fi(), gpt.la(), u, TRUE_DATA, id.code()));
        all_data_u.append(MData(gpt, u, TRUE_DATA, id.code()));
        //all_data_v.append(MData(gpt.fi(), gpt.la(), v, TRUE_DATA, id.code()));
        all_data_v.append(MData(gpt, v, TRUE_DATA, id.code()));
    }
  }

  meteo::field::DataDesc fdescr;
  fdescr.set_center     (global::kCenterWeather);
  fdescr.set_count_point(all_data_u.count());
  fdescr.set_date       (req.date_start());
  fdescr.set_hour       (0);
  fdescr.set_level      (req.level_p());
  fdescr.set_meteodescr (11003);
  fdescr.set_level_type (req.type_level());
  fdescr.set_model      (global::kModelAero);
  fdescr.set_net_type   (kNetType);
  fdescr.set_dt1        (fdescr.date());
  fdescr.set_dt2        (fdescr.date());
  fdescr.set_time_range (255);

  if(false == obanal_->calculateAndSaveField(&all_data_u,  &fdescr)) { return false; }

  fdescr.set_meteodescr(11004);
  return obanal_->calculateAndSaveField(&all_data_v,  &fdescr);
}



//! добавляем координаты углов сетки
void TObanalService::addCorners(QVector<RegionParam>& corners, const TGrid::LatLon& msg)
{
  double subdivision = 1e-3;
  double subdiv_max = 1e6;
  //1 250 000

  if(msg.lo1() > subdiv_max ||
     msg.lo2() > subdiv_max ||
     msg.la1() > subdiv_max ||
     msg.la2() > subdiv_max )
  { subdivision = 1e-6; }
  float latFirst = msg.la1() * subdivision;
  float lonFirst = msg.lo1() * subdivision;
  float latLast  = msg.la2() * subdivision;
  float lonLast  = msg.lo2() * subdivision;
  bool  lonAdd   = (msg.scanmode() & 0x80) == 0;
  bool  latAdd   = (msg.scanmode() & 0x40) != 0;

  RegionParam region;
  if (lonAdd) {
    region.start.setLaDeg(MnMath::M180To180(lonFirst));
    region.end.setLaDeg  (MnMath::M180To180(lonLast));
  }
  else {
    region.start.setLaDeg(MnMath::M180To180(lonLast));
    region.end.setLaDeg  (MnMath::M180To180(lonFirst));
  }
//      if(latFirst > 89.0)  { latFirst = 89.0; }
//      if(latLast  > 89.0)  { latLast = 89.; }
//      if(latFirst < -89.0) { latFirst = -89.0; }
//      if(latLast  < -89.0) { latLast = -89.0; }

  if (latAdd) {
    region.start.setFiDeg(latFirst);
    region.end.setFiDeg(latLast);
  }
  else {
    region.start.setFiDeg(latLast);
    region.end.setFiDeg(latFirst);
  }

  //debug_log << region.start.toString() << region.end.toString();

  if (region.start.laDeg() > region.end.laDeg())
    // || (region.start.laDeg() < 0. && region.end.laDeg() > 0.))
  {
    RegionParam region1 = region;
    region.end.setLaDeg   (180.0);
    region1.start.setLaDeg(-180.0);
    corners.append(region);
    ////   debug_log << corners.last().start.toString() << corners.last().end.toString();
    corners.append(region1);
  }
  else {
    corners.append(region);
  }
   //debug_log << corners.last().start.toString() << corners.last().end.toString();
}

bool TObanalService::checkParam(int64_t descr, float val, int level, int level_type)
{
  if(10009 == descr) { val *= 0.1f; }

  bool ok = false;

  float minval = ::obanal::singleton::TFieldParams::instance()->minvalue(descr, level, level_type, &ok);
  if(ok && minval > val) { return false; }

  float maxval = ::obanal::singleton::TFieldParams::instance()->maxvalue(descr, level,level_type, &ok);
  if(ok && maxval < val) { return false; }

  return true;
}



} // meteo
