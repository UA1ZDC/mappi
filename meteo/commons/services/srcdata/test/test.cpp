#include <qapplication.h>
#include <qdir.h>

#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>

#include <cross-commons/debug/tlog.h>
#include <commons/meteo_data/meteo_data.h>
#include <commons/meteo_data/tmeteodescr.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/proto/alphanum.pb.h>
#include <commons/textproto/tprototext.h>


#include <meteo/commons/proto/surface_service.pb.h>
#include <meteo/commons/proto/services.pb.h>
#include <meteo/commons/settings/settings.h>

#include <meteo/commons/services/srcdata/meteodata_service.h>
#include <meteo/commons/services/srcdata/tzond_data_service.h>
#include <meteo/commons/services/srcdata/tdata_service.h>
#include <meteo/commons/services/srcdata/tgrib_data_service.h>

#include <meteo/commons/proto/surface.pb.h>
#include <sql/nosql/nosqlquery.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/settings/settings.h>

#include <unistd.h>

#define _DEBUG

meteo::surf::TDataServiceObj dobj;
constexpr float DEG2RAD = M_PIf / 180.0f;


void moveToThread(meteo::surf::TBaseDataService *obj){
  // Создание потока
  QThread* thread = new QThread;
  // Передаем права владения "рабочим" классом, классу QThread.
  obj->moveToThread(thread);
  dobj.appendThread(obj,thread);
  
  QObject::connect(thread, SIGNAL(started()), obj, SLOT(run()));
  QObject::connect(obj, SIGNAL(finished(TBaseDataService *)), &dobj, SLOT(finished(TBaseDataService *)));
  QObject::connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
  
  thread->start();
}



void test_getOneAeroData(const QDate &date)
{

  QDateTime sdt = QDateTime( date.addDays(0), QTime(0,0,0) );
  QDateTime edt = QDateTime( date, QTime(23,59,59) );
  meteo::surf::DataRequest req;

  req.add_station("40179");

  req.set_date_start(sdt.toString(Qt::ISODate).toUtf8().constData());
  req.set_date_end(edt.toString(Qt::ISODate).toUtf8().constData());
  req.add_type(meteo::surf::kAeroFix);
  req.set_max_srok(25*7);
  meteo::rpc::Channel * ctrl_src = meteo::global::serviceChannel(meteo::settings::proto::kSrcData);
  if ( nullptr == ctrl_src ) {
    error_log.msgBox() << meteo::msglog::kNoConnect.arg(meteo::global::serviceTitle(meteo::settings::proto::kSrcData));
    return;
  }

  meteo::surf::TZondValueReply* reply = ctrl_src->remoteCall(&meteo::surf::SurfaceService::GetTZondOnStation, req, 120000);
  // debug_log << reply->Utf8DebugString();
  delete ctrl_src;
  ctrl_src = nullptr;
  debug_log<< "data size "<< reply->data_size();

  foreach(auto a,reply->data()){
    debug_log<<"date  "<<a.date();
  }
 // delete reply;
}

void test_getOneAeroData()
{
  while(true){
    QDate date = QDateTime::currentDateTimeUtc().date();
    for(int a=0; a < 100;a++){
      test_getOneAeroData(date.addDays(-a));
    }
  }
}

void test_getManySurfaceData1()
{
  /* meteo::surf::MeteoDataService ms;

  meteo::surf::DataRequest req;
  req.set_date_start("2016-06-16 00:00:00");
  req.add_meteo_descr(11001);
  req.add_group_descr(0);
  req.set_type(0);
  meteo::surf::DataReply res;

  bool ok =  ms.runService(&req, &res);
  var(ok);

  for (int idx =0; idx < res.meteodata_size(); idx++) {
    var(idx);
    TMeteoData md;
    md << QByteArray(res.meteodata(idx).data(), res.meteodata(idx).size());
    md.printData();
  }*/
}

void test_getTlgMeteo()
{
  /* meteo::surf::MeteoDataService ms;

  meteo::surf::TlgDataRequest req;
  req.set_id(0);
  meteo::surf::DataReply res;

  bool ok =  ms.getTlgData(&req, &res);
  var(ok);

  for (int idx =0; idx < res.meteodata_size(); idx++) {
    var(idx);
    TMeteoData md;
    md << QByteArray(res.meteodata(idx).data(), res.meteodata(idx).size());
    md.printData();
  }*/
}


