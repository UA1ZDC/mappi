#include <qdir.h>
#include <qcoreapplication.h>
#include <qtranslator.h>
#include <qtextcodec.h>
#include <QtScript/QScriptEngine>

#include <meteo/commons/global/log.h>
#include <meteo/commons/proto/forecast.pb.h>
#include <meteo/commons/services/forecast/service/tforecastservice.h>
#include <meteo/commons/services/forecast/tdataprovider.h>
#include <meteo/commons/services/forecast/tforecastlist.h>
#include <meteo/commons/services/forecast/tforecast.h>
#include <meteo/commons/services/forecast/tfmodel.h>
#include <meteo/commons/services/forecast/tfitem.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/proto/field.pb.h>
#include <meteo/commons/settings/settings.h>
// #include <meteo/bank/settings/banksettings.h>

void testModelItem(TFItem* item){
  if(nullptr == item ) return;
  debug_log << item->getDescr()<<item->getText()<<item->getItemData().toString();
  for(int i =0; i< item->childCount();++i){
    testModelItem(item->getChilds().at(i));
  }
}

void runOnePoint(const StationData &st)
{
  QDateTime date_time = QDateTime::currentDateTimeUtc();
  TForecastList forecastList(date_time, st);
  const QString scripts_path = MnCommon::varPath("meteo") + "/forecast/scripts/";
  //const QString scripts_path = QString(getenv("BUILD_DIR")) + "/var/meteo/forecast/scripts/";
  QDir dir(scripts_path);
  dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
  dir.setSorting(QDir::Name);
  QStringList filters;
  filters << "*.js";
  //dir.setNameFilters(filters);
  meteo::rpc::Channel * ctrl_field=0;
  ctrl_field = meteo::global::serviceChannel(meteo::settings::proto::kField);
  if(0 == ctrl_field ) {
    error_log << meteo::msglog::kNoConnect.arg(meteo::settings::proto::kField);
  }
  meteo::field::DataRequest request;
  request.set_date_start(QDateTime::currentDateTimeUtc().addSecs(-12*3600).toString(Qt::ISODate).toStdString());
  request.set_date_end(QDateTime::currentDateTimeUtc().toString(Qt::ISODate).toStdString());
  // request.add_center(34);
  request.add_level(1000);
  request.add_type_level(100);
  request.add_hour(0);
  request.add_meteo_descr(10009);
  request.set_need_field_descr(true);
  ::meteo::surf::Point* p =  request.add_coords();
  p->set_fi( st.coord.fi());
  p->set_la(st.coord.la());
  int curSynSit_ =-1;
  if(ctrl_field){
    meteo::field::ValueDataReply * reply =
        ctrl_field->remoteCall( &meteo::field::FieldService::getSynSit, request,  30000);
    delete ctrl_field; ctrl_field = nullptr;
    if ( 0 == reply ) {
      error_log << meteo::msglog::kServiceAnswerFailed;
    }
    int i =reply->data_size()-1;
    QString tp = QObject::tr("Ситуация автоматически не определена");
    for ( ; i > 0; --i ){
      const meteo::field::OnePointData& d = reply->data(i);
      if(d.fdesc().count_point() < 2000) continue;
      curSynSit_ = (int)d.value();
      tp = QObject::tr("Ситуация определена по полю давления за %1 центр %2")
           .arg(QString::fromStdString(d.fdesc().date()))
           .arg(QString::fromStdString(d.fdesc().center_name()));
      break;
    }

    info_log<< tp;
    delete reply; reply = 0;
  }

  QFileInfoList list = dir.entryInfoList();
  info_log << QObject::tr("Обнаружено %1 методов").arg((int)list.size()/2);
  forecastList.setSynSit(curSynSit_);
  forecastList.setSeason(TForecast::oprSeason(QDateTime::currentDateTimeUtc().date()) );
  foreach (QFileInfo fileInfo, list) {
    if(fileInfo.fileName().contains("forecast")) continue; //TODO
    // debug_log << fileInfo.filePath();
    forecastList.setDateTime(QDateTime::currentDateTimeUtc());
    TForecast * cf = forecastList.addMethodNoRun(fileInfo.fileName());
    if( forecastList.validateMethod(cf->methodName())){
      cf->runForecastOnly(false);
      if( nullptr != cf->getSrcModel()){
        testModelItem( cf->getSrcModel()->rootItem() );
      }
    }
  }
}


int appMain( int argc, char* argv[] )
{
  TAPPLICATION_NAME("meteo");
  meteo::global::setLogHandler();
  ::meteo::gSettings(meteo::global::Settings::instance());
  if ( false == meteo::global::Settings::instance()->load() ) {
    error_log << QObject::tr("Не удалось загрузить настройки.");
    return EXIT_FAILURE;
  }
  QCoreApplication* app = new QCoreApplication( argc, argv, false );
  ::setlocale(LC_NUMERIC, "C"); // локаль для записи координат в mongo (libbson)

  meteo::rpc::Channel* ctrl_forecast = meteo::global::serviceChannel(meteo::settings::proto::kForecastData);
  if(nullptr == ctrl_forecast ) {
    error_log << meteo::msglog::kNoConnect.arg(meteo::settings::proto::kForecastData);
    return -1;
  }

  meteo::forecast::PunktRequest req;
  req.set_requestcode(meteo::forecast::kPunktGetRequest);
  meteo::forecast::PunktResponce* reply =
      ctrl_forecast->remoteCall(&meteo::forecast::ForecastData::GetForecastPunkts, req, 30000);
  if ( 0 == reply ) {
    error_log << meteo::msglog::kServerAnswerFailed;
    return -1;
  }
  
  foreach (::meteo::forecast::PunktValue punkt, reply->punkts()) {
    if(false == punkt.isactive()) continue;
    info_log << QObject::tr("Начало работы ПК прогнозирования по пункту %1 (%2)")
                .arg(QString::fromStdString(punkt.name()))
                .arg(QString::fromStdString(punkt.stationid()));
    //debug_log<<reply->DebugString();
    StationData st(meteo::GeoPoint( punkt.fi(), punkt.la(),punkt.height()),
                   QString::fromStdString(punkt.name()),
                   QString::fromStdString(punkt.stationid()),
                   punkt.station_type());
    runOnePoint(st);
  }
  return 0;
  return app->exec();
}



