#include <qdir.h>
#include <qcoreapplication.h>
#include <qtranslator.h>
#include <qtextcodec.h>

#include <meteo/commons/global/log.h>
#include <meteo/commons/proto/forecast.pb.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/proto/field.pb.h>
#include <meteo/commons/proto/surface_service.pb.h>
#include <meteo/commons/proto/field.pb.h>
#include <meteo/commons/settings/settings.h>

#include <commons/textproto/pbtools.h>

#include <commons/meteo_data/meteo_data.h>
#include <commons/meteo_data/complexmeteo.h>
#include <commons/meteo_data/tmeteodescr.h>

#include <sql/dbi/dbi.h>
#include <sql/dbi/dbiquery.h>
#include <sql/nosql/nosqlquery.h>



// перерасчитываем оправдываемость
bool saveForecastOpr(const QString& station, int station_type)
{
  bool ok=true;

  meteo::rpc::Channel* ctrl_forecast = meteo::global::serviceChannel(meteo::settings::proto::kForecastData);
  if(nullptr == ctrl_forecast ) {
    ok=false;
    return ok;
  }

  //
  // выполняем запрос по расчету оправдываемости
  //

  meteo::forecast::AccuracyRequest areq;
  areq.mutable_punkt()->set_index(station.toStdString());
  areq.mutable_punkt()->set_type(station_type);
  areq.set_center(meteo::global::kCenterWeather);

  meteo::field::SimpleDataReply* simplerep = ctrl_forecast->remoteCall(&meteo::forecast::ForecastData::CalcForecastOprStation, areq, 5000);
  if ( nullptr == simplerep ) {
    debug_log << QObject::tr("Ошибка расчета оправдываемости по станции") << station;
    ok = false;
  }
  else {
    info_log << QObject::tr("Оправдываемость по станции %1 рассчитана").arg(station) << simplerep->comment();
  }
  
  delete simplerep;
  delete ctrl_forecast;
  return ok;
}

//
void saveObservedValues(const meteo::DbiEntry& doc, const QDateTime& end, meteo::Dbi* db)
{
  if (nullptr == db) {
    return;
  }
  
  meteo::rpc::Channel* channel = meteo::global::serviceChannel(meteo::settings::proto::kSrcData);
  if ( nullptr == channel ) {
    return;
  }
  
  //запрос метеоданных, соответствующих найденным прогнозам
  meteo::surf::DataRequest req;
  req.set_date_start(doc.valueDt("dt_beg").toString(Qt::ISODate).toStdString());
  req.set_date_end(end.toString(Qt::ISODate).toStdString());
  req.set_level_p(doc.valueInt32("level"));
  req.set_type_level(doc.valueInt32("level_type"));
  meteo::Array descrs = doc.valueArray("descrname");
  while (true == descrs.next()) {
    req.add_meteo_descrname(descrs.valueString().toStdString());
  }
  req.add_station(doc.valueString("station").toStdString());
  req.add_station_type(doc.valueInt32("station_type"));
  
  
  meteo::surf::DataReply* resp = channel->remoteCall(&::meteo::surf::SurfaceService::GetMeteoData, req, 90000);
  if (resp == nullptr) {
    delete channel;
    return;
  }
  
  // var(req.Utf8DebugString());
  // var(resp->Utf8DebugString());
  
  //сохранение наблюдавшихся величин
  for ( int i=0,isz=resp->meteodata_size(); i<isz; ++i ) {
    TMeteoData md;
    md << pbtools::fromBytes(resp->meteodata(i));
    //md.printData();
      
    QDateTime mdt = TMeteoDescriptor::instance()->dateTime(md);
    
    for (auto descrname :  req.meteo_descrname()) {
      TMeteoParam* param = md.meteoParamPtr(QString::fromStdString(descrname));
      if (nullptr != param && param->isValid()) {
	auto query = db->queryptrByName("insert_forecast_accuracy_observed");
	if(nullptr == query) {
	  error_log << QObject::tr("Не найден запрос insert_forecast_accuracy_observed");
	  break;
	}
	query->argDt("start_dt", mdt.toString(Qt::ISODate));
	query->arg("level",      int(round(req.level_p())));
	query->arg("level_type", req.type_level());
	query->arg("station",    req.station(0));
	query->arg("station_type", req.station_type(0));
	query->arg("descrname",  descrname);
	query->arg("value",      param->value());
	
	//var(query->query());
	if (!query->exec()) {
	  error_log << QObject::tr("Не удалось выполнить запрос = %1").arg(query->query());
	}
      }
    }
    
  }

  delete resp;  
  delete channel;
}