void test_zondgetOneAeroData()
{
  /* meteo::surf::TZondDataService ms;

  meteo::surf::DataRequest req;
  // req.add_station(26038);
  // req.set_date_start("2016-06-29 00:00:00");

  req.add_station(26063);
  req.set_date_start("2016-07-08 12:00:00");
  req.set_type(61);
  meteo::surf::ZondValueReply res;

  bool ok =  ms.getValue(&req, &res);
  var(ok);

  //var(res.Utf8DebugString());
  */
}

void test_oceangetManyData()
{
  // meteo::surf::DataRequest req;
  // req.set_date_start("2016-08-10 05:00:00");
  // meteo::surf::DataReply res;

  // meteo::surf::OceanDataService od(&req, (google::protobuf::Closure*)0);
  // bool ok = od.getManyPoints(&req, &res);
  // var(ok);

  // var(res.Utf8DebugString());

}


void test_getValues() {
  // debug_log<<"Начали тест";
  // debug_log<<"Создаём запрос";
  // meteo::surf::DataRequest* req = new meteo::surf::DataRequest;
  // //req->add_station("26063");
  // req->set_date_start("2018-08-10 12:00:00");
  // req->add_meteo_descr(11002);
  // req->set_type_level(100);
  // req->add_type(61);
  // //req->set_level_p(850);
  // req->set_type_level(100);
  // //req->set_type(0,61);
  // //meteo::surf::DataReply *res = new meteo::surf::DataReply;
  // meteo::surf::ManyZondValueReply* res = new meteo::surf::ManyZondValueReply();
  // debug_log<<"Создали";

  // TAPPLICATION_NAME("meteo");
  //    ::meteo::gSettings(meteo::global::Settings::instance());
  //    if ( false == meteo::global::Settings::instance()->load() ) {
  //      error_log << QObject::tr("Не удалось загрузить настройки.");
  //      return ;
  //    }
  // meteo::rpc::Channel* ctrl = meteo::global::serviceChannel( meteo::settings::proto::kSrcData );
  // debug_log<<"Создали ктрл";
  // if (0 == ctrl) {
  //    error_log<<"Не удалось установить соединение с сервисом данных";
  //   return;
  // }
  // res = ctrl->remoteCall( &meteo::surf::SurfaceService::GetZondDataOnLevel, *req, 100000 );
  // delete ctrl;
  // debug_log<<"Должны всё получить";
  // res->PrintDebugString();

}

void test_getValue() {
  debug_log<<"Начали тест";
  debug_log<<"Создаём запрос";
  meteo::surf::DataRequest* req = new meteo::surf::DataRequest;
  //req->add_station("26063");
  req->set_date_start("2018-08-07 12:00:00");
  req->add_meteo_descr(11002);
  req->add_station("26063");
  req->set_max_srok(220);
  req->set_type_level(100);
  //req->set_date_end("2018-08-10 18:00:00");
  req->add_type(61);
  //req->set_level_p(850);
  req->set_type_level(100);
  //req->set_type(0,61);
  //meteo::surf::DataReply *res = new meteo::surf::DataReply;
  meteo::surf::ZondValueReply* res = new meteo::surf::ZondValueReply();
  debug_log<<"Создали";

  TAPPLICATION_NAME("meteo");
  ::meteo::gSettings(new meteo::Settings);
  if ( false == meteo::gSettings()->load() ) {
    return;
  }
  meteo::rpc::Channel* ctrl = meteo::global::serviceChannel( meteo::settings::proto::kSrcData );
  debug_log<<"Создали ктрл";
  if (0 == ctrl) {
    error_log<<"Не удалось установить соединение с сервисом данных";
    return;
  }
  res = ctrl->remoteCall( &meteo::surf::SurfaceService::GetZondDataOnStation, *req, 100000 );
  delete ctrl;
  debug_log<<"Должны всё получить";
  res->PrintDebugString();
}

