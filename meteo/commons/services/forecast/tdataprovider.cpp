#include "tdataprovider.h"

#include <cross-commons/debug/tlog.h>

#include <commons/meteo_data/meteo_data.h>
#include <commons/textproto/pbtools.h>

#include <meteo/commons/astrodata/astrodata.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/global/log.h>
#include <meteo/commons/proto/climat.pb.h>
#include <meteo/commons/proto/forecast.pb.h>
#include <meteo/commons/proto/surface_service.pb.h>
#include <meteo/commons/rpc/rpc.h>
#include <meteo/commons/rpc/channel.h>
#include <meteo/commons/services/sprinf/sprinfservice.h>
#include <meteo/commons/zond/turoven.h>
#include "tdataprovider.h"
#include <qsettings.h>

#define MAX_SRF_DESCRIPTOR 100
#include <qdebug.h>

TDataProvider::TDataProvider(const QDateTime& adt, bool is_service )
  : date_time_(adt)
{
  is_service_ = is_service;
  //station = st;
  _d = TMeteoDescriptor::instance();
  loadSettings();
}

TDataProvider::~TDataProvider()
{
}

zond::Zond *TDataProvider::getZond(const StationData& st, int srok) {
  zond::Zond * z = zond_hash_.value(dataKey(st, date_time_.addSecs(srok * 3600), nullptr, 0, 0));
  if (nullptr != z) {
    return z;
  }
  z = getAeroData(st, srok);
  if (nullptr == z) {
    z = getProfileData(st, srok);
  }
  return z;
}

float TDataProvider::getPrognozInterval(const QString& hours, QDateTime *dt_start, QDateTime *dt_end  ) {
  QStringList hourList = hours.split(";");
  // debug_log << hours;
  if (2 != hourList.size()) {
    debug_log << "Ошибка в скрипте (функция hours())";
    return -1;
  }
  bool ok;
  int hour0 = hourList.at(0).toInt(&ok);
  if (!ok) {
    debug_log << "Ошибка в скрипте (функция hours())";
    return -1;
  }
  int hour1 = hourList.at(1).toInt(&ok);
  if (!ok) {
    debug_log << "Ошибка в скрипте (функция hours())";
    return -1;
  }
  *dt_start = getDate().addSecs(hour0 * 3600);
  *dt_end   = getDate().addSecs(hour1 * 3600);
  return hour1;
}

bool TDataProvider::savePrognoz(const StationData& station, const QString& method_name, const QString& method_title,  const QString& hours, int descr,
                                const QString& value, const QString& textvalue,
                                int level, int type_level ) {
  if(true == is_service_){
    return false;
  }

  QDateTime dt_start, dt_end;

  float hour = getPrognozInterval(hours, &dt_start, &dt_end  );
  if ( 0 > hour) {
    return false;
  }

  int model = meteo::global::kModelSurface;
  if (type_level != meteodescr::kSurfaceLevel) {
    model = meteo::global::kModelAero;
  }
  
  meteo::forecast::ForecastPointData req;
  req.set_time_start(dt_start.toString(Qt::ISODate).toStdString());
  req.set_time_end(dt_end.toString(Qt::ISODate).toStdString());

  if (!station.name.isEmpty()) {
    req.mutable_coord()->set_name(station.name.toStdString());
  }
  req.mutable_coord()->set_index(station.index.toStdString());
  req.mutable_coord()->set_fi(station.coord.fi());
  req.mutable_coord()->set_la(station.coord.la());
  req.mutable_coord()->set_height(station.coord.alt());
  req.mutable_coord()->set_type(station.station_type);
  req.mutable_fdesc()->set_date(getSDate().toStdString());
  req.mutable_fdesc()->set_level_type(type_level);
  req.mutable_fdesc()->set_level(level);
  req.mutable_fdesc()->set_center(meteo::global::kCenterWeather);
  req.mutable_fdesc()->set_hour(hour * 3600);
  req.mutable_fdesc()->set_meteodescr(descr);
  req.mutable_fdesc()->set_model(model);
  bool ok = false;
  double  fvalue = value.toDouble(&ok);
  fvalue = QString::number(fvalue, 'f', 2).toDouble(&ok); //FIXME ???
  if (ok) req.set_value(fvalue);
  req.set_text_value(textvalue.toStdString());
  req.set_method_name(method_name.toStdString());
  req.set_method_title(method_title.toStdString());
  meteo::rpc::Channel* forecast_ctrl = meteo::global::serviceChannel(meteo::settings::proto::kForecastData);
  if ( nullptr == forecast_ctrl ) {
    return false;
  }
  auto reply = std::unique_ptr<meteo::field::SimpleDataReply>(forecast_ctrl->remoteCall( &meteo::forecast::ForecastData::SaveForecast, req,  10000));
  if ( nullptr == reply ) {
    debug_log << "Ошибка сохранения";
    return false;
  }
  delete forecast_ctrl;

  /**
* делаем расчет оправдываемости
* надо вынести в крон
*/
  return true;
}


