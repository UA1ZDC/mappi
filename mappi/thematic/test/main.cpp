#include <mappi/proto/thematic.pb.h>
#include <meteo/commons/proto/field.pb.h>

#include <mappi/global/global.h>
#include <commons/proc_read/daemoncontrol.h>

#include <QCoreApplication>

#include <unistd.h>
#include <getopt.h>

using namespace mappi;
void printHelp(const QString& progName)
{
	info_log << QObject::tr("\nИспользование: %1 [option]\n").arg(progName)
					 << QObject::tr("Опции:\n")
					 << QObject::tr("-n <satname>\t\t Название спутника\n")
					 << QObject::tr("-s <start_dt>\t\t Дата/время начала прием\n")
					 << QObject::tr("-i <instrument_type>\t Тип прибора (через запятую, если несколько)\n")
					 << QObject::tr("-h \t\t Справка\n");
}

//-n 'NOAA 19' -s '2022-07-12 13:41:37'
//test_ndvi("NOAA 19", QDateTime(QDate(2022, 07, 12), QTime(13, 41, 37)));

int main(int argc, char *argv[])
{
  QCoreApplication app(argc, argv);
  TAPPLICATION_NAME("meteo");

  
  // ::meteo::gGlobalObj(new ::mappi::MappiGlobal);
  ::meteo::gSettings(::meteo::global::Settings::instance());
  if( !::meteo::global::Settings::instance()->load() ){
    return EXIT_FAILURE;
  }

  /////////////

/////////////

  meteo::rpc::Channel* ch = meteo::global::serviceChannel(meteo::settings::proto::kField);
  if (nullptr == ch) {
    error_log << meteo::msglog::kServiceConnectFailedSimple.arg(meteo::global::serviceTitle(meteo::settings::proto::kField));
    return 0;
  }

  meteo::field::DataRequest request;
  QDateTime dt = QDateTime::fromString("2022-11-08 00:00:00", Qt::ISODate);
  //QDateTime dt = TSqlQuery::datetimeFromString( info.datetime() );

  // request.set_date_start(dt.addSecs(info.hour()*3600).toString( Qt::ISODate ).toStdString() );
  request.set_date_start(dt.toString( Qt::ISODate ).toStdString() );
  //  request.set_date_end( info.datetime() );
  request.add_meteo_descr( 12101 );
  request.add_hour( 0);
  request.add_level( 1000 );
  request.add_center( 250 );
  request.add_type_level( 100 );
  meteo::field::DataReply* reply = ch->remoteCall( &meteo::field::FieldService::GetGeoTiff, request, 50000);
  delete ch;
  if (nullptr == reply ) {
    error_log << QObject::tr("Ответ от севиса полей не получен");
    return 0;
  }
  if ( false == reply->result() ) {
    debug_log << QObject::tr("Нет данных по запросу %1")
    .arg( QString::fromStdString( reply->error() ) )<< reply->Utf8DebugString();
    delete reply;
    return 0;
  }
  QByteArray arr(reply->fielddata().data(), reply->fielddata().size());

  QFile fl("/home/user/tmp/out.tif");
  fl.open(QIODevice::ReadWrite);
  fl.write(arr);
  fl.close();



  
  return 0;
}
