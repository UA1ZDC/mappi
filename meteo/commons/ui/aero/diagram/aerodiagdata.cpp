#include "aerodiagdata.h"

#include <cross-commons/debug/tlog.h>


#include <meteo/commons/rpc/rpc.h>
#include <meteo/commons/global/global.h>

#include <meteo/commons/zond/placedata.h>


#include <meteo/commons/proto/field.pb.h>
#include <meteo/commons/proto/surface_service.pb.h>
#include <qelapsedtimer.h>
#include <qsettings.h>
#include <qdir.h>



namespace meteo {
namespace adiag {

static const int kTimeout = 1000000;

AeroDiagData::AeroDiagData( )

{
  dt_ = QDateTime::currentDateTimeUtc();
  stype_ = kSrcData;
  //  type_ = kSensType;
  center_ = -1;
  isDataRestore = true;
  forecHour = 0;
}

AeroDiagData::~AeroDiagData()
{
}


surf::TZondValueReply*  AeroDiagData::loadSrcData(const QString& aindex, bool only_last )
{
  surf::DataRequest req;

  QString index = aindex;
  index.remove(QRegExp("^0*"));

  req.add_station(index.toStdString());
  req.set_date_start(dt_.toString(Qt::ISODate).toUtf8().constData());
  req.set_date_end(dt_.toString(Qt::ISODate).toUtf8().constData());
  req.add_type(surf::kAeroFix);
  req.set_max_srok(25*7);
  req.set_only_last(only_last);
  rpc::Channel * ctrl_src = global::serviceChannel(settings::proto::kSrcData);
  if ( nullptr == ctrl_src ) {
    error_log.msgBox() << msglog::kNoConnect.arg(global::serviceTitle(settings::proto::kSrcData));
    return nullptr;
  }

  surf::TZondValueReply* reply = ctrl_src->remoteCall(&surf::SurfaceService::GetTZondOnStation, req, 120000);
  delete ctrl_src;
  ctrl_src = nullptr;
  return reply;
}

surf::TZondValueReply* AeroDiagData::loadFieldData(zond::PlaceData *pdata , bool isLast )
{
  field::DataRequest request = makeDataFieldRequest(pdata->coord());
  request.set_only_last(isLast);
  rpc::Channel * ctrl_field = global::serviceChannel(settings::proto::kField);
  if(nullptr == ctrl_field ) {
    error_log.msgBox() << msglog::kNoConnect.arg(global::serviceTitle(settings::proto::kField));
    return nullptr;
  }

  surf::TZondValueReply* reply = ctrl_field->remoteCall(&field::FieldService::GetADZond, request, 240000);
  delete ctrl_field;ctrl_field=nullptr;
  return reply;
}


bool AeroDiagData::getData(zond::PlaceData* pdata,QList <zond::PlaceData*> *rez_pdata, bool is_last)
{
  rez_pdata->clear();


  surf::TZondValueReply *_zond =nullptr;
  if ( stype_ == adiag::kSrcData ) {
    _zond = loadSrcData(pdata->index(),is_last);
  }
  if ( stype_ == adiag::kFieldData ) {
    _zond = loadFieldData(pdata,is_last);
  }

  if ( nullptr == _zond  || 0 > _zond->data_size() ) {
    return false;
  }

  QList<QDateTime> dtlist;
  for ( int i=0; i<_zond->data_size(); ++i ) {
    QByteArray buf(_zond->data(i).meteodata().data(), _zond->data(i).meteodata().size() );
    zond::PlaceData* adata = new zond::PlaceData();
    adata->setZond(buf);
    adata->zond().preobr();
    adata->setDt(adata->zond().dateTime());
    adata->setCoord(adata->zond().coord());
    adata->setEnName(pdata->enName());
    adata->setRuName(pdata->ruName());
    adata->setName(pdata->name());
    adata->setIndex(pdata->index());
    if ( dtlist.contains(adata->zond().dateTime()) ) { continue; }
    dtlist.append(adata->zond().dateTime());
    rez_pdata->append(adata);
  }
  delete _zond;

  return true;
}


bool AeroDiagData::zondToPdata(zond::PlaceData *pdata,const ::meteo::surf::OneZondValueOnStation &one_zond )
{
  pdata->setDt(QDateTime());
  pdata->zond().clear();

  //debug_log<<one_zond.DebugString();
  QDateTime tmp;
  if ( !one_zond.has_point()) {
    return false;
  }

  int dataCount = 0;
  for ( int i=0,isz=one_zond.ur_size(); i<isz; ++i ) {
    const surf::Uroven& lvl = one_zond.ur(i);

    if ( !lvl.has_p() || !lvl.p().has_value() || !lvl.p().has_quality() || control::DOUBTFUL <= lvl.p().quality() )
    { continue; }

    double p = lvl.p().value();
    if ( p >= 200 && p <= 1050 ) { ++dataCount; }
  }
  //debug_log<<"dataCount" << dataCount;
  if ( dataCount > 2 ) {
    QString dtStr = QString::fromUtf8(one_zond.date().c_str());
    tmp = QDateTime::fromString(dtStr, Qt::ISODate);
    tmp.setTimeSpec(Qt::UTC);
    if(one_zond.has_point()){
      GeoPoint vs1= GeoPoint(one_zond.point().fi(),one_zond.point().la());
      pdata->setCoord(vs1);
    }
    if(one_zond.has_station()){
      pdata->setIndex(one_zond.station());
    }
    pdata->setDt(tmp);
    pdata->setData(one_zond);
  }

  return true;
}

bool AeroDiagData::loadCenters( QMap< int , QString > *list )
{
  rpc::Channel * ctrl_field = meteo::global::serviceChannel(meteo::settings::proto::kField);
  if(nullptr == ctrl_field ) {
    error_log.msgBox() << msglog::kNoConnect.arg(meteo::global::serviceTitle(meteo::settings::proto::kField));
    return false;
  }

  field::DataRequest req;
  req.set_forecast_start(dt_.toString(Qt::ISODate).toUtf8().constData());
  req.set_forecast_end(dt_.toString(Qt::ISODate).toUtf8().constData());
 // req.add_hour(forecHour*3600);
  meteo::field::CentersResponse* resp =
  ctrl_field->remoteCall(&field::FieldService::GetAvailableCentersAD, req, kTimeout);
  delete ctrl_field;ctrl_field=nullptr;
  if ( nullptr == resp ) {
    error_log << QObject::tr("Ошибка при взаимодействии с сервисом "
    "Не получен ответ на запрос GetAvailableCentersAD");
    return false;
  }

  for ( int i=0,isz=resp->info_size(); i<isz; ++i ) {
    const ::meteo::field::CenterInfo& cinfo = resp->info(i);
    if(cinfo.has_name() && cinfo.has_number())
      list->insert(cinfo.number(), QString::fromUtf8(cinfo.name().c_str()));
  }

  delete resp;

  return true;
}

field::DataRequest AeroDiagData::makeDataFieldRequest(const GeoPoint& coord)
{
  field::DataRequest req;
  req.set_forecast_start(dt_.toString(Qt::ISODate).toUtf8().constData());
  req.set_forecast_end(dt_.toString(Qt::ISODate).toUtf8().constData());
  //req.set_date_end(dt_.toString(Qt::ISODate).toUtf8().constData());
 // req.add_hour(forecHour*3600);
  req.set_only_best(true);
  req.set_only_last(true);
  surf::Point* p = req.add_coords();
  p->set_fi(coord.fi());
  p->set_la(coord.la());
  p->set_height(coord.alt());
  if(-1 < center_){
    req.add_center(center_);
  }
  return req;
}


}
}