//! Методы для работы с данными
bool TDataProvider::getValue(const FMetaData& ameta, const QString &descr,
                             QMultiMap<int,FMetaData>* meta_list, QMap<int ,float>* mres_list )
{
  FMetaData meta  = ameta;
  // QTime ttt; ttt.start();

  bool res =  getSynValue( &meta, descr, meta_list,mres_list); //пробуем получить данные по результатам наблюдений
  // info_log << QObject::tr("Время обработки запроса getSynValue: %1 мсек").arg(ttt.elapsed());
  if(false == res){
    specialCaseValue( &meta, descr, meta_list,mres_list); //пробуем получить данные по результатам наблюдений
    //   info_log << QObject::tr("Время обработки запроса getSynValue: %1 мсек").arg(ttt.elapsed());
  }
  meta  = ameta;
  // ttt.restart();
  getFiedValue(&meta, descr, meta_list,mres_list);
  // info_log << QObject::tr("Время обработки запроса getFiedValue: %1 мсек").arg(ttt.elapsed());
  if(0 < meta_list->size()) return true;
  else return false;
}

bool TDataProvider::specialCaseValue(FMetaData *meta, const QString& adescr, QMultiMap<int, FMetaData> *meta_list, QMap<int, float> *mres_list){
  QString descr1 = "T";
  QString descr2 = "D";
  FMetaData meta1 = *meta;

  if("Td" == adescr){
    descr1 = "T";
    descr2 = "D";
  } else if("Td" == adescr){
    descr1 = "D";
    descr2 = "T";
  } else return false;

  QMultiMap<int, FMetaData> meta_list1;
  QMap<int, float> mres_list1;
  bool res = getSynValue(&meta1, descr1, &meta_list1, &mres_list1);
  if(false == res) return false;
  // FMetaData meta2;
  QMultiMap<int, FMetaData> meta_list2;
  QMap<int, float> mres_list2;
  res = getSynValue(&meta1, descr2, &meta_list2, &mres_list2);
  if(false == res||mres_list1.size()<1||mres_list2.size()<1) return false;
  meta->srok = meta1.srok;//TODO в MeteoData НЕТ СРОКА!!!
  meta->dt   = meta1.dt;
  meta->data_source_type = meta1.data_source_type;
  meta->station = meta1.station;
  meta_list->insert(meta_list->size(),*meta);
  float v1 = mres_list1.first();
  float v2 = mres_list2.first();
  float v3 = v1 - v2;
  mres_list->insert(mres_list->size(),v3);
  return res;
}


