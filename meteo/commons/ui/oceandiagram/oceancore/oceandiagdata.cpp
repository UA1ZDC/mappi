#include "oceandiagdata.h"

#include <cross-commons/debug/tlog.h>

#include <commons/meteo_data/tmeteodescr.h>

#include <meteo/commons/global/global.h>

#include <meteo/commons/services/sprinf/sprinfservice.h>
#include <meteo/commons/services/fieldata/fieldservice.h>
#include <meteo/commons/ocean/placedata.h>


#include <meteo/commons/proto/surface_service.pb.h>
#include <qelapsedtimer.h>
#include <qsettings.h>
#include <qdir.h>

#define start_time QElapsedTimer _timer__name; _timer__name.start()
#define reset_time(text) debug_log << text << _timer__name.restart() << "msec"


namespace meteo {
namespace odiag {

static const int kTimeout = 1000000;

OceanDiagData::OceanDiagData( )
{
  sdt_ = QDateTime::currentDateTimeUtc();
  edt_ = QDateTime::currentDateTimeUtc();
  stype_ = kSrcData;
  type_ = kSensType;
  center_ = -1;
  
  
  dataFieldReqMap_[kSensType] = QList<Section>() << kStartDt << kHour << kCoords << kCenter;
  dataFieldReqMap_[kForecType]  = QList<Section>() << kStartDt << kHour << kCoords << kCenter;
  
}

OceanDiagData::~OceanDiagData()
{
  
}

bool OceanDiagData::loadAvailableData(QMap<QString, ocean::PlaceData> *data)
{
  if ( nullptr == data ) { return false; }
  data->clear();

  surf::DataRequest request;
  request.add_type(meteo::surf::kOceanType);
  request.set_date_start(sdt_.toString(Qt::ISODate).toUtf8().constData());

  rpc::Channel * ctrl_src = meteo::global::serviceChannel(meteo::settings::proto::kSrcData);
  if(nullptr == ctrl_src ) {
    error_log.msgBox() << msglog::kNoConnect.arg(meteo::global::serviceTitle(meteo::settings::proto::kSrcData));
    return false;
  }
  
  surf::OceanAvailableReply* reply = ctrl_src->remoteCall(&surf::SurfaceService::GetAvailableOcean, request, 120000);
  delete ctrl_src;ctrl_src=nullptr;
  
  if ( nullptr == reply ) {
    error_log << QObject::tr("При попытке получить данные в коде FM64 Ext. для ДСМ ответ от сервиса данных не получен.");
    return false;
  }

  for (int idx = 0; idx < reply->point_size(); idx++) {
    ocean::PlaceData pd(QString::fromStdString(reply->point(idx).name()),
                        GeoPoint::fromDegree(reply->point(idx).lat(), reply->point(idx).lon()),
                        reply->point(idx).count(),
			reply->point(idx).maxlevel(),
			QDateTime::fromString(QString::fromStdString(reply->point(idx).dt()), Qt::ISODate));
    // var(reply->point(idx).name());
    // var(reply->point(idx).dt());
    data->insert(QString::fromStdString( reply->point(idx).id()), pd);
  }
  
  delete reply;
  return true;
}

bool OceanDiagData::fillProfile( QString* id, ocean::PlaceData* data)
{
  if ( nullptr == data ) { return false; }

  if (data->ocean().urovenList().size() != 0) {
    return true;
  }
  //var(data->dt());
  surf::SimpleDataRequest request;
  request.add_id(id->toStdString());
  request.set_text(data->dt().toString(Qt::ISODate).toStdString());
  //  request.set_text(sdt_.toString(Qt::ISODate).toUtf8().constData());

  rpc::Channel* ch_src = meteo::global::serviceChannel(meteo::settings::proto::kSrcData);
  if(nullptr == ch_src ) {
    error_log.msgBox() << msglog::kNoConnect.arg(meteo::global::serviceTitle(meteo::settings::proto::kSrcData));
    return false;
  }
  
  auto reply = std::unique_ptr<surf::DataReply>(ch_src->remoteCall(&surf::SurfaceService::GetOceanById, request, 120000));
  delete ch_src;ch_src=nullptr;
  
  if ( nullptr == reply || 0 == reply->meteodata_size()) {
    error_log << QObject::tr("При попытке получить данные в коде FM64 Ext. для ДСМ ответ от сервиса данных не получен.");
    return false;
  }

  QByteArray buf(reply->meteodata(0).data(), reply->meteodata(0).size());
  TMeteoData md;
  md << buf;

  //md.printData();
  
  data->setData(md);

  return true;
}



}
}