void test_getMeteoData(const QDate & date)
{
  debug_log<<"Начали тест getMeteoData" << date.toString();
  debug_log<<"Создаём запрос";

  QDateTime sdt = QDateTime( date.addDays(0), QTime(0,0,0) );
  QDateTime edt = QDateTime( date, QTime(23,59,59) );


  meteo::surf::DataRequest* req = new meteo::surf::DataRequest;
  //req->add_station("26063");
  ::meteo::surf::Polygon* reg = req->mutable_region();
  ::meteo::surf::Point* p= reg->add_point();
  p->set_fi(60*DEG2RAD);
  p->set_la(30*DEG2RAD);
  req->set_as_proto(true);
  req->set_date_start(sdt.toString(Qt::ISODate).toUtf8().constData());
  req->set_date_end(edt.toString(Qt::ISODate).toUtf8().constData());
  req->add_meteo_descrname("T");
  //req->add_meteo_descr(10051);
  //req->add_meteo_descr(20001);
  //req->set_type_level(100);
  req->add_type(1);
  req->add_type(61);
  req->set_level_p(0);
  req->set_type_level(1);
  req->set_duplication(true);
  // req->set_query_type(meteo::surf::rGetDataCount);
  
  //req->set_type(0,61);
  //meteo::surf::DataReply *res = new meteo::surf::DataReply;
  debug_log<<req->DebugString();
  meteo::surf::CountDataReply* res = new meteo::surf::CountDataReply();
  debug_log<<"Создали";

  meteo::rpc::Channel* ctrl = meteo::global::serviceChannel( meteo::settings::proto::kSrcData );
  debug_log<<"Создали ктрл";
  if (0 == ctrl) {
    error_log<<"Не удалось установить соединение с сервисом данных";
    return;
  }
  res = ctrl->remoteCall( &meteo::surf::SurfaceService::GetDataCount, *req, 100000 );
  delete ctrl;
  debug_log<<"Должны всё получить";
  // debug_log<<res->meteodata_proto_size();
  debug_log<<res->DebugString();
  /* for(int i = 0, sz = res->meteodata_proto_size(); i<sz;++i) {
    meteo::surf::MeteoDataProto proto = res->meteodata_proto(i);
    proto.PrintDebugString();
  }*/
}

void test_getMeteoData(){

  while(true){
    QDate date = QDateTime::currentDateTimeUtc().date();
    for(int a=0; a < 100;a++){
      test_getMeteoData(date.addDays(-a));
    }
  }
}

void test_getMeteoDataOnStation()
{

  TAPPLICATION_NAME("meteo");
  ::meteo::gSettings(new meteo::Settings);
  if ( false == meteo::gSettings()->load() ) {
    return;
  }


  meteo::rpc::Channel* ch = meteo::global::serviceChannel(meteo::settings::proto::kSrcData);
  if ( nullptr == ch ) {
    error_log << QObject::tr("Нет связи с сервисом данных. Расчет индекса пожарной опасности не возможен.");
    return ;
  }

  // meteodescr::TMeteoDescriptor* mdescr = TMeteoDescriptor::instance();
  // auto category_descr = mdescr->descriptor("category");

  QMap< QPair< QString, int >, double > R24map;
  meteo::surf::DataRequest request;
  //  request.add_type(surf::kSynopFix);
  QDate date = QDateTime::currentDateTimeUtc().date();

  QDateTime sdt = QDateTime( date.addDays(-3), QTime(0,0,0) );
  QDateTime edt = QDateTime( date, QTime(23,59,59) );
  request.set_date_start( sdt.toString( Qt::ISODate ).toStdString() );
  request.set_date_end( edt.toString( Qt::ISODate ).toStdString() );
  request.set_level_p(0);
  request.set_type_level(1);
  request.set_query_type(meteo::surf::kNearestTermValue);
  request.add_meteo_descrname(QString("T").toStdString());
  //   request.set_term(date.toString(Qt::ISODate).toStdString());

  auto pnt = request.mutable_region()->add_point();
  pnt->set_fi(60*DEG2RAD);
  pnt->set_la(30*DEG2RAD);
  request.set_query_type(meteo::surf::kNearPoCoord);
  request.set_rast(10000);
  request.set_as_proto(true);

  meteo::surf::DataReply* resp = ch->remoteCall(&meteo::surf::SurfaceService::GetMeteoData, request, 300000 );
  //  delete ch; ch = nullptr;
  if ( nullptr == resp) {
    error_log << QObject::tr("Не получена информация о количестве осадков. Расчет индекса пожарной опасности не возможен.");
    return ;
  }
  debug_log << resp->Utf8DebugString();
  for ( const std::string& data: resp->meteodata() ) {
    QByteArray barr(data.data(), data.size());
    TMeteoData md;
    md << barr;
    md.printData();
    /* QString stindent = mdescr->stationIdentificator(md);
    int category = md.getValue( category_descr, -1, false );
    if ( true == md.hasParam("R24") ) {
      TMeteoParam R24 = md.meteoParam("R24");
      if ( control::SPECIAL_VALUE > R24.quality() ) {
        R24map.insert( qMakePair( stindent, category ), R24.value() );
      }
    }*/
  }


}
//namespace meteo {
//namespace surf {