bool TDataProvider::getFiedValue(FMetaData* meta, const QString& aparam,
                                 QMultiMap<int,FMetaData>* meta_list, QMap<int ,float>*  ) {
  int srok = meta->srok * 3600;
  meteo::field::DataRequest request;
  request.add_level(meta->level);
  request.add_type_level(meta->type_level);
  request.set_only_best(set_only_best_);
  request.add_meteo_descr(_d->descriptor(aparam));
  //request.set_interval(3);
  request.set_forecast_start(getDate().addSecs(srok).addSecs(-field_search_interval_*3600).toString(Qt::ISODate).toStdString());
  // ставим зазор в 3 часа
  request.set_forecast_end(getDate().addSecs(srok).addSecs(field_search_interval_*3600).toString(Qt::ISODate).toStdString());
  // debug_log<< "try get FILEDS " << request.DebugString();
  //QTime ttt; ttt.start();

  QList< meteo::field::DataDesc> afields;
  getFieldList(request, &afields);
  //info_log << QObject::tr("Время обработки запроса getFieldList: %1 мсек").arg(ttt.elapsed());
  //ttt.restart();

  if (afields.size() == 0) {

    QString str = QString("Нет данных по %1 (%2 на уровне %3 за %4)")
                  .arg(meta->station.index)
                  .arg(aparam)
                  .arg(meta->level)
                  .arg(getDate().addSecs(srok).toString(Qt::ISODate));

    warning_log << str;
    return false;
  }
  bool res = false;
  for(int i = 0; i < afields.size(); ++i){
    const meteo::field::DataDesc &f = afields.at(i);
    std::string fnum = f.id();
    meta->station.index = QString();//QString::number(f.hour()/3600.)+QObject::tr(" ч");//внезапно сохраним срок здесь
    meta->station.name =  QString::fromStdString(f.center_name());
    meta->dt =  QDateTime::fromString(pbtools::toQString(f.date()), "yyyy-MM-ddThh:mm:ss");
    meta->data_source_type = field_src;
    meta->srok = f.hour()/3600;
    meta->field_id = QString::fromStdString(fnum);
    meta_list->insert(meta_list->size(),*meta);
  }

  return res;
}


bool TDataProvider::getSynValue(FMetaData*  meta, const QString& descr,
                                QMultiMap<int,FMetaData>* meta_list, QMap<int ,float>* mres_list )
{
  bool ret_val = false;
  meteo::surf::DataRequest src_request;
  QDateTime cdt = date_time_.addSecs(meta->srok*3600);
  QString dddstart = cdt.addSecs(-src_search_interval_*3600-1).toString(Qt::ISODate);
  QString dddend = cdt.addSecs(src_search_interval_*3600+1).toString(Qt::ISODate);

  src_request.set_date_start(dddstart.toStdString());
  src_request.set_date_end(dddend.toStdString());
  src_request.clear_station();
  meteo::surf::Point* p = src_request.mutable_region()->add_point();
  if (meta->station.coord.isValid()) {
    p->set_fi(meta->station.coord.fi());
    p->set_la(meta->station.coord.la());
    p->set_height(meta->station.coord.alt());
  }
  src_request.set_level_p(meta->level);
  src_request.set_type_level(meta->type_level);
  src_request.clear_meteo_descr();
  src_request.add_meteo_descrname(descr.toStdString());
  src_request.set_query_type(meteo::surf::kNearPoCoord);
  src_request.set_rast(max_dist_to_near_station_);
  src_request.set_as_proto(true);
  QString skey = dataKey(meta->station, cdt, descr, meta->level, meta->type_level);

  if (true == value_hash_.contains(skey)) {
    float value = value_hash_.value(skey).value;
    meta->data_source_type = cur_station_src;
    meta->station = value_hash_.value(skey).station;
    meta_list->insert(meta_list->size(),*meta);
    mres_list->insert(mres_list->size(),value);
    return true;
  }
  meteo::rpc::Channel* src_ctrl = meteo::global::serviceChannel(meteo::settings::proto::kSrcData);
  if ( nullptr == src_ctrl ) {
    return false;
  }
  meteo::surf::DataReply* resp = src_ctrl->remoteCall(&::meteo::surf::SurfaceService::GetMeteoData, src_request, 12000);
  delete src_ctrl;
  if ( nullptr == resp ) {
    debug_log << QObject::tr("При попытке получить приземные данные ответ от сервиса данных не получен");
    return false;
  }
  int mdsize =   resp->meteodata_proto_size();
  for ( int i =0; i < mdsize; ++i ) {
    const ::meteo::surf::MeteoDataProto& data = resp->meteodata_proto(i);
    if(data.has_station_info()){
      const ::meteo::surf::StationInfo& st = data.station_info();
      meta->station.index = QString::fromStdString(st.cccc());
      meta->station.name = QString::fromStdString(st.name_rus());
      meteo::GeoPoint point;
      point.setLat(data.gp().la());
      point.setLon(data.gp().lo());
      if(point.isValid()){
        meta->distance_to_point = point.calcDistance(meta->station.coord);
        meta->station.coord=point;
      }
    }
    int param_size = data.param_size();
    for(int j =0; j < param_size;++j)
    {
      const ::meteo::surf::MeteoParamProto& param = data.param(j);
      QDateTime cur_dt = pbtools::toQDateTime(data.dt());;
      skey = dataKey(meta->station, cur_dt, descr, meta->level, meta->type_level);
      if(data.has_hour()){ meta->srok = data.hour();}
      else {meta->srok = 0;}
      meta->dt = cur_dt;
      StationValue sv;
      sv.value = param.value();
      sv.qual = param.quality();
      sv.station = meta->station;
      sv.descr = descr;
      value_hash_.insert(skey, sv);
      if(descr == QString::fromStdString(param.descrname())){
        meta_list->insert(meta_list->size(),*meta);
        mres_list->insert(mres_list->size(),sv.value);
        ret_val = true;
      }
    }
  }
  delete resp;
  return ret_val;
}


