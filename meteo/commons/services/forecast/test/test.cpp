#include <qtranslator.h>
#include <qcoreapplication.h>
#include <qtranslator.h>
#include <qtextcodec.h>

#include <meteo/commons/global/log.h>
#include <meteo/commons/proto/forecast.pb.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/proto/field.pb.h>
#include <meteo/commons/settings/settings.h>
#include <commons/textproto/pbtools.h>


int main( int argc, char* argv[] )
{

  TAPPLICATION_NAME("meteo");
  meteo::global::setLogHandler();
  ::meteo::gSettings(meteo::global::Settings::instance());
  if ( false == meteo::global::Settings::instance()->load() ) {
    error_log << QObject::tr("Не удалось загрузить настройки.");
    return EXIT_FAILURE;
  }
  QCoreApplication* app = new QCoreApplication( argc, argv, false );

  meteo::rpc::Channel* ctrl_forecast = meteo::global::serviceChannel(meteo::settings::proto::kForecastData);
  if(nullptr == ctrl_forecast ) {
    error_log << meteo::msglog::kNoConnect.arg(meteo::settings::proto::kForecastData);
    return -1;
  }

 /* meteo::forecast::Dummy req;
  meteo::forecast::ForecastMethodList* reply =
      ctrl_forecast->remoteCall(&meteo::forecast::ForecastData::GetForecastMethodsList, req, 30000);
  if ( 0 == reply ) {
    error_log << meteo::msglog::kServerAnswerFailed;
    return -1;
  }
  debug_log << reply->Utf8DebugString();
 */ debug_log << "*****************************************************************";
  meteo::forecast::ForecastResultRequest req1;
  QDateTime dt = QDateTime::currentDateTimeUtc();

  req1.set_time_start(pbtools::toString(dt));
  ::meteo::surf::Point* point = req1.mutable_coord();
  point->set_fi(meteo::DEG2RAD*60);
  point->set_la(meteo::DEG2RAD*30);
  point->set_name("SPB");
  point->set_index("26063");
  req1.set_method_name("Metel");
  while(1){
  meteo::forecast::ForecastFizResult* reply1 =
      ctrl_forecast->remoteCall(&meteo::forecast::ForecastData::GetForecastMethodsResultRun, req1, 30000);

  if ( 0 == reply1 ) {
    error_log << meteo::msglog::kServerAnswerFailed;
    return -1;
  }
  debug_log << reply1->Utf8DebugString();
  delete reply1;
}


 return app->exec();
}