//сохранение наблюдавшихся значений по станциям
void saveFieldValues(const meteo::DbiEntry& doc, const QDateTime& /*end*/, meteo::Dbi* db)
{
  if (nullptr == db) {
    return;
  }
  
  meteo::rpc::Channel* channel = meteo::global::serviceChannel(meteo::settings::proto::kField);
  if ( nullptr == channel ) {
    return;
  }
  
  //запрос метеоданных, соответствующих найденным прогнозам
  meteo::field::DataRequest req;
  req.set_need_field_descr(true);
  req.set_date_start(doc.valueDt("dt_beg").toString(Qt::ISODate).toStdString());
  req.set_date_end(doc.valueDt("dt_beg").toString(Qt::ISODate).toStdString());
  req.add_hour(0);

  req.add_level(doc.valueInt32("level"));
  req.add_type_level(doc.valueInt32("level_type"));
  meteo::surf::Point* coord = req.add_coords();
  coord->set_fi(MnMath::deg2rad(doc.valueDouble("lat")));
  coord->set_la(MnMath::deg2rad(doc.valueDouble("lon")));
  meteo::Array descrs = doc.valueArray("descrname");
  while (true == descrs.next()) {
    req.add_meteo_descr(TMeteoDescriptor::instance()->descriptor(descrs.valueString()));
  } 
  
  meteo::field::ValueDataReply* resp = channel->remoteCall(&::meteo::field::FieldService::GetForecastValues, req, 90000);
  if (resp == nullptr) {
    delete channel;
    return;
  }

  delete channel;
  
  // var(req.Utf8DebugString());

  
  for (auto data : resp->data()) {
    auto query = db->queryptrByName("insert_forecast_accuracy");
    if(nullptr == query) {
      error_log << QObject::tr("Не найден запрос insert_forecast_accuracy");
      break;
    }

    if (!data.has_fdesc()) {
      continue;
    }

    //var(data.Utf8DebugString());
    
    query->arg("center",     data.fdesc().center());
    query->argDt("dt_beg",   data.fdesc().date());
    query->argDt("dt_end",   data.fdesc().date());
    query->arg("level",      int(round(data.fdesc().level())));
    query->arg("level_type", data.fdesc().level_type());
    query->arg("station",    doc.valueString("station"));
    query->arg("station_type", doc.valueInt32("station_type"));
    query->arg("descrname",    data.fdesc().descr_name());
    query->arg("field_value",  float(data.value()));
    query->argDt("dt",     data.fdesc().date());
    

    // var(query->query());
    if (!query->exec()) {
      error_log << QObject::tr("Не удалось выполнить запрос = %1").arg(query->query());
    }
  }
  
  
  delete resp;
  
}

//сохранение значений из полей
void calcOpravdByStation(const QDateTime& beg, const QDateTime& end)
{ 
  std::unique_ptr<meteo::Dbi> db(meteo::global::dbForecast());

  //запрос прогнозов с временем окончания позже предыдущего запуска (на сутки назад)
  auto query = db->queryptrByName("get_forecast_info");
  if(nullptr == query) {
    error_log << QObject::tr("Не найден запрос get_forecast_info");
    return;
  }
  query->arg("dt", beg);
  QString error;
  if (!query->execInit(&error)) {
    error_log << error;
    return;
  }

  //var(query->query());

  QList<QPair<QString, int>> station_list;
  
  while ( true == query->next()) {
    const meteo::DbiEntry& doc = query->entry();

    //сохранение наблюдавшихся значений по станциям
    saveObservedValues(doc, end, db.get());
    //сохранение значений из полей
    saveFieldValues(doc, end, db.get());

    if (!station_list.contains(QPair<QString, int>(doc.valueString("station"), doc.valueInt32("station_type")))) {
      station_list.append(QPair<QString, int>(doc.valueString("station"), doc.valueInt32("station_type")));
    }
  }
  
  //расчет оправдываемости
  for (auto val : station_list) {
    saveForecastOpr(val.first, val.second);
  }   

}

int appMain( int argc, char* argv[] )
{


  TAPPLICATION_NAME("meteo");
  meteo::global::setLogHandler();
  ::meteo::gSettings(meteo::global::Settings::instance());
  if ( false == meteo::global::Settings::instance()->load() ) {
    error_log << QObject::trUtf8("Не удалось загрузить настройки.");
    return EXIT_FAILURE;
  }
  QTime timer;
  
  QCoreApplication app(argc, argv);
  ::setlocale(LC_NUMERIC, "C"); // локаль для записи координат в mongo (libbson)

  QDateTime end = QDateTime::currentDateTimeUtc();
  QDateTime beg = end.addDays(-3).addSecs(60*60);

  calcOpravdByStation(beg, end);

 return 0;
 
}