//****************************************************************
//**Методы для работы с  данными  зондирования                     **
//****************************************************************



zond::Zond *TDataProvider::getAeroData(const StationData& st, int srok)
{
  meteo::surf::DataRequest src_request;
  // debug_log<<"getAeroData";
  zond::Zond * z = zond_hash_.value(dataKey(st,date_time_.addSecs(srok*3600),nullptr,0,0));
  if(0 != z){ return z; }

  src_request.add_type(meteo::surf::kAeroType);
  QString dddstart = date_time_.addSecs(-src_search_interval_*3600).toString(Qt::ISODate);
  QString dddend = date_time_.addSecs(src_search_interval_*3600).toString(Qt::ISODate);
  src_request.set_date_start(dddstart.toStdString());
  src_request.set_date_end(dddend.toStdString());
  src_request.clear_station();
  src_request.set_rast(max_dist_to_near_station_);

  meteo::surf::Point* p = src_request.mutable_region()->add_point();
  if (!st.index.isEmpty() && !st.index.isNull()) {
    src_request.add_station(st.index.toStdString());
    p->set_index(st.index.toStdString());
  }


  if (st.coord.isValid()) {
    p->set_fi(st.coord.fi());
    p->set_la(st.coord.la());
    p->set_height(st.coord.alt());
  }

  meteo::rpc::Channel* src_ctrl = meteo::global::serviceChannel(meteo::settings::proto::kSrcData);
  if ( nullptr == src_ctrl ) {
    return nullptr;
  }
  meteo::surf::TZondValueReply* reply = src_ctrl->remoteCall(&meteo::surf::SurfaceService::GetTZondOnStation, src_request, 120000);
  if ( nullptr == reply ) {
    debug_log << QObject::tr("При попытке выполнить анализ аэрологических данных ответ от сервиса данных не получен");
    return nullptr;
  }
  delete src_ctrl;

  // zond::Zond * azond = new zond::Zond();

  // debug_log<<reply->DebugString();
  //
  // хз зачем это, но мне нужен работающий зонд, а по-другому не работает
  //
  zond::PlaceData * place = new zond::PlaceData();
  place->setIndex( st.index.toInt() );
  place->setCoord( st.coord );

  //  debug_log<<reply->ShortDebugString()<<"getAeroData4";
  if (reply->IsInitialized() && 0 < reply->data_size()) {
    for ( int i = 0, sz = reply->data_size(); i < sz; ++i ) {
      QByteArray buf( reply->data(i).meteodata().data(), reply->data(i).meteodata().size() );

      place->setZond(buf);
      place->zond().preobr();
      place->setDt(place->zond().dateTime());
      place->setIndex(place->zond().stIndex());
      place->setCoord(place->zond().coord());
      if (place->zond().haveData()) {
        zond_hash_.insert(dataKey(st, place->zond().dateTime(), nullptr, 0, 0), &place->zond() );
      }
    }
  }
  delete reply; reply = nullptr;
  if (false == place->zond().haveData()) {
    return nullptr;
  }
  return &place->zond();
}



