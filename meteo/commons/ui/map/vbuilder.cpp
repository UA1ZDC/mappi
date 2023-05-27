#include "vbuilder.h"
#include "vprofiledoc.h"

#include <cross-commons/debug/tlog.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/proto/services.pb.h>
#include <meteo/commons/proto/surface_service.pb.h>


namespace meteo {
namespace map {

VBuilder::VBuilder(VProfileDoc *doc)
{
  doc_ = doc;
}

bool VBuilder::createVProfile(const proto::Map &map)
{
  if ( nullptr == doc_ ) {
    error_log << QObject::tr("Параметр doc == nullptr.");
    return false;
  }
  if ( 0 == map.point_size() ) {
    error_log << QObject::tr("Не указаны станции, по которым необходимо построить вертикальный разрез атмосферы");
    return false;
  }
  doc_->setMap(map);

  auto prop = doc_->property();
  prop.MergeFrom(map.document());
  doc_->setProperty(prop);
  doc_->init();

  switch (map.source()) {
    case meteo::map::proto::DataSource::kSurface:
      return createSurfaceVProfile( map );
    case meteo::map::proto::DataSource::kField:
      return createFieldVProfile( map );
    default:
      error_log << QObject::tr(" Неизвестный тип источника данных ");
      return false;
    }

//  return true;
}

bool VBuilder::createSurfaceVProfile(const proto::Map &map)
{
  QList<zond::PlaceData> list;
  QDateTime dt = QDateTime::fromString(QString::fromStdString(map.datetime()), "yyyy-MM-ddThh:mm:ss");
  QDateTime dts = dt.addSecs(-(map.hour()*3600));
  auto ctrl = std::unique_ptr<meteo::rpc::Channel>(meteo::global::serviceChannel( meteo::settings::proto::kSrcData ));
  if ( nullptr == ctrl ) {
    error_log << QObject::tr("Не удалось установить соденинение с сервисом данных.");
    return false;
  }
  for(int i = 0, isz = map.point_size(); i<isz;++i){
    surf::DataRequest req;
    req.add_station(map.point(i).index());
    req.add_type(surf::kAeroFix);
    switch (map.profile()) {
    case proto::ProfileType::kTimeList:
      case proto::ProfileType::kSpace:
      req.set_date_start(map.point(i).date_time());
      req.set_date_end(map.point(i).date_time());
      req.set_only_last(true);
      break;
    case proto::ProfileType::kBulletinTime:
      req.set_date_start(dts.toString(Qt::ISODate).toStdString());
      req.set_date_end(map.datetime());
      req.set_only_last(false);
      break;
    case proto::ProfileType::kTime:
        req.set_date_start(dts.toString(Qt::ISODate).toStdString());
        req.set_date_end(map.datetime());
        req.set_only_last(false);
      break;
    case proto::ProfileType::kBulletinSpace:
      req.set_date_start(dts.toString(Qt::ISODate).toStdString());
      req.set_date_end(map.datetime());
      req.set_only_last(true);
      break;
    default:
      break;
    }
    surf::TZondValueReply* reply = ctrl->remoteCall(&surf::SurfaceService::GetTZondOnStation, req, 120000);
    if ( nullptr == reply ) {
      error_log << QObject::tr("Не удалось получить ответ от сервиса данных на запрос GetTZondOnStation = %1")
                   .arg( QString::fromStdString( req.Utf8DebugString() ) );
      continue;
    }
    if ( 0 == reply->data_size() ) {
      delete reply;
      continue;
    }
    fillZond(reply, &list, map.echelon());
    delete reply;
  }
  return createDoc(&list, map);
}

bool VBuilder::createFieldVProfile(const proto::Map &map)
{
  if ( false == map.has_center()) {
    rpc::Channel* ctrl_field = global::serviceChannel(settings::proto::kField);
    if( nullptr == ctrl_field ) {
      return false;
    }
    field::DataRequest reqCenters;
    reqCenters.set_date_start(map.datetime());
    auto respCenters = ctrl_field->remoteCall(&field::FieldService::GetAvailableCentersAD, reqCenters, 240000);
    if (nullptr == respCenters) {
      return false;
    }
    if ( 0 == respCenters->info_size() ) {
      return false;
    }
    const_cast<proto::Map&>(map).set_center( respCenters->info(0).number());
  }
  switch (map.profile()) {
  case proto::ProfileType::kSpace:
    return createFieldSpace( map );
    break;
  case proto::ProfileType::kTime:
    return createFieldTime( map );
    break;
  case proto::ProfileType::kTimeList:
    return createFieldTimeList( map );
    break;
  case proto::ProfileType::kBulletinSpace:
    return createFieldBulletinSpace( map );
    break;
  case proto::ProfileType::kBulletinTime:
    return createFieldBulletinTime( map );
    break;
  default:
    error_log << QObject::tr("Неизвестный тип профиля");
    return false;
    break;
  }
}

bool VBuilder::createFieldSpace(const proto::Map &map)
{
  QList<zond::PlaceData> list;
  //QDateTime dt = QDateTime::fromString(QString::fromStdString(map.datetime()), "yyyy-MM-ddThh:mm:ss");
  //QDateTime dts = dt.addSecs(-(map.hour()*3600));
  //QDateTime dte = dt.addSecs((map.hour()*3600));

  rpc::Channel* ctrl_field = global::serviceChannel(settings::proto::kField);
  if( nullptr == ctrl_field ) {
    error_log.msgBox() << msglog::kNoConnect.arg(global::serviceTitle(settings::proto::kField));
    return false;
  }

  field::DataRequest request;
  //fillRequest( request );
  request.add_center(map.center());
  request.set_need_field_descr(true);
  request.set_only_best(true);
  request.set_only_last(true);

  for(int i = 0, isz = map.point_size(); i<isz;++i)  {
    surf::Point* p = request.add_coords();
    p->CopyFrom(map.point(i));
    request.set_forecast_start(map.point(i).date_time());//FIXME Нужно определиться с временем отбора данных
    request.set_forecast_end(map.point(i).date_time());
  }
  surf::TZondValueReply* reply = ctrl_field->remoteCall(&field::FieldService::GetADZond, request, 240000);
  fillZond(reply,&list, map.echelon(), &request);
  delete ctrl_field;
  delete reply;
  return createDoc(&list, map );
}

bool VBuilder::createFieldTime(const proto::Map &map)
{
  QList<zond::PlaceData> list;
  QDateTime dt = QDateTime::fromString(QString::fromStdString(map.datetime()), "yyyy-MM-ddThh:mm:ss");
  QDateTime dts = dt.addSecs(-(map.hour()*3600));
//  QDateTime dte = dt.addSecs((map.hour()*3600));
  rpc::Channel* ctrl_field = global::serviceChannel(settings::proto::kField);
  if( nullptr == ctrl_field ) {
    error_log.msgBox() << msglog::kNoConnect.arg(global::serviceTitle(settings::proto::kField));
    return false;
  }
  for(int i = 0, isz = map.point_size(); i<isz;++i)  {
    field::DataRequest request;
    //fillRequest( request );
    request.add_center(map.center());
    surf::Point* p = request.add_coords();
    p->CopyFrom(map.point(i));
    request.set_need_field_descr(true);
    request.set_only_best(true);
    request.set_only_last(false);
    request.set_forecast_start(dts.toString(Qt::ISODate).toStdString());
    request.set_forecast_end(map.datetime());
    surf::TZondValueReply* reply = nullptr;
    reply = ctrl_field->remoteCall(&field::FieldService::GetADZond, request, 240000);
    if (nullptr == reply) {
      continue;
    }
    fillZond(reply, &list, map.echelon(), &request);
    delete reply;
  }
  return createDoc(&list, map );
}

bool VBuilder::createFieldTimeList(const proto::Map &map)
{
  QList<zond::PlaceData> list;
  rpc::Channel* ctrl_field = global::serviceChannel(settings::proto::kField);
  if( nullptr == ctrl_field ) {
    error_log.msgBox() << msglog::kNoConnect.arg(global::serviceTitle(settings::proto::kField));
    return false;
  }
  for(int i = 0, isz = map.point_size(); i<isz;++i)  {
    field::DataRequest request;
    //fillRequest( request );
    request.add_center(map.center());
    surf::Point* p = request.add_coords();
    p->CopyFrom(map.point(i));
    request.set_need_field_descr(true);
    request.set_only_best(true);
    request.set_only_last(true);
    request.set_forecast_start(map.point(i).date_time());
    request.set_forecast_end(map.point(i).date_time());
    surf::TZondValueReply* reply = nullptr;
    reply = ctrl_field->remoteCall(&field::FieldService::GetADZond, request, 240000);
    if (nullptr == reply) {
      continue;
    }
    fillZond(reply,&list,map.echelon(), &request);
    delete reply;
  }
  return createDoc(&list, map );
}

bool VBuilder::createFieldBulletinSpace(const proto::Map &map)
{
  QList<zond::PlaceData> list;
  rpc::Channel* ctrl_field = global::serviceChannel(settings::proto::kField);
  if( nullptr == ctrl_field ) {
    error_log.msgBox() << msglog::kNoConnect.arg(global::serviceTitle(settings::proto::kField));
    return false;
  }
  for (int i = 0, sz=map.point_size(); i<sz;++i) {
    surf::Point point = map.point(i);
    field::DataRequest reqDate;
    reqDate.set_date_start(point.date_time());
    reqDate.add_center(map.center());
    if ( 0 == i ) {
      reqDate.set_only_best(true);
    }
    else if ( (sz-1) == i ) {
      reqDate.set_only_last(true);
    }
    auto respDate = ctrl_field->remoteCall(&field::FieldService::GetNearDate, reqDate, 240000);
    if (nullptr == respDate) {
      continue;
    }
    else if (0 == respDate->date_size()) {
      continue;
    }
    auto respInfo = getStationInfo(QString::fromStdString(point.index()));
    if (nullptr == respInfo ) {
      continue;
    }
    else if (0 == respInfo->station_size()) {
      continue;
    }
    field::DataRequest reqZond;
    reqZond.set_forecast_start(respDate->date(0));
    reqZond.set_forecast_end(respDate->date(0));
    reqZond.add_center(map.center());
    surf::Point* p = reqZond.add_coords();
    p->set_fi(respInfo->station(0).position().lat_radian());
    p->set_la(respInfo->station(0).position().lon_radian());
    p->set_height(respInfo->station(0).position().height_meters());
    p->set_index(respInfo->station(0).station());
    reqZond.set_only_best(true);
    reqZond.set_only_last(true);
    //fillRequest( reqZond );
    auto respZond = ctrl_field->remoteCall(&field::FieldService::GetADZond, reqZond, 240000);
    fillZond(respZond,&list,point.echelon());
    delete respZond;
    delete respInfo;
  }
 return createDoc(&list, map );
}

bool VBuilder::createFieldBulletinTime(const proto::Map &map)
{
  QList<zond::PlaceData> list;
  QDateTime dt = QDateTime::fromString(QString::fromStdString(map.datetime()), "yyyy-MM-ddThh:mm:ss");
  QDateTime dts = dt.addSecs(-(map.hour()*3600));
  QDateTime dte = dt;
  rpc::Channel* ctrl_field = global::serviceChannel(settings::proto::kField);
  if( nullptr == ctrl_field ) {
    error_log.msgBox() << msglog::kNoConnect.arg(global::serviceTitle(settings::proto::kField));
    return false;
  }
  if (0 == map.point_size()) {
    return false;
  }
  surf::Point point = map.point(0);
  field::DataRequest request;
  request.set_date_start(dts.addSecs(-(24*3600)).toString(Qt::ISODate).toStdString());
  request.set_date_end(dte.addSecs((24*3600)).toString(Qt::ISODate).toStdString());
  request.add_center(map.center());
  auto datesResp = ctrl_field->remoteCall(&field::FieldService::GetFieldDates,request,240000);
  if ( nullptr == datesResp ) {
    return false;
  }
  if ( 3 > datesResp->date_size() ) {
    return false;
  }
  auto respInfo = getStationInfo(QString::fromStdString(point.index()));
  if (nullptr == respInfo ) {
    return false;
  }
  else if (0 == respInfo->station_size()) {
    delete respInfo;
    return false;
  }
  field::DataRequest zondRequest;
  surf::Point* p = zondRequest.add_coords();
  p->set_fi(respInfo->station(0).position().lat_radian());
  p->set_la(respInfo->station(0).position().lon_radian());
  p->set_height(respInfo->station(0).position().height_meters());
  p->set_index(QString::number(respInfo->station(0).index()).toStdString());
  zondRequest.add_center(map.center());
  zondRequest.set_only_best(true);
  zondRequest.set_only_last(true);
  QList<std::string> dates;
  findDatesForBulletinTime(datesResp, &dates, dts, dte);
  for ( auto str : dates ) {
    //fillRequest( zondRequest );
    zondRequest.set_forecast_start( str );
    zondRequest.set_forecast_end( str );
    auto reply = ctrl_field->remoteCall(&field::FieldService::GetADZond, zondRequest, 240000);
    if (nullptr != reply) {
      fillZond(reply, &list, map.point(0).echelon());
    }
    delete reply;
  }
  delete datesResp;
  delete respInfo;
  return createDoc(&list, map);
}

bool VBuilder::createDoc(QList<zond::PlaceData>* list, const proto::Map& map)
{
  bool result = false;
  if ( nullptr == list || 0 == list->size() ) {
    return result;
  }
  if ((map.profile() == meteo::map::proto::ProfileType::kTime)
      || (map.profile() == meteo::map::proto::ProfileType::kTimeList)) {
    qSort(list->begin(),list->end(),foSortPoTime);
  }
  if  ( (map.profile() != meteo::map::proto::ProfileType::kBulletinSpace)
        && (map.profile() != meteo::map::proto::ProfileType::kBulletinTime) ) {
    result = doc_->create(*list, map.profile());
  }
  else {
    result = doc_->createForBulletin(*list, map);
  }
  return result;
}

void VBuilder::fillZond(const surf::TZondValueReply *zondRep, QList<zond::PlaceData> *list,
const int echelon, const field::DataRequest *req)
{
  if ( nullptr == zondRep ) {
    return;
  }
  for ( int j=0; j<zondRep->data_size(); ++j ) {
    QByteArray buf(zondRep->data(j).meteodata().data(), zondRep->data(j).meteodata().size() );
    zond::PlaceData adata;
    adata.setZond(buf);
    adata.zond().preobr();
    adata.setDt(adata.zond().dateTime());
    adata.setIndex(adata.zond().stIndex());
    adata.setCoord(adata.zond().coord());
    auto stRep = getStationInfo(adata.zond().stIndex());
//    debug_log << stRep->DebugString();
    if (nullptr != stRep) {
      if ( 0 < stRep->station_size() ) {
        adata.setRuName( QString::fromStdString( stRep->station(0).name().rus() ) );
        adata.setEnName( QString::fromStdString( stRep->station(0).name().rus() ) );
        adata.setName( QString::fromStdString( stRep->station(0).name().rus() ) );
      } else if ( nullptr != req) {
        if ( 0 != req->coords_size() ) {
          for (int i = 0, sz = req->coords_size(); i < sz;++i ) {
            if (( adata.coord().fi() == req->coords(i).fi() )
                && ( adata.coord().la() == req->coords(i).la() ) ) {
              adata.setName( QString::fromStdString( req->coords(i).name() ) );
            }
          }
        }
      }
      delete stRep;
    }
    adata.setEchelon(echelon);
//    adata.setHelpPoint(pdata.helpPoint());
    list->append(adata);
  }
}
/*
void VBuilder::fillRequest(field::DataRequest &request)
{
  for(const int descr: { 10009, 10004, 10051, 12101, 12103, 12108, 11003, 11004, 7004, 20010, 20011, 20012, 20013}) {
    request.add_meteo_descr(descr);
  }
  for(const int level : { 1000, 925, 850, 700, 500, 400, 300, 250, 100, 70, 50, 30, 20, 10, 5, 0 }) {
    request.add_level(level);
  }
  for(const int level_type : { 100, 200, 7, 1, 6, 102 }) {
    request.add_type_level(level_type);
  }
}
*/
void VBuilder::findDatesForBulletinTime(const field::DateReply* dateReply, QList<std::string>* dates, const QDateTime& dts, const QDateTime& dte)
{
  if ( nullptr == dateReply || nullptr == dates) {
    return;
  }
  if ( 3 == dateReply->date_size() ) {
    for ( int i = 0, sz = dateReply->date_size(); i<sz; ++i) {
      dates->append( dateReply->date(i) );
    }
    return;
  }
  int midIndx = ( dateReply->date_size()-2 )/2;
  std::string start = dateReply->date(dateReply->date_size()-1);
  std::string end = dateReply->date(0);
  std::string middle = dateReply->date(midIndx);

  for ( int i = 0; i<midIndx; ++i) {
    QDateTime dtCurrent = QDateTime::fromString(QString::fromStdString( dateReply->date(i) ), "yyyy-MM-ddThh:mm:ss");
    if (dte <= dtCurrent) {
      end = dateReply->date(i);
    }
  }
  for (int i = dateReply->date_size()-1; i>midIndx ; i--) {
    QDateTime dtCurrent = QDateTime::fromString(QString::fromStdString( dateReply->date(i) ), "yyyy-MM-ddThh:mm:ss");
    if (dts >= dtCurrent) {
      start = dateReply->date(i);
    }
  }
  dates->append(start);
  dates->append(middle);
  dates->append(end);
}

sprinf::Stations* VBuilder::getStationInfo(const  sprinf::MultiStatementRequest& request)
{
  meteo::rpc::Channel* ctrl_sprinf = meteo::global::serviceChannel( meteo::settings::proto::kSprinf );
  if ( nullptr == ctrl_sprinf ) {
    warning_log << QObject::tr("Не удалось установить соденинение с сервисом справки. ");
    return nullptr;
  }
  sprinf::Stations* response = ctrl_sprinf->remoteCall( &sprinf::SprinfService::GetStations, request, 20000 );
  if ( nullptr == response ) {
    warning_log << QObject::tr("Не получен ответ от сервиса справки.");
  }
  else if ( 0 == response->station_size() ) {
    warning_log << QObject::tr("В ответе от сервиса справки отсутствует описание станции");
  }
  else {
    delete ctrl_sprinf;
    return response;
  }
  delete ctrl_sprinf;
  delete response;
  return nullptr;
}

sprinf::Stations* VBuilder::getStationInfo(const QString& index)
{
  QRegExp regExp;
  regExp.setPattern(QString("[A-ZА-Яa-zа-я0-9]{4}"));
  bool exctMatch = false;
  exctMatch = regExp.exactMatch(index);
  sprinf::MultiStatementRequest request;
  if (true == exctMatch) {
    request.add_cccc(index.toStdString());
    return getStationInfo(request);
  }
  else {
    request.add_index(index.toInt());
    return getStationInfo(request);
  }
  return nullptr;
}

bool VBuilder::foSortPoTime(const zond::PlaceData &p1, const zond::PlaceData &p2)
{
  return p1.dt() < p2.dt();
}

}
}