//void test_MeteoDataService_getOneSurfaceData2(){
//  debug_log << __FUNCTION__;
//  DataRequest req;
//  req.add_type(kSynopType);
//  req.add_station("14567");
//  req.set_date_start(QDateTime(QDate(2018, 8, 4), QTime(0,0,0)).toString(Qt::ISODate).toStdString());
//  req.set_date_end(QDateTime(QDate(2018, 8, 6), QTime(0,0,0)).toString(Qt::ISODate).toStdString());
//  req.add_meteo_descrname("Tw");

//  meteo::rpc::Channel* ctrl = meteo::global::serviceChannel(meteo::settings::proto::kSrcData);
//  DataReply* reply = ctrl->remoteCall(&meteo::surf::SurfaceService::GetMeteoDataOnStation, req, 30000);
//  debug_log << req.Utf8DebugString();
//  debug_log << reply->Utf8DebugString();
//}


//void test_MeteoDataService_getOneSurfaceData1(){
//  debug_log << __FUNCTION__;
//  DataRequest req;
//  req.add_type(kSynopType);
//  req.add_station("14567");
//  req.set_date_start(QDateTime(QDate(2018, 8, 5), QTime(0,0,0)).toString(Qt::ISODate).toStdString());
//  req.add_meteo_descrname("Tw");

//  meteo::rpc::Channel* ctrl = meteo::global::serviceChannel(meteo::settings::proto::kSrcData);
//  DataReply* reply = ctrl->remoteCall(&meteo::surf::SurfaceService::GetMeteoDataOnStation, req, 30000);
//  debug_log << req.Utf8DebugString();
//  debug_log << reply->Utf8DebugString();
//}

//void test_TSurfaceDataService_getManySurfaceData1(){
//  debug_log << __FUNCTION__;
//  meteo::surf::DataRequest req;
//  QDateTime dt = QDateTime(QDate(2018, 8, 4), QTime(23, 0, 0));

//  req.add_type(kAirplane);
//  req.set_date_start(dt.toString(Qt::ISODate).toStdString());
//  req.add_meteo_descr(10051);
//  req.set_level_p(0);
//  req.set_type_level(1);

//  meteo::rpc::Channel* ctrl = meteo::global::serviceChannel(meteo::settings::proto::kSrcData);
//  ValueReply* reply = ctrl->remoteCall( &meteo::surf::SurfaceService_Stub::GetValueOnStation, req, 30000);
//  debug_log << req.Utf8DebugString();
//  debug_log << reply->Utf8DebugString();
//}

//void test_GetOceanById(){
//  debug_log << __FUNCTION__;
//  meteo::surf::SimpleDataRequest req;

//  QDateTime dt = QDateTime(QDate(2018, 8, 6), QTime(11, 0, 0));
//  req.set_text(dt.toString(Qt::ISODate).toStdString());
//  req.add_id("52085");

//  meteo::rpc::Channel* ctrl = meteo::global::serviceChannel(meteo::settings::proto::kSrcData);
//  meteo::surf::DataReply* reply = ctrl->remoteCall( &meteo::surf::SurfaceService::GetOceanById, req, 30000);
//  debug_log << req.Utf8DebugString();
//  debug_log << reply->Utf8DebugString();
//}

//void test_GetAvailableOcean(){
//  debug_log << __FUNCTION__;
//  meteo::surf::DataRequest req;

//  QDateTime dt = QDateTime(QDate(2018, 8, 6), QTime(11, 0, 0));
//  req.set_date_start(dt.toString(Qt::ISODate).toStdString());
//  req.add_type(kOceanType);

//  meteo::rpc::Channel* ctrl = meteo::global::serviceChannel(meteo::settings::proto::kSrcData);
//  meteo::surf::OceanAvailableReply* reply = ctrl->remoteCall( &meteo::surf::SurfaceService::GetAvailableOcean, req, 30000);
//  debug_log << req.Utf8DebugString();
//  debug_log << reply->Utf8DebugString();
//}