void TDataProvider::setDate(const QDateTime &adt)
{
  date_time_ = adt;
}

using namespace TGis;

int TDataProvider::getFieldList(const meteo::field::DataRequest &request, QList< meteo::field::DataDesc> *afields)
{
  meteo::rpc::Channel* field_ctrl = meteo::global::serviceChannel(meteo::settings::proto::kField);
  if ( nullptr == field_ctrl ) {
    return 0;
  }
  meteo::field::DataDescResponse * reply =
      field_ctrl->remoteCall( &meteo::field::FieldService::GetAvailableData, request,  12000);
  delete field_ctrl;
  if ( nullptr == reply ) {
    debug_log << QObject::tr("При попытке получить список доступных полей, ответ от сервиса данных не получен");
    return 0;
  }
  for (int fc =0; reply->descr_size() >fc  ; ++fc ) {
  //for (int fc =reply->descr_size()-1; fc >-1 ; --fc ) {
    meteo::field::DataDesc * fdescr = reply->mutable_descr(fc);
    if (!fdescr->has_id() || fdescr->priority()< 0) continue;
    //afields->insert(fdescr->priority(), *fdescr);
    afields->append( *fdescr);
  }
  delete reply; reply = nullptr;
  return afields->count();
}

float TDataProvider::getAdvectTrackLength()
{
  return advectTrackLength_;
}


//bool TDataProvider::getAdvectPoint(const meteo::field::AdvectDataRequest &request, meteo::GeoPoint* agp)
bool TDataProvider::getAdvectPoint( int level, int center, int time, meteo::GeoPoint* agp)
{
  meteo::field::AdvectObjectRequest request;
  // request.set_date(getDate().addSecs(time).toString(Qt::ISODate).toStdString());
  request.set_date(getDate().toString(Qt::ISODate).toStdString());
  request.set_adv_time(time);
  request.set_level(level);
  request.set_center(center);
  request.set_need_prom_points(false);
  request.set_need_length(true);
  request.set_need_field_descr(false);
  request.set_need_prom_points(false);
  request.set_time_step(1 * 3600);
  request.set_koef(0.8);

  meteo::field::Skelet* skelet = request.add_skelet();
  skelet->set_uuid("");
  meteo::field::Skelet_Dot* dot = skelet->add_dots();
  dot->set_number(0);
  meteo::surf::Point* p = dot->add_coord();
  p->set_fi(agp->fi());
  p->set_la(agp->la());
  p->set_date_time(getDate().toString(Qt::ISODate).toStdString());
  *agp = meteo::GeoPoint();
  meteo::rpc::Channel* field_ctrl = meteo::global::serviceChannel(meteo::settings::proto::kField);
  if ( nullptr == field_ctrl){
    return false;
  }

  meteo::field::AdvectObjectReply * reply =
      field_ctrl->remoteCall( &meteo::field::FieldService::GetAdvectObject, request,  30000);
  delete field_ctrl;
  if ( nullptr == reply ||
       1 > reply->skelets_size() ||
       1 > reply->skelets(0).dots_size()) {
    debug_log << QObject::tr("ответ от сервиса данных не получен");
    return false;
  }
  int sizeOfDots = reply->skelets(0).dots_size();
  if( sizeOfDots < 1 ) return false;
  const meteo::field::Skelet_Dot &sdot = reply->skelets(0).dots(0);
  int point_kol = sdot.coord_size();

  if (0 < point_kol ) {
    agp->setFi(sdot.coord(point_kol - 1).fi());
    agp->setLa(sdot.coord(point_kol - 1).la());
    agp->setAlt(sdot.coord(point_kol - 1).height());
    advectStation_.coord = *agp;
    if(sdot.has_length()) advectTrackLength_ = sdot.length();
    delete reply; reply = nullptr;
    return true;
  }
  delete reply; reply = nullptr;

  return false;
}

