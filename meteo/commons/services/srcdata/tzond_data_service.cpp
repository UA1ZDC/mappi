#include "tzond_data_service.h"

#include <qmap.h>
#include <qprocess.h>
#include <qstring.h>
#include <qcoreapplication.h>
#include <qjsonarray.h>
#include <qjsonvalue.h>
#include <qjsonobject.h>
#include <qjsondocument.h>
#include <qelapsedtimer.h>

#include <cross-commons/debug/tlog.h>

#include <commons/meteo_data/meteo_data.h>
#include <meteo/commons/global/global.h>

#include <sql/nosql/nosqlquery.h>
#include <sql/nosql/array.h>

#include <meteo/commons/zond/zond.h>
#include <meteo/commons/zond/turoven.h>
#include <meteo/commons/zond/urovenlist.h>
#include <meteo/commons/zond/zondfunc.h>
#include <meteo/commons/services/sprinf/clienthandler.h>

#include <commons/mathtools/mnmath.h>

namespace meteo {
namespace surf {

TZondDataService::TZondDataService(const meteo::surf::DataRequest *r, google::protobuf::Closure* done ):
  TBaseDataService(done  ),
  data_request_(r),
  req_type_(rGetNo)
{
  //auto conf = meteo::Global::instance()->mongodbConfMeteo();
  for(int i =0; i < kEndAeroType; ++i)
  {
    aero_types_<< i;//meteo::surf::kAeroType
  }
//             << meteo::surf::kAeroFix
//             //<< meteo::surf::kAeroArtillery
//             << meteo::surf::kAeroWind
//             //<< meteo::surf::kAeroFixLayer
//             << meteo::surf::kAeroMobSea
//             << meteo::surf::kAeroMobAir
//             << meteo::surf::kAeroMob
//             << meteo::surf::kAeroWindMob
//             << meteo::surf::kAeroBufr;
}

TZondDataService::~TZondDataService(  ){
  //debug_log<<"поток "<< this <<" завершил работу(destructor)"<<--h;
}

void TZondDataService::run() {
  bool ret_val=false;

  switch(req_type_){
  case   rGetValue:
    if(nullptr != zond_value_repl_ && nullptr!=data_request_) {
      ret_val= getValue(data_request_,zond_value_repl_);
    }
  break;
  case   rGetValueWithCalc:
    // if(nullptr != tzond_data_reply_ && nullptr!=data_request_) {
      ret_val= getValueWithCalc( data_request_, tzond_data_reply_ );
    // }
  break;
    /* case   rGetValues:
   if(nullptr != many_zond_value_reply_ && nullptr!=data_request_) {
     ret_val= getValues(data_request_,many_zond_value_reply_);
    }*/

  break;
  case   rGetOneAeroData:
    if(nullptr != zond_value_repl_ && nullptr!=data_request_) {
      ret_val= getOneAeroData(data_request_, zond_value_repl_ );
    }
  break;
  case   rGetManyAeroData:
    if(nullptr != zond_value_repl_ && nullptr!=data_request_) {
      ret_val= getManyAeroData(data_request_,zond_value_repl_);
    }
  break;
  case   rGetTZondValue:
    if(nullptr != tzond_data_reply_ && nullptr!=data_request_) {
      ret_val= getTZondValue(data_request_,tzond_data_reply_);
    }
  break;
  case   rGetTZondSatValue:
    if(nullptr != tzond_data_reply_ && nullptr != data_request_) {
      ret_val= getTZondSatValue(data_request_, tzond_data_reply_);
    }
  break;
  case   rGetDates:
    if(nullptr != date_reply_ && nullptr!=data_request_) {
      ret_val= getDates(data_request_, date_reply_);
    }
  break;
  case   rGetZondAvailable:
    if(nullptr != stations_reply_ && nullptr != data_request_) {
      ret_val= getAvailable(data_request_, stations_reply_);
    }
  break;
  default:
  break;
  }
  if(!ret_val){
    //TODO
  }
  //  debug_log<<"поток "<< this <<" завершил работу(run)";
  emit finished(this);
}

void TZondDataService::setReplyType( kZondReqType r){
  req_type_ = r;
}

void TZondDataService::setDataRequest(const meteo::surf::DataRequest* r){
  data_request_ = r;
}

void TZondDataService::setZondValueReply( meteo::surf::ZondValueReply* r){
  zond_value_repl_ = r;
}

void TZondDataService::setDataReply( meteo::surf::DataReply* r){
  data_reply_ = r;
}

void TZondDataService::setDataReply( meteo::surf::TZondValueReply* r){
  tzond_data_reply_ = r;
}

void TZondDataService::setDateReply(DateReply *r) {
  date_reply_ = r;
}

void TZondDataService::setZondStationsReply(StationsDataReply *r) {
  stations_reply_ = r;
}


bool TZondDataService::getTZondValue(const meteo::surf::DataRequest* req,
                                     meteo::surf::TZondValueReply* res)
{
  if( 0 == req->station_size() ){
    return false;
  }
  if ( 0 == req->type_size() ) {
    debug_log << QObject::tr("Запрос заполнен не полностью");
    res->set_result(false);
    res->set_comment( QObject::tr("Запрос заполнен не полностью").toStdString() );
    return false;
  }
  int datatype = req->type(0);
  QString station_id_ =  QString::fromStdString(req->station(0));
  QString dts = QString::fromStdString(req->date_start());
  QString dte = dts;
  if ( true == req->has_date_end() ) {
    dte = QString::fromStdString(req->date_end());
  }
  QList <QString> dlist;
  if(dts == dte){
    dlist<<dts;
  } else {
    if(!getAeroDataList(station_id_, dts,  dte, datatype, &dlist)){
      return false;
    }
  }
  if(0 == dlist.size()) {
    res->set_result(false);
    return false;
  }
  if(true == req->only_last()){
    zond::Zond znd;
    if(! getOneZondData( station_id_, dlist[0], &znd)){
      return false;
    }
    meteo::surf::OneTZondValueOnStation* onezond = res->add_data();
    znd.restoreUrList();

    onezond->mutable_point()->set_fi(znd.coord().fi());
    onezond->mutable_point()->set_la(znd.coord().la());
    onezond->mutable_point()->set_height(znd.coord().alt());
    onezond->mutable_point()->set_id(znd.stIndex().toStdString());
    onezond->set_date(znd.dateTime().toString(Qt::ISODate).toUtf8().constData());
    onezond->set_hour(0);
    QByteArray buf;
    znd >>buf;
    onezond->set_meteodata(buf.data(),buf.size());
    res->set_comment(msglog::kServerAnswerOK.arg("getManyAeroData").toUtf8().constData());
    res->set_result(true);
    return true;
  }
  for(int i =0; i< dlist.size(); ++i){
    zond::Zond znd;
    QString dt = dlist.at(i);
    if(! getOneZondData( station_id_, dt, &znd)){
      return false;
    }
    meteo::surf::OneTZondValueOnStation* onezond = res->add_data();
    znd.restoreUrList();

    onezond->mutable_point()->set_fi(znd.coord().fi());
    onezond->mutable_point()->set_la(znd.coord().la());
    onezond->mutable_point()->set_height(znd.coord().alt());
    onezond->mutable_point()->set_id(znd.stIndex().toStdString());
    onezond->set_date(znd.dateTime().toString(Qt::ISODate).toUtf8().constData());
    onezond->set_hour(0);
    QByteArray buf;
    znd >>buf;
    onezond->set_meteodata(buf.data(),buf.size());
  }
  res->set_comment(msglog::kServerAnswerOK.arg("getManyAeroData").toUtf8().constData());
  res->set_result(true);
  return true;

}


bool TZondDataService::getTZondSatValue(const meteo::surf::DataRequest* req,
                                        meteo::surf::TZondValueReply* res)
{
  if( 0 == req->station_size() ){
    return false;
  }
  if ( 0 == req->type_size() ) {
    debug_log << QObject::tr("Запрос заполнен не полностью");
    res->set_result(false);
    res->set_comment( QObject::tr("Запрос заполнен не полностью").toStdString() );
    return false;
  }
  int datatype = req->type(0);
  QString station_id =  QString::fromStdString(req->station(0));
  QString dts = QString::fromStdString(req->date_start());

  zond::Zond znd;

  if(! getOneZondSatData(station_id, datatype, dts, &znd)) {
    return false;
  }
  meteo::surf::OneTZondValueOnStation* onezond = res->add_data();
  onezond->mutable_point()->set_fi(znd.coord().fi());
  onezond->mutable_point()->set_la(znd.coord().la());
  onezond->mutable_point()->set_height(znd.coord().alt());
  onezond->mutable_point()->set_id(znd.stIndex().toStdString());
  onezond->set_date(znd.dateTime().toString(Qt::ISODate).toUtf8().constData());
  onezond->set_hour(0);
  QByteArray buf;
  znd >>buf;
  onezond->set_meteodata(buf.data(),buf.size());

  res->set_comment(msglog::kServerAnswerOK.arg("getManyAeroData").toUtf8().constData());
  res->set_result(true);
  return true;
}

bool TZondDataService::getValue(const meteo::surf::DataRequest* req,
                                meteo::surf::ZondValueReply* res)
{
  // значения профиля - по индексу и  времени
  if ( 1 == req->station_size() && req->has_date_start()
       && !req->has_date_end() )
  {
    if(req->has_level_h() )  return getOneAeroLevel(zond::UR_H, req, res);
    if(req->has_level_p() )  return getOneAeroLevel(zond::UR_P, req, res);
    return getOneAeroData(req,res);
  }

  // значения профилей - по индексу и интервалу времени
  if ( 1 == req->station_size() && req->has_date_start()
       && req->has_date_end() )
  {
    return getManyAeroData(req,res);
  }

  return false;
}

/**
 * @brief получаем АД с расчетами слоев, высот и индексов в прото формате
 * 
 * @param req 
 * @param res 
 * @return true 
 * @return false 
 */
bool TZondDataService::getValueWithCalc(const meteo::surf::DataRequest* req,
                                     meteo::surf::TZondValueReply* res)
{

  if( 0 == req->station_size() ){
    debug_log << QObject::tr("Запрос заполнен не полностью");
    res->set_result(false);
    res->set_comment( QObject::tr("Запрос заполнен не полностью нет станции").toStdString() );
    return false;
  }

  if ( 0 == req->type_size() ) {
    debug_log << QObject::tr("Запрос заполнен не полностью");
    res->set_result(false);
    res->set_comment( QObject::tr("Запрос заполнен не полностью").toStdString() );
    return false;
  }

  int     datatype    = req->type(0);
  QString station_id_ = QString::fromStdString( req->station(0));
  QString dts         = QString::fromStdString(req->date_start());
  QString dte         = QString::fromStdString(req->date_end());

  QList <QString> dlist;
  if(!getAeroDataList(station_id_, dts,  dte, datatype, &dlist)){
    debug_log << tr("Нет данных, соответствующих запросу");
    return false;
  }
  for(int i =0; i< dlist.size(); ++i){
    zond::Zond znd;
    if(! getOneAeroData( station_id_, dlist.at(i), &znd)){
      return false;
    }
    znd.restoreUrList();
    znd.preobr();
    meteo::surf::OneTZondValueOnStation *data =  res->add_data();

    data->mutable_point()->set_fi(znd.coord().fi());
    data->mutable_point()->set_la(znd.coord().la());
    data->mutable_point()->set_height(znd.coord().alt());
    data->mutable_point()->set_id(znd.stIndex().toStdString());
    data->set_date(znd.dateTime().toString(Qt::ISODate).toUtf8().constData());
    data->set_hour(0);


    meteo::sprinf::HandlerConnection sprinf_handler( nullptr, nullptr );
 
    sprinf::MultiStatementRequest station_req;
    sprinf::Stations station_resp;
 
    station_req.add_station( station_id_.toStdString() );
    station_req.add_type(3);
    //  station_req.add_id( station_indx_.toStdString() );
    //  station_req.add_index( station_indx_.toInt() );
    sprinf_handler.GetStations( &station_req, &station_resp );
    if ( 0 != station_resp.station_size() ) {
      auto station = station_resp.station(0);
      if ( true == station.has_name() ) {
        if ( true == station.name().has_rus() ) {
          data->mutable_point()->set_name( station.name().rus() );
        }
        if ( true == station.name().has_international() ) {
          data->mutable_point()->set_eng_name( station.name().international() );
        }
      }	      
    }
    //  debug_log << "STATION =" << station_req.Utf8DebugString() << station_resp.Utf8DebugString();
 
    //  debug_log << "QUERY ZOND =" << query->query();

    
    QList<meteo::surf::MeteoDataProto> meteodata_proto = znd.toMeteoDataProto();
    for (int i = 0; i < meteodata_proto.size(); i++) {
      meteo::surf::MeteoDataProto *mdata = data->add_meteodata_proto();
      mdata->CopyFrom(meteodata_proto.at(i));      
    }

    zond::calculateZondLayers( &znd, data ); //расчеты для зонда


//    // расчеты для зонда
//
//    QList <float> layers ;
//    // болтанка
//    if ( znd.oprGranBoltan(&layers) ){
//      fillZondLayersFromList(layers, data->mutable_boltanka(), &znd );
//    }
//    layers.clear();
//    // облачность
//    if ( znd.oprGranOblak(&layers) ){
//      fillZondLayersFromList(layers, data->mutable_cloudlayers(), &znd );
//    }
//    layers.clear();
//    // обледенение
//    if ( znd.oprGranObled(&layers) ){
//      fillZondLayersFromList(layers, data->mutable_obledenenie(), &znd );
//    }
//    layers.clear();
//    // слои конденсационный следов
//    if ( znd.oprGranTrace(&layers) ){
//      fillZondLayersFromList(layers, data->mutable_kondensate_trace(), &znd );
//    }
//    layers.clear();
//
//    // 
//    // Ищем границы КНС
//    if ( znd.oprGranKNS(&layers) ){
//      if ( layers.size()==2 ){
//        meteo::surf::ZondLayer* zondlayer = data->mutable_kns();
//        zondlayer->set_bottom( layers.at(0)   );
//        zondlayer->set_top( layers.at(1) );
//      }
//    }
//
//    // забираем тропопаузу
//    QList<zond::Uroven> ur_tropo;
//    if( true == znd.getTropo(&ur_tropo) && ur_tropo.size()>0 ) {
//      for (int idx = 0; idx < ur_tropo.size(); idx++) {
//        meteo::surf::ZondLayer *zondlayer = data->add_tropo();
//        zondlayer->set_bottom( ur_tropo.at(idx).value(zond::UR_H) );
//        zondlayer->set_top( ur_tropo.at(idx).value(zond::UR_H) );
//        zondlayer->set_t( ur_tropo.at(idx).value(zond::UR_T) );
//      }
//    }
//
//    // слои инверсии и изотремии
//    QVector<zond::InvProp> vectorlayers;
//    if ( znd.getSloiInver(&vectorlayers) ){
//      for (int i =0; i< vectorlayers.size(); ++i ){
//        meteo::surf::ZondLayer *zondlayer;
//        if ( vectorlayers.at(i).invType == zond::InvProp::InvType::INVERSIA ) {
//          zondlayer = data->add_inverse();
//        }else{
//          zondlayer = data->add_isoterm();
//        }
//        zondlayer->set_bottom( vectorlayers.at(i).h_lo   );
//        zondlayer->set_top(    vectorlayers.at(i).h_hi   );
//      }
//    }
//
//    // энергия неустойчивости
//    int energy = 0;
//    energy = znd.oprZnakEnergy( 850.0 );
//    data->set_energy( energy );
//
//    // значение индекса
//    float index_value;
//    
//    // индексы неустойчивости
//    if ( zond::kiIndex(znd, &index_value) ) {
//      // ссылка на протоструктуру
//      meteo::surf::MeteoParamProto* cur_index = data->add_indexes();
//      cur_index->set_value(index_value);
//      cur_index->set_descrname("K_index");
//    }
//    if ( zond::verticalTotalsIndex(znd, &index_value) ) {
//      // ссылка на протоструктуру
//      meteo::surf::MeteoParamProto* cur_index = data->add_indexes();
//      cur_index->set_value(index_value);
//      cur_index->set_descrname("VT_index");
//    }
//    if ( zond::crossTotalsIndex(znd, &index_value) ) {
//      // ссылка на протоструктуру
//      meteo::surf::MeteoParamProto* cur_index = data->add_indexes();
//      cur_index->set_value(index_value);
//      cur_index->set_descrname("CT_index");
//    }
//    if ( zond::totalTotalsIndex(znd, &index_value) ) {
//      // ссылка на протоструктуру
//      meteo::surf::MeteoParamProto* cur_index = data->add_indexes();
//      cur_index->set_value(index_value);
//      cur_index->set_descrname("T_index");
//    }
//    if ( zond::sweatIndex(znd, &index_value) ) {
//      // ссылка на протоструктуру
//      meteo::surf::MeteoParamProto* cur_index = data->add_indexes();
//      cur_index->set_value(index_value);
//      cur_index->set_descrname("SWEAT_index");
//    }
  }

  res->set_comment(msglog::kServerAnswerOK.arg("getManyAeroData").toUtf8().constData());
  res->set_result(true);

  return true;
}

///**
// * @brief функция, которая будет заполнять протосообщение из QList
// * 
// * @param layers 
// * @param res 
// */
//void TZondDataService::fillZondLayersFromList(QList<float> &layers, 
//                          google::protobuf::RepeatedPtrField<meteo::surf::ZondLayer> *res,
//                          zond::Zond* znd)
//{
//  // если слоев четное количество
//  if ( layers.size()%2 == 0 ){
//    for (int i =0; i< layers.size(); i+=2 ){
//      meteo::surf::ZondLayer *zondlayer = res->Add();
//      zondlayer->set_bottom( znd->oprHpoP(layers.at(i))   );
//      zondlayer->set_top( znd->oprHpoP(layers.at(i+1)) );
//    }
//  }
//  return;
//}

bool TZondDataService::getAeroDataList(const QString & station_id_,  const QString &dts,  const QString &dte, int type, QList <QString> * dlist)
{
  Q_UNUSED(type);
  QElapsedTimer ttt; ttt.start();

  std::unique_ptr<Dbi> db(meteo::global::dbMeteo());
  if ( nullptr == db.get() )  {return false;}
  auto query = db->queryptrByName("get_aero_date_list");
  if(nullptr == query) {return false;}

  query->argDt("start_dt",  dts);
  query->argDt("end_dt",  dte);
  query->arg("data_type", aero_types_);
  query->arg("station", station_id_);

  if(false == query->execInit( nullptr)){
    return false;
  }

  while ( true == query->next() ) {
    const DbiEntry& doc = query->entry();
    dlist->append(doc.valueDt("dt").toString(Qt::ISODate));
  }
  if (dlist->isEmpty()){
  // debug_log<<QObject::tr("Ответ пришёл, но данных нет. Запрос %1").arg( query->query() );
    return false;
  }
  int cur =  ttt.elapsed();
  if(1000<cur){
    warning_log << msglog::kServiceRequestTime.arg(query->query()).arg(cur);
  }
  return true;
}



bool TZondDataService::getNearDate(const QString&  station_id_,int type, int  max_srok_, QString *dts)
{
  Q_UNUSED(type);
  QElapsedTimer ttt; ttt.start();
  std::unique_ptr<Dbi> db(meteo::global::dbMeteo());
  if ( nullptr == db.get() )  {return false;}
  auto query = db->queryptrByName("get_near_date");
  if(nullptr == query) {return false;}

  if ( (max_srok_ < 0) ) {
    max_srok_ = 240;
  }
  query->arg("start_dt",  DbiQuery::datetimeFromString( *dts ).addSecs(0-(max_srok_*60*60)));
  query->argDt("end_dt",  *dts);
  query->arg("data_type", aero_types_);
  query->arg("station", station_id_);
  if(false == query->execInit( nullptr)){
    return false;
  }
  while ( true == query->next() ) {
    const DbiEntry& doc = query->entry();
    *dts = doc.valueDt("dt").toString(Qt::ISODate);
  }
  int cur =  ttt.elapsed();
  if(1000<cur){
    warning_log << msglog::kServiceRequestTime.arg(query->query()).arg(cur);
  }
  return true;
}

bool TZondDataService::getOneZondData(const QString & station_number_, const QString &dts,zond::Zond *znd)
{
  QElapsedTimer ttt; ttt.start();
  std::unique_ptr<Dbi> db(meteo::global::dbMeteo());
  if ( nullptr == db.get() )  {return false;}
  auto query = db->queryptrByName("get_one_zond_data");
  if(nullptr == query) {return false;}
  query->argDt("start_dt",  dts);
  query->arg("data_type", aero_types_);
  query->arg("station", station_number_);

  if(false == query->execInit( nullptr)){
    return false;
  }

  zond::CloudData cld;
  while ( true == query->next() ) {
    const DbiEntry& doc = query->entry();
    Array params = doc.valueArray("param");
    zond::Uroven aur;
    int level_ns = 0;
    while (params.next()) {
      Document param;
      params.valueDocument(&param);
      QString descr = param.valueString("descrname");
      double  value = param.valueDouble("value");
      double  qual  = param.valueDouble("quality");
      if ("Nh" == descr) {
        cld.set(zond::Nh,value,qual);
      } else if ("h" == descr) {
        cld.set(zond::h,value,qual);
      } else if ("CL" == descr) {
        cld.set(zond::CL,value,qual);
      } else if ("CM" == descr) {
        cld.set(zond::CM,value,qual);
      } else if ("CH" == descr) {
        cld.set(zond::CH,value,qual);
      } else {
        zond::ValueType vt = zond::UrTypePoDescr(TMeteoDescriptor::instance()->descriptor(descr));
        aur.set(vt,value,qual);
        if ( 10004 == TMeteoDescriptor::instance()->descriptor(descr)) {
          level_ns = value;
        }
      }
    }
    int  level = qRound( doc.valueDouble("level"));
    int  plevel = qRound( doc.valueDouble("level"));
    if ( ( 0 == level) && (0 != level_ns)) {
      plevel = level_ns;
    }
    //FIXME в монго было doc.valueDouble("_id.level_type")
    meteodescr::LevelType level_type = static_cast <meteodescr::LevelType> (doc.valueDouble("level_type"));
    switch (level_type) {
      case meteodescr::kGeopotentialLevel:
      case meteodescr::kHeightLevel:
      case meteodescr::kHeightMslLevel:
      case meteodescr::kDepthSurfLevel:
      case meteodescr::kDepthLevel:
      case meteodescr::kCbTopLevel:
      case meteodescr::kCbBaseLevel:
        aur.set( zond::UR_H, plevel,control::NO_CONTROL);
      break;
      default:
        aur.set( zond::UR_P, plevel,control::NO_CONTROL);
      break;
    }
    aur.setLevelType(level_type);
    aur.to_uv();
    if(!aur.isGood( zond::UR_Td))      { aur.to_Td();}
    else if(!aur.isGood( zond::UR_D))  { aur.to_D(); }
    znd->setData(doc.valueDouble("level_type"), level, aur);
    QDateTime dt;
    dt = dt.fromString(dts,Qt::ISODate);
    znd->setDateTime(dt);
    znd->setStIndex(station_number_);
    znd->setCoord(doc.valueGeo("location"));
  }
  //zond::CloudData cld;
  //getOneCloudData(station_number_, dts,&cld);
  znd->setCloud(cld);
  int cur =  ttt.elapsed();
  if(1000 < cur){
    warning_log << msglog::kServiceRequestTime.arg(query->query()).arg(cur);
  }
  return true;
}

bool TZondDataService::getOneZondSatData(const QString & station_number, int datatype,
                                         const QString &dts, zond::Zond *znd)
{
  QElapsedTimer ttt; ttt.start();

  std::unique_ptr<Dbi> db(meteo::global::dbMeteo());
  if ( nullptr == db.get() )  {return false;}
  auto query = db->queryptrByName("get_one_zond_data");
  if(nullptr == query) {return false;}
  query->argDt("start_dt",  dts);
  query->arg("data_type", QList<int>() << datatype);
  query->arg("station", station_number);
  if(false == query->execInit( nullptr)){
    return false;
  }

  while ( true == query->next() ) {
    const DbiEntry& doc = query->entry();
    Array params = doc.valueArray("param");
    zond::Uroven aur;
    int level_ns = 0;
    while (params.next()) {
      Document param;
      params.valueDocument(&param);
      QString descr = param.valueString("descrname");
      double value = param.valueDouble("value");
      double qual = param.valueDouble("quality");

      zond::ValueType vt = zond::UrTypePoDescr(TMeteoDescriptor::instance()->descriptor(descr));
      aur.set(vt,value,qual);
      if ( 10004 == TMeteoDescriptor::instance()->descriptor(descr)) {
        level_ns = value;
      }
    }
    //FIXME V MONGO BILO _id.  int  level = qRound( doc.valueDouble("_id.level"));
    int  level = qRound( doc.valueDouble("level"));
    int  plevel = qRound( doc.valueDouble("level"));
    if ( ( 0 == level) && (0 != level_ns)) {
      plevel = level_ns;
    }
    //FIXME V MONGO BILO _id.
    meteodescr::LevelType level_type = static_cast <meteodescr::LevelType> (doc.valueDouble("level_type"));
    switch (level_type) {
    case meteodescr::kGeopotentialLevel:
      aur.set( zond::UR_H, plevel,control::NO_CONTROL);
    break;
    default:
      aur.set( zond::UR_P, plevel,control::NO_CONTROL);
    break;
    }
    aur.setLevelType(level_type);

    aur.to_uv();
    if(!aur.isGood( zond::UR_Td))      { aur.to_Td();}
    else if(!aur.isGood( zond::UR_D))  { aur.to_D(); }
    //FIXME V MONGO BILO _id.
    znd->setData(doc.valueDouble("level_type"), level, aur);
    QDateTime dt;
    dt = dt.fromString(dts,Qt::ISODate);
    znd->setDateTime(dt);
    znd->setStIndex(station_number);
    //FIXME V MONGO BILO _id.
    znd->setCoord(doc.valueGeo("location"));
  }

  int cur =  ttt.elapsed();
  if(1000<cur){
    warning_log << msglog::kServiceRequestTime.arg(query->query()).arg(cur);
  }
  return true;
}

bool TZondDataService::getOneAeroData(const QString &  station_indx_, const QString &dts, zond::Zond *znd)
{
  QElapsedTimer ttt; ttt.start();
  std::unique_ptr<Dbi> db(meteo::global::dbMeteo());
  if ( nullptr == db.get() )  {return false;}
  auto query = db->queryptrByName("get_one_zond_data");
  if(nullptr == query) {return false;}
  query->argDt("start_dt",  dts);
  query->arg("data_type", aero_types_);
  query->arg("station", station_indx_);

  if(false == query->execInit( nullptr)){
    return false;
  }

  while ( true == query->next() ) {
    const DbiEntry& doc = query->entry();
    Array params = doc.valueArray("param");
    zond::Uroven aur;
    int level_ns = 0;
    while (params.next()) {
      Document param;
      params.valueDocument(&param);
      QString descr = param.valueString("descrname");
      double value = param.valueDouble("value");
      double qual = param.valueDouble("quality");
      if (("Nh" == descr)
          || ("h" == descr)
          || ("CL" == descr)
          || ("CM" == descr)
          || ("CH" == descr)) {
        continue;
      } else {
        zond::ValueType vt = zond::UrTypePoDescr(TMeteoDescriptor::instance()->descriptor(descr));
        aur.set(vt,value,qual);
        if ( 10004 == TMeteoDescriptor::instance()->descriptor(descr)) {
          level_ns = value;
        }
      }
    }
    //FIXME V MONGO BILO _id.

    int  level = qRound( doc.valueDouble("level"));
    int  plevel = qRound( doc.valueDouble("level"));
    if ( ( 0 == level) && (0 != level_ns)) {
      plevel = level_ns;
    }
    aur.set( zond::UR_P, plevel,control::NO_CONTROL);
    aur.setLevelType( static_cast <meteodescr::LevelType> (doc.valueDouble("level_type")));
    aur.to_uv();
    if(!aur.isGood( zond::UR_Td))      { aur.to_Td();}
    else if(!aur.isGood( zond::UR_D))  { aur.to_D(); }
    znd->setData(doc.valueDouble("level_type"), level, aur);
    znd->setCoord(doc.valueGeo("location"));
  }
  QDateTime dt;
  dt = dt.fromString(dts,Qt::ISODate);
  znd->setDateTime(dt);
  znd->setStIndex(station_indx_);

  int cur =  ttt.elapsed();
  if(1000<cur){
    warning_log << msglog::kServiceRequestTime.arg(query->query()).arg(cur);
  }
  return true;
}

bool TZondDataService::getDates(const DataRequest *req, DateReply *res)
{
  if( 0 == req->station_size() ){
    debug_log << QObject::tr("Запрос заполнен не полностью");
    return false;
  }
  if ( 0 == req->type_size() ) {
    debug_log << QObject::tr("Запрос заполнен не полностью");
    return false;
  }
  int datatype = req->type(0);
  QString station_id_ =  QString::fromStdString(req->station(0));
  QString dts = QString::fromStdString(req->date_start());
  QString dte = dts;
  if ( true == req->has_date_end() ) {
    dte = QString::fromStdString(req->date_end());
  }
  QList <QString> dlist;
  if(!getAeroDataList(station_id_, dts,  dte, datatype, &dlist)){
    return false;
  }
  if (0 == dlist.size()) {
    return false;
  }
  for (int i =0, sz=dlist.size(); i<sz; ++i){
    res->add_date();
    res->set_date(i, dlist.at(i).toStdString());
  }
  return true;
}


void TZondDataService::toProtoUroven(const zond::Uroven &ur_, meteo::surf::Uroven* res)
{
  meteo::surf::Value avalue;

  avalue.set_value(ur_.value(zond::UR_P));
  avalue.set_quality(ur_.quality(zond::UR_P));
  res->mutable_p()->CopyFrom(avalue);
  //set_allocated_p (&avalue);

  avalue.set_value(ur_.value(zond::UR_H)*0.1);
  avalue.set_quality(ur_.quality(zond::UR_H));
  res->mutable_h()->CopyFrom(avalue);
  //res->set_allocated_h(&avalue);

  avalue.set_value(ur_.value(zond::UR_T));
  avalue.set_quality(ur_.quality(zond::UR_T));
  res->mutable_t()->CopyFrom(avalue);
  //res->set_allocated_t(&avalue);

  avalue.set_value(ur_.value(zond::UR_Td));
  avalue.set_quality(ur_.quality(zond::UR_Td));
  res->mutable_td()->CopyFrom(avalue);
  //res->set_allocated_td(&avalue);

  avalue.set_value(ur_.value(zond::UR_D));
  avalue.set_quality(ur_.quality(zond::UR_D));
  res->mutable_d()->CopyFrom(avalue);
  //res->set_allocated_td(&avalue);

  avalue.set_value(ur_.value(zond::UR_dd));
  avalue.set_quality(ur_.quality(zond::UR_dd));
  res->mutable_dd()->CopyFrom(avalue);
  //    res->set_allocated_dd(&avalue);

  avalue.set_value(ur_.value(zond::UR_ff));
  avalue.set_quality(ur_.quality(zond::UR_ff));
  res->mutable_ff()->CopyFrom(avalue);
  //res->set_allocated_ff(&avalue);
  avalue.set_value(ur_.value(zond::UR_u));
  avalue.set_quality(ur_.quality(zond::UR_u));
  res->mutable_u()->CopyFrom(avalue);

  avalue.set_value(ur_.value(zond::UR_v));
  avalue.set_quality(ur_.quality(zond::UR_v));
  res->mutable_v()->CopyFrom(avalue);


  // res->set_group_type( ur_.groupType());
  res->set_level_type( ur_.levelType());


  if(!MnMath::isEqual(res->t().value() , BAD_METEO_ELEMENT_VAL)  &&
     !MnMath::isEqual(res->d().value(), BAD_METEO_ELEMENT_VAL)
     && MnMath::isEqual(res->td().value(),BAD_METEO_ELEMENT_VAL)){
    res->mutable_td()->set_value(res->t().value() - res->d().value() );
    res->mutable_td()->set_quality(control::AUTO_CORRECTED);
  } else {
    if(!MnMath::isEqual(res->t().value() , BAD_METEO_ELEMENT_VAL )
       &&MnMath::isEqual(res->d().value() , BAD_METEO_ELEMENT_VAL)
       && !MnMath::isEqual(res->td().value() , BAD_METEO_ELEMENT_VAL)){
      res->mutable_d()->set_value(res->t().value() - res->td().value() );
      res->mutable_d()->set_quality(control::AUTO_CORRECTED);
    }
    if(!MnMath::isEqual(res->dd().value(), BAD_METEO_ELEMENT_VAL)
       &&!MnMath::isEqual(res->ff().value() , BAD_METEO_ELEMENT_VAL)){

      float dd = res->dd().value();
      float ff  = res->ff().value();
      float u,v;
      MnMath::convertDFtoUV(dd, ff,&u, &v);
      res->mutable_u()->set_value(u);
      res->mutable_v()->set_value(v);
      res->mutable_u()->set_quality(control::AUTO_CORRECTED);
      res->mutable_v()->set_quality(control::AUTO_CORRECTED);
    }

  }
}

bool TZondDataService::getAvailable(const meteo::surf::DataRequest* req, meteo::surf::StationsDataReply* res)
{
  auto errFunc = [res](QString err)
  {
    error_log << err;
    res->set_result(false);
    res->set_comment(err.toStdString());
    return false;
  };

  if (0 == req->type_size() || !req->has_type_level() || !req->has_date_start()) {
    return errFunc(QObject::tr("Запрос заполнен не полностью"));
  }
  std::unique_ptr<Dbi> db(meteo::global::dbMeteo());
  if ( nullptr == db.get() )  {return errFunc("");}
  auto query = db->queryptrByName("get_available_zond");
  if(nullptr == query) {return errFunc("");}

  query->argDt("start_dt", req->date_start());
  query->arg("data_type", req->type());
  query->arg("level_type", req->type_level());
  QString err;
  if(false == query->execInit( &err)){
    return errFunc(err);
  }
  while ( true == query->next() ) {
    const DbiEntry& doc = query->entry();
    meteo::sprinf::MeteostationType type = meteo::sprinf::MeteostationType(doc.valueInt32("station_type"));
    QString station = doc.valueString("station");
    if ( type > meteo::sprinf::kStationUnk && !station.isEmpty() ) {
      surf::Point* pnt = res->add_coords();
      pnt->set_date_time(req->date_start());
      pnt->set_type(type);
      pnt->set_index(station.toStdString());
      if ( true == doc.hasField("station_info") ) {
        const meteo::Document& info = doc.valueDocument("station_info");
        if ( true == info.hasField("name") ) { //MONGODB
          pnt->set_eng_name(info.valueString("name").toStdString());
        }
        if ( true == info.hasField("name_ru") ) {
          pnt->set_name(info.valueString("name_ru").toStdString());
        }
        if ( true == info.hasField("satellite") ) {
          pnt->set_eng_name(info.valueString("satellite").toStdString());
        }
        if ( true == info.hasField("ru") ) {     //PSQL
          pnt->set_name( info.valueString("ru").toStdString() );
        }
        if ( true == info.hasField("international") ) {
          pnt->set_eng_name( info.valueString("international").toStdString() );
        }
      }
      meteo::GeoPoint gp = doc.valueGeo("location");
      pnt->set_fi(gp.fi());
      pnt->set_la(gp.la());
      pnt->set_height(gp.alt());
    }
  }
  res->set_result(true);
  return true;
}

bool TZondDataService::getOneAeroLevel(zond::ValueType ur_type, const meteo::surf::DataRequest* req,
                                       meteo::surf::ZondValueReply* res)
{
  if( 0 == req->station_size() ){
    return false;
  }
  QString station_indx_ =  QString::fromStdString(req->station(0));

  meteo::surf::OneZondValueOnStation *data =  res->add_data();
  data->set_station(station_indx_.toInt());
  QString dts = QString::fromStdString(req->date_start());
  data->set_date(dts.toUtf8().constData());

  zond::Zond znd;
  if(! getOneAeroData( station_indx_, dts, &znd)){
    return false;
  }
  zond::Uroven zur_;

  switch(ur_type){
  case zond::UR_P:
    if( !znd.getUrPoP(req->level_p(), &zur_) ){
      return false;
    }
  break;
  case zond::UR_H:
    if( !znd.getUrPoHH(req->level_h(), &zur_) ){
      return false;
    }
  break;
  default: return false;
  }

  //float fi = 0, la = 0;
  //TMeteoDescriptor::instance()->getCoord(md, &fi, &la);
  data->mutable_point()->set_fi(znd.coord().fi());
  data->mutable_point()->set_la(znd.coord().la());

  meteo::surf::Uroven* pur_ = data->add_ur();
  toProtoUroven(zur_ , pur_);

  res->set_comment(msglog::kServerAnswerOK.arg("getOneAeroLevel").toUtf8().constData());
  res->set_result(true);
  return true;
}

bool TZondDataService::getManyAeroData(const meteo::surf::DataRequest* req,
                                       meteo::surf::ZondValueReply* res)
{
  if( 0 == req->station_size() ){
    return false;
  }
  if ( 0 == req->type_size() ) {
    debug_log << QObject::tr("Запрос заполнен не полностью");
    res->set_result(false);
    res->set_comment( QObject::tr("Запрос заполнен не полностью").toStdString() );
    return false;
  }

  // заполняем уровни
  int     datatype    = req->type(0);
  QString station_id_ = QString::fromStdString( req->station(0));
  QString dts         = QString::fromStdString(req->date_start());
  QString dte         = QString::fromStdString(req->date_end());

  QList <QString> dlist;
  if(!getAeroDataList(station_id_, dts,  dte, datatype, &dlist)){
    //   debug_log << tr("Нет данных, соответствующих запросу");
    return false;
  }
  for(int i =0; i< dlist.size(); ++i){
    zond::Zond znd;
    if(! getOneAeroData( station_id_, dlist.at(i), &znd)){
      return false;
    }
    meteo::surf::OneZondValueOnStation *data =  res->add_data();
    data->set_station(station_id_.toInt());
    data->set_date(dts.toUtf8().constData());

    const QMap<zond::Level, zond::Uroven> &urList = znd.urovenList();

    QMap <zond::Level, zond::Uroven>::const_iterator it = urList.begin();
    QMap <zond::Level, zond::Uroven>::const_iterator eit = urList.end();

    for(;it!=eit;++it){
      meteo::surf::Uroven* ur_ = data->add_ur();

      toProtoUroven(it.value() , ur_);
    }
  }

  res->set_comment(msglog::kServerAnswerOK.arg("getManyAeroData").toUtf8().constData());
  res->set_result(true);
  return true;
}


bool TZondDataService::getOneAeroData(const meteo::surf::DataRequest* req,
                                      meteo::surf::ZondValueReply* res)
{
  if( 0 == req->station_size() ){
    return false;
  }
  if ( 0 == req->type_size() ) {
    debug_log << QObject::tr("Запрос заполнен не полностью");
    res->set_result(false);
    res->set_comment( QObject::tr("Запрос заполнен не полностью").toStdString() );
    return false;
  }
  int datatype = req->type(0);
  QString station_id_ =  QString::fromStdString(req->station(0));
  meteo::surf::OneZondValueOnStation *data =  res->add_data();

  data->set_station(station_id_.toInt());
  QString dts = QString::fromStdString(req->date_start());
  if(req->has_max_srok() && 0 != req->max_srok()){
    if (!getNearDate(station_id_,  datatype, req->max_srok(), &dts)) {
      return false;
    }
  }
  data->set_date(dts.toUtf8().constData());

  zond::Zond znd;
  if(! getOneAeroData( station_id_, dts, &znd)){
    return false;
  }
  const QMap<zond::Level, zond::Uroven> &urList = znd.urovenList();

  QMap <zond::Level, zond::Uroven>::const_iterator it = urList.begin();
  QMap <zond::Level, zond::Uroven>::const_iterator eit = urList.end();

  for(;it!=eit;++it){
    meteo::surf::Uroven* ur_ = data->add_ur();
    toProtoUroven(it.value() , ur_);
  }
  //float fi = 0 , la = 0;
  //TMeteoDescriptor::instance()->getCoord(md, &fi, &la);
  data->mutable_point()->set_fi(znd.coord().fi());
  data->mutable_point()->set_la(znd.coord().la());
  // md.printData();

  res->set_comment(msglog::kServerAnswerOK.arg("getOneAeroData").toUtf8().constData());
  res->set_result(true);
  return true;
}


}
}