//void test_getManyOceanDataField(){
//  debug_log << __FUNCTION__;
//  meteo::surf::DataRequest req;
// //"{ "find": "meteodata", "filter": { "dt": { "$gte": { "$date": "2018-08-08T08:30:00Z" }, "$lte": { "$date": "2018-08-08T09:30:00Z" } }, "level": 0, "level_type": 1, "descr": 22043 } }"
//  QDateTime dt = QDateTime(QDate(2018, 8, 6), QTime(11, 0, 0));

//  req.add_type(kOceanType);
//  req.set_date_start(dt.toString(Qt::ISODate).toStdString());
//  req.set_level_h(0);
//  req.set_type_level(1);
//  req.add_meteo_descr(22043);

//  meteo::rpc::Channel* ctrl = meteo::global::serviceChannel( meteo::settings::proto::kSrcData );
//  meteo::surf::ValueReply* reply = ctrl->remoteCall( &meteo::surf::SurfaceService::GetValueOnStation, req, 300000 );
//  debug_log << "Запрос к srcdata" << req.Utf8DebugString();
//  debug_log << "Ответ от srcdata" << reply->Utf8DebugString();
//}


//void test_TSurfaceDataService_getOneSurfaceData1(){
//  debug_log << __FUNCTION__;
//  meteo::surf::DataRequest req;
//  QDateTime dt = QDateTime(QDate(2018, 8, 5), QTime(3, 0, 0));

//  req.add_type(kAeroWind);
//  req.set_date_start(dt.toString(Qt::ISODate).toStdString());
//  req.add_meteo_descr(10051);
//  req.add_station("26063");

//  meteo::rpc::Channel* ctrl = meteo::global::serviceChannel( meteo::settings::proto::kSrcData );
//  meteo::surf::ValueReply* reply = ctrl->remoteCall( &meteo::surf::SurfaceService::GetValueOnStation, req, 300000 );
//  debug_log << "Запрос к srcdata" << req.Utf8DebugString();
//  debug_log << "Ответ от srcdata" << reply->Utf8DebugString();
//}


//void test_TSurfaceDataService_getManySurfaceData2(){
//  debug_log << __FUNCTION__;
//  meteo::surf::DataRequest req;
//  QDateTime dtStart = QDateTime(QDate(2018, 8, 5), QTime(0, 0, 0));
//  QDateTime dtEnd = dtStart.addDays(1);

//  req.add_type(kAeroWind);
//  req.set_date_start(dtStart.toString(Qt::ISODate).toStdString());
//  req.set_date_end(dtEnd.toString(Qt::ISODate).toStdString());
//  req.add_meteo_descr(10051);
//  req.add_station("26063");

//  meteo::rpc::Channel* ctrl = meteo::global::serviceChannel( meteo::settings::proto::kSrcData );
//  meteo::surf::ValueReply* reply = ctrl->remoteCall( &meteo::surf::SurfaceService::GetValueOnStation, req, 300000 );
//  debug_log << "Запрос к srcdata" << req.Utf8DebugString();
//  debug_log << "Ответ от srcdata" << reply->Utf8DebugString();
//}

//void test_TSurfaceDataService_getManySurfaceData3(){
//  debug_log << __FUNCTION__;
//  meteo::surf::DataRequest req;
//  QDateTime dt = QDateTime(QDate(2018, 8, 5), QTime(3, 0, 0));

//  req.add_type(kAeroWind);
//  req.set_date_start(dt.toString(Qt::ISODate).toStdString());
//  req.add_meteo_descr(10051);
//  req.add_meteo_descr(20001);
//  req.add_station("26063");

//  meteo::rpc::Channel* ctrl = meteo::global::serviceChannel( meteo::settings::proto::kSrcData );
//  meteo::surf::ValueReply* reply = ctrl->remoteCall( &meteo::surf::SurfaceService::GetValueOnStation, req, 300000 );
//  debug_log << "Запрос к srcdata" << req.Utf8DebugString();
//  debug_log << "Ответ от srcdata" << reply->Utf8DebugString();
//}

//}

//}