bool TDataProvider::getFiedValuePoIndex(const meteo::GeoPoint &agp, std::string fnum, fieldValue* aval)
{

  QString skey = QString("%1%2").arg(QString::fromStdString(fnum)).arg(agp.toString("%1%2"));

  if (field_value_hash_.contains(skey)) {
    *aval = field_value_hash_.value(skey);
    return true;
  }

  bool ret_val = false;
  ::meteo::field::DataRequest request;
  request.set_field_id(fnum);
  request.set_is_df(true);
  ::meteo::surf::Point* pnt = request.add_coords();
  pnt->set_fi(agp.fi());
  pnt->set_la(agp.la());
  meteo::rpc::Channel* field_ctrl = meteo::global::serviceChannel(meteo::settings::proto::kField);
  if ( nullptr == field_ctrl){
    return false;
  }
  meteo::field::ValueDataReply * reply =
      field_ctrl->remoteCall( &meteo::field::FieldService::GetForecastValues, request,  12000);
  delete field_ctrl;
  if ( nullptr == reply ) {
    debug_log << QObject::tr("ответ от сервиса данных не получен");
    return false;
  }
  if (0 < reply->data_size()) {
    aval->value = reply->data(0).value();
    aval->dx = reply->data(0).dd_x();
    aval->dy = reply->data(0).dd_y();
    aval->dxx = reply->data(0).dd2_xx();
    aval->dyy = reply->data(0).dd2_yy();
    aval->dxy = reply->data(0).dd2_xy();
    aval->rk = reply->data(0).rad_kriv();
    field_value_hash_.insert(skey, *aval);
    ret_val = true;
  }
  delete reply; reply = nullptr;

  return ret_val;
}

zond::Zond *TDataProvider::getProfileData(const StationData& st, int srok)
{
  meteo::field::DataRequest req;
  meteo::surf::Point* agp = req.add_coords();
  meteo::GeoPoint p = st.coord;
  agp->set_la(p.la());
  agp->set_fi(p.fi());
  agp->set_height(p.alt());
  req.set_only_best(true);
  req.set_only_last(true);
  req.set_forecast_start(date_time_.addSecs(srok * 3600).toString(Qt::ISODate).toStdString());
  req.set_forecast_end(date_time_.addSecs(srok * 3600).toString(Qt::ISODate).toStdString());
  zond::Zond * z = zond_hash_.value(dataKey(st, date_time_.addSecs(srok * 3600), nullptr, 0, 0));
  if (nullptr != z ) {
    return z;
  }
  meteo::rpc::Channel* field_ctrl = meteo::global::serviceChannel(meteo::settings::proto::kField);
  if ( nullptr == field_ctrl){
    return nullptr;
  }
  meteo::surf::TZondValueReply* resp = field_ctrl->remoteCall(& meteo::field::FieldService::GetADZond, req, 90000);
  delete field_ctrl;
  if ( nullptr == resp ) {
    debug_log << QObject::tr("Ошибка при получении данных от сервиса fielddata.");
    return nullptr;
  }

  if ( nullptr == resp  || 1 > resp->data_size() ) {//FIXME
    delete resp;
    return nullptr;
  }
  zond::Zond * azond = new zond::Zond();

  QList<QDateTime> dtlist;
  for ( int i = 0; i < resp->data_size(); ++i ) {
    QByteArray buf(resp->data(i).meteodata().data(), resp->data(i).meteodata().size() );
    azond->clear();
    (*azond) << buf;
    azond->preobr();

    // azond->print();

    if ( dtlist.contains(azond->dateTime()) ) { continue; }
    dtlist.append(azond->dateTime());
    zond_hash_.insert(dataKey(st, date_time_.addSecs(srok * 3600), nullptr, 0, 0), azond);
  }
  delete resp;
  return azond;
}

QString TDataProvider::dataKey(const StationData& st, const QDateTime &dt, const QString& descr, int level, int type_level) {
  return  QString("%1%2%3%4%5%6").arg(st.index).arg(dt.toTime_t()).
      arg(st.coord.toString("%1%2"))/*.arg(descr)*/.arg(level).arg(type_level).arg(descr);

}

void TDataProvider::calcTableDailyRateTemp(const::meteo::climat::SurfMonthAllValues &atemp, const::meteo::climat::SurfMonthAllValues &cloud, QMap<int, QMap<int, float> > *table, QMap<int, QMap<int, int> > *count)
{

  QMap<QDate, int> maxcloud;
  for (int idx = 0; idx < cloud.avglist_size(); idx++) {
    const ::meteo::climat::SurfAllValuesList& v = cloud.avglist(idx);
    QDate date(v.year(), cloud.month(), v.day());
    if (!maxcloud.contains(date) || maxcloud.value(date) < v.avg())
    {
      maxcloud.insert(date, v.avg());
    }
  }

  QMap<QDateTime, float> temp;
  for (int idx = 0; idx < atemp.avglist_size(); idx++) {
    const ::meteo::climat::SurfAllValuesList& v = atemp.avglist(idx);
    temp.insert(QDateTime(QDate(v.year(), atemp.month(), v.day()), QTime(v.hour(), 0)), v.avg());
  }

  QMap<QDateTime, float>::iterator it = temp.begin();
  while (it != temp.end()) {
    QMap<int, float>& h = (*table)[it.key().time().hour()];
    int categ = -1;

    if (maxcloud.contains(it.key().date()) && temp.contains(QDateTime(it.key().date(), QTime(21, 0)))) {
      if (maxcloud.value(it.key().date()) <= 3) {
        categ = 0;
      } else if (maxcloud.value(it.key().date()) >= 4 && maxcloud.value(it.key().date()) <= 7) {
        categ = 1;
      } else if (maxcloud.value(it.key().date()) >= 8) {
        categ = 2;
      }
      if (!h.contains(categ)) {
        h.insert(categ, 0);
        (*count)[it.key().time().hour()].insert(categ, 0);
      }
      h[categ] += it.value() - temp.value(QDateTime(it.key().date(), QTime(21, 0)));
      (*count)[it.key().time().hour()][categ] += 1;
    }
    ++it;
  }
}


double TDataProvider::getSutHod(const StationData& st, int month, int hour)
{
  meteo::climat::DataRequest req;
  req.set_station(st.index.toStdString());
  req.set_station_type(meteo::sprinf::kStationSynop);
  req.add_descrname("T");
  req.add_descrname("N");

  meteo::rpc::Channel* channel = meteo::global::serviceChannel(meteo::settings::proto::kClimat);
  if(nullptr == channel){
    return -199991.;
  }
  meteo::climat::SurfAllListReply* resp;
  resp = channel->remoteCall(&::meteo::climat::Service::GetSurfAllList, req, 120000);
  delete channel;
  if ( nullptr == resp || 1 > resp->all_size() ) {
    return -9999;
  }

  int idx = -1;
  for (int i = 0; i < resp->all(0).val_size(); i++) {
    if (resp->all(0).val(i).month() == month) {
      idx = i;
      break;
    }
  }

  if (idx == -1)
  {
    return -9999.;
  }
  const ::meteo::climat::SurfMonthAllValues& temp = resp->all(0).val(idx); //один месяц температуры

  idx = -1;
  for (int i = 0; i < resp->all(1).val_size(); i++)
  {
    if (resp->all(1).val(i).month() == month)
    {
      idx = i;
      break;
    }
  }

  if (idx == -1)
  {
    return -9999.;

  }
  const ::meteo::climat::SurfMonthAllValues& cloud = resp->all(1).val(idx); //один месяц облачности
  delete resp;
  QMap<int, QMap<int, float> > table; //срок, (категория, сумма отклонений)
  QMap<int, QMap<int, int> > count; // количество случаев

  calcTableDailyRateTemp(temp, cloud, &table, &count);

  QMap<int, double> data; //срок, отклонение
  QList<int> allHours = count.keys();
  int cloudType = 0;

  for (int row = 0; row < allHours.count(); ++row) {
    if (table.contains(allHours.at(row)) &&
        count.value(allHours.at(row)).contains(cloudType) &&
        count.value(allHours.at(row)).value(cloudType) != 0) {
      data.insert(allHours.at(row),
                  table.value(allHours.at(row)).value(cloudType) /
                  count.value(allHours.at(row)).value(cloudType));
    }
  }

  QMapIterator<int, double> i(data);
  while (i.hasNext()) {
    i.next();
    if ( i.key() > hour)
    {
      return 1111.;
      double prev = i.previous().value();
      double current = i.value();
      return (prev + current) *0.5;

    }
  }
  return -9999;
}