void test_getMeteoDataFromGrib()
{
  meteo::surf::GribDataRequest request;
  ::grib::GribInfo* ginfo = request.mutable_info();
  request.set_date_start("2020-10-29T00:00:00");
  ginfo->set_level_type( 100 );
  ginfo->set_level( 500 );
  ginfo->set_center( 4 );
  ginfo->mutable_hour()->set_hour( 0 );
  ginfo->set_model( 2 );
  ginfo->set_net_type( 3 );

  // QStringList descrs = QStringList();
  // for ( auto d : descrs ) {
  //   ginfo->add_param( TMeteoDescriptor::instance()->descriptor(d) );
  // }

  ginfo->add_param(10009);
  //ginfo->add_param(11003);
  ginfo->add_param(11004);
  // ginfo->add_param(12101);
  // ginfo->add_param(12108);

  var(request.Utf8DebugString());

  meteo::rpc::Channel* ctrl = meteo::global::serviceChannel( meteo::settings::proto::kSrcData );
  if ( nullptr == ctrl ) {
    error_log << QObject::tr("Не удалось установить соединение с сервисом данных");
    return;
  }
  
  meteo::surf::DataReply* reply = ctrl->remoteCall( &meteo::surf::SurfaceService::GetMeteoDataFromGrib, request, 100000 );
  delete ctrl;
  
  var(reply->result());
  var(reply->comment());
  var(reply->meteodata_size());
  
  delete reply;
}

int main (int argc, char* argv[])
{
  TAPPLICATION_NAME("meteo");

  QTextCodec::setCodecForLocale( QTextCodec::codecForLocale() );
  //meteo::gSettings(new meteo::Settings());
  // meteo::gSettings()->load();


  int flag = 0, opt;
  QString type;

  while ((opt = getopt(argc, argv, "123456789h")) != -1) {
    switch (opt) {
      case '1':
        flag = 1;
      break;
      case '2':
        flag = 2;
      break;
      case '3':
        flag = 3;
      break;
      case '4':
        flag = 4;
      break;
      case '5':
        flag = 5;
      break;
      case '6':
        flag = 6;
      break;
      case '7':
        flag = 7;
      break;
      case '8':
        flag = 8;
      break;
      case '9':
        flag = 9;
      break;

      case 'h':
        info_log << "Usage:" << argv[0]<<"-1|-2|-3|-4|-5\n";
        exit(0);
      break;
      default: /* '?' */
        error_log<<"Option"<<opt<<"not realized";
        exit(-1);
    }
  }
  var(flag);

  ::meteo::gSettings(meteo::global::Settings::instance());
  QCoreApplication app(argc, argv);

  if ( false == meteo::global::Settings::instance()->load() ) {
    error_log << QObject::tr("Не удалось загрузить настройки.");
    return EXIT_FAILURE;
  }


  //test_getMeteoDataOnStation();


  //  if (flag == 0) {
  //    //meteo::surf::test_TSurfaceDataService_getManySurfaceData2();
  //    /*
  //    meteo::surf::test_TSurfaceDataService_getOneSurfaceData1();
  //    meteo::surf::test_TSurfaceDataService_getManySurfaceData3();

  //    meteo::surf::test_getManyOceanDataField();
  //    meteo::surf::test_GetAvailableOcean();
  //    meteo::surf::test_GetOceanById();
  //    meteo::surf::test_MeteoDataService_getOneSurfaceData1();
  //    meteo::surf::test_MeteoDataService_getOneSurfaceData2();
  //    meteo::surf::test_TSurfaceDataService_getManySurfaceData1();*/
  //    debug_log << "All tests finished";
  //    return 0;
  //  }

  switch (flag) {
    case 1:
      debug_log<<"test_getOneAeroData()";
      test_getOneAeroData();
    break;
    case 2:
      debug_log<<"test_getManySurfaceData1()";
      test_getManySurfaceData1();
    break;
    case 3:
      debug_log<<"test_getTlgMeteo()";
      test_getTlgMeteo();
    break;
    case 4:
      debug_log<<"test_zondgetOneAeroData()";
      test_zondgetOneAeroData();
    break;
    case 5:
      debug_log<<"test_oceangetManyData()";
      test_oceangetManyData();
    break;
    case 6:
      debug_log<<"test_getValues()";
      test_getValues();
    break;
    case 7:
      //    debug_log<<"test_getValue()";
      //    test_getValue();
      debug_log<<"getMeteoData()";
      test_getMeteoData();
    break;
    case 8:
      debug_log<<"test_getMeteoDataonStation()";
      test_getMeteoDataOnStation();
    break;
    case 9:
      debug_log<<"test_getMeteoDataFromGrib()";
      test_getMeteoDataFromGrib();
    break;
    default: {}
  }

  return 0;//app.exec();
}