float TDataProvider::getSunRiseSet(const StationData& st, int type)
{
  QDateTime date = getDate();
  QDateTime date2 = getDate();
  astro::AstroDataCalendar cal;
  astro::ObserverPoint& p = cal.observerPoint();
  p.setLatitude(st.coord.latDeg());
  p.setLongitude(st.coord.lonDeg());
  p.setHeight(st.coord.alt());

  date.setTime(QTime(0, 0));
  date2 = date;
  date2 = date2.addDays(1);

  p.setFirstDateTime(date);
  p.setSecondDateTime(date2);

  cal.clear();
  cal.setTimeSpec(Qt::UTC);
  cal.makeUp();
  float riseHour = cal.day(0).getSunRise().hour();
  riseHour += cal.day(0).getSunRise().minute()/60.;
  float setHour = cal.day(0).getSunSet().hour();
  setHour += cal.day(0).getSunSet().minute()/60.;
  if (type == 1) {  return riseHour; }
  if (type == 2) {  return setHour; }
  return -9999.;

}
/**
 * Возвращаем регулярку для каждого типа параметров
 * @param  param [description]
 * @return       [description]
 */
QRegExp TDataProvider::getRegexpByParam(const QString &param){
  QString rx="^\\-?\\d{1,3}(\\.?\\d{0,2})?(e\\-?\\d*)?$";
  rx = regexp_map_.value(param,rx);
  return QRegExp(rx);
}

void TDataProvider::loadSettings(){
  QSettings s(MnCommon::etcPath() + "inputvalidators.ini", QSettings::IniFormat);
  s.beginGroup("round");
  QStringList keys = s.childKeys();
  for(int i = 0; i < keys.size();++i){
    const QString& k = keys.at(i);
    round_map_.insert(k,s.value(k,1).toInt());
  }
  s.endGroup();
  s.beginGroup("regexp");
  keys = s.childKeys();
  QString rx="^\\-?\\d{1,3}(\\.?\\d{0,2})?(e\\-?\\d*)?$";
  for(int i = 0; i < keys.size();++i){
    const QString& k = keys.at(i);
    regexp_map_.insert(k,s.value(k,rx).toString());
  }
  s.endGroup();
  s.beginGroup("settings");
  max_dist_to_near_station_ = s.value("max_dist_to_near_station",20000).toInt();
  set_only_best_ = s.value("set_only_best",true).toBool();
  field_search_interval_ = s.value("field_search_interval",0).toInt();
  src_search_interval_= s.value("src_search_interval",1).toInt();
  max_hour_= s.value("max_hour",48).toInt();
  s.endGroup();
}


/**
 * проверяем насколько надо округлить, чтобы было все ок
 * @param  param [description]
 * @return       [description]
 */
int TDataProvider::checkRoundDigits(const QString &param){
  return round_map_.value(param,1);
}
