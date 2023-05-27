#include "ocean_data_service.h"

#include <qmap.h>
#include <qprocess.h>
#include <qstring.h>
#include <qcoreapplication.h>
#include <qjsondocument.h>
#include <qjsonobject.h>
#include <qjsonarray.h>

#include <cross-commons/debug/tlog.h>

#include <commons/meteo_data/meteo_data.h>
#include <commons/meteo_data/tmeteodescr.h>
#include <meteo/commons/global/global.h>
//#include <sql/nspgbase/ns_pgbase.h>
//#include <sql/nspgbase/tsqlquery.h>

#include <sql/nosql/nosqlquery.h>
#include <array>

#include <commons/mathtools/mnmath.h>

static const QString kOceanColl = "meteodata";

static const std::string msgIncorrectInputData = "Полученные данные содержат одну или несколько ошибок.";
static const std::string msgQueryExecutionError = "Ошибка при выполнении запроса в базу данных";

namespace meteo {
namespace surf {


OceanDataService::OceanDataService(const DataRequest* req, google::protobuf::Closure* done)
  : TBaseDataService(done)
  , data_request_(req)
  , simpledata_request_(nullptr)
  , value_reply_(nullptr)
  , data_reply_(nullptr)
  , available_reply_(nullptr)
{
}

OceanDataService::OceanDataService(const SimpleDataRequest* req, google::protobuf::Closure* done)
  : TBaseDataService(done)
  , data_request_(nullptr)
  , simpledata_request_(req)
  , value_reply_(nullptr)
  , data_reply_(nullptr)
  , available_reply_(nullptr)
{
}

void OceanDataService::run() {
  //debug_log<<"поток "<< this <<" начал работу(run)";
  QTime ttt; ttt.start();
  bool ret_val = false;
  switch(req_type_) {
  case rGetManyOceanDataField:
    if(nullptr != data_request_ && nullptr != value_reply_) {
      ret_val = getManyOceanDataField(data_request_, value_reply_);
    }
  break;
  case rGetAvailableOcean:
    if(nullptr != data_request_ && nullptr != available_reply_) {
      ret_val = getAvailable(data_request_, available_reply_);
    }
  break;
  case rGetAvailableSynMobSea:
    if(nullptr != data_request_ && nullptr != available_reply_) {
      ret_val = getAvailableSynMobSea(data_request_, available_reply_);
    }
  break;
  case rGetOceanById:
    if(nullptr != simpledata_request_ && nullptr != data_reply_) {
      ret_val = getById(simpledata_request_, data_reply_);
    }
  break;
  default:
  break;
  }
  if(false == ret_val) {
    //TODO
  }
  int cur =  ttt.elapsed();
  if(1000 < cur) {
    warning_log << QString::fromUtf8("Время обработки запроса GetValueOnStation: %1 мсек").arg(cur);
  }
  // debug_log<<"поток "<< this <<" завершил работу(run)";
  emit finished(this);
}

//куча значений на одном горизонте - по дескриптору и  времени
bool OceanDataService::getManyOceanDataField(const DataRequest* req, ValueReply* resp)
{
  auto errFunc = [resp](QString err)
  {
    error_log << err;
    resp->set_result(false);
    resp->set_comment(err.toStdString());
    return false;
  };

  resp->set_result(false);

  QTime ttt; ttt.start();
  if ((req->meteo_descr_size() != 1 && req->meteo_descrname_size() != 1) ||
      !req->has_level_h() || !req->has_date_start() || !req->has_type_level() ) {
    return errFunc(QObject::tr("Полученные данные содержат одну или несколько ошибок."));
  }

  QString meteoDescrName = req->meteo_descrname_size() == 1?
        QString::fromStdString(req->meteo_descrname(0)):
        TMeteoDescriptor::instance()->name(req->meteo_descr(0));
  int meteoDescr = TMeteoDescriptor::instance()->descriptor(meteoDescrName);


  QDateTime dt = NosqlQuery::datetimeFromString(QString::fromStdString(req->date_start()));
  int levelH = req->level_h();
  int typeLevel = req->type_level();

  std::unique_ptr<Dbi> db(meteo::global::dbMeteo());
  if ( nullptr == db.get() )  {errFunc("");}
  auto query = db->queryptrByName("get_many_ocean_data");
  if(nullptr == query) {return errFunc("");}

  query->arg("dt", dt);
  query->arg("level", levelH);
  query->arg("level_type", typeLevel);
  query->arg("descrname", meteoDescrName);

  QString error;
  if(false == query->execInit( &error)){
    return errFunc(error);
  }

  while ( true == query->next()) {
    const DbiEntry& doc = query->entry();
    FullValue* val = resp->add_values();
    val->set_date(req->date_start());     // FIXME должно быть одним на весь набор данных
    val->set_meteo_descr(meteoDescr);        // FIXME должно быть одним на весь набор данных

    val->set_id(doc.valueOid("id").toStdString());
    auto coordinates = doc.valueArray("coords");

    Point * p = val->mutable_point();
    coordinates.next();
    p->set_la(MnMath::deg2rad(coordinates.valueDouble()));
    coordinates.next();
    p->set_fi(MnMath::deg2rad(coordinates.valueDouble()));
    val->set_value(doc.valueDouble("param.value"));
    val->set_quality(doc.valueInt32("param.quality"));

    val->set_code(doc.valueString("param.code").toStdString());
  }

  resp->set_level(req->level_h());
  resp->set_type_level(req->type_level());
  resp->set_comment(msglog::kServerAnswerOK.arg("getManyOceanDataField").toUtf8().constData());
  resp->set_result(true);

  int cur =  ttt.elapsed();
  if(1000 < cur){
    warning_log << msglog::kServiceRequestTime.arg(query->query()).arg(cur);
  }
  return true;
}

bool OceanDataService::getAvailable(const DataRequest* req, OceanAvailableReply* resp)
{
  auto errFunc = [resp](QString err)
  {
    error_log << err;
    resp->set_result(false);
    resp->set_comment(err.toStdString());
    return false;
  };

  resp->set_result(false);

  QTime ttt; ttt.start();
  if(false == req->has_date_start()|| 0 == req->type_size()) {
    return errFunc(QObject::tr("Запрос заполнен не полностью"));
  }

  QDateTime dtEnd = NosqlQuery::datetimeFromString(req->date_start());
  QDateTime dtStart = dtEnd.addSecs(-60*60);

  std::unique_ptr<Dbi> db(meteo::global::dbMeteo());
  if ( nullptr == db.get() )  {errFunc("");}
  auto query = db->queryptrByName("get_available_ocean");
  if(nullptr == query) {return errFunc("");}

  query->arg("start_dt",dtStart);
  query->arg("end_dt",dtEnd);
  QString error;
  if(false == query->execInit( &error)){
    return errFunc(error);
  }
  while ( true == query->next()) {
    const DbiEntry& doc = query->entry();
    QString station = doc.valueString("station");
    QDateTime dt = doc.valueDt("dt");
    GeoPoint coords = doc.valueGeo("location");
    int maxlvl = round(doc.valueDouble("maxlvl"));
    double count = doc.valueDouble("count");

    OceanAvailablePoint* pnt = resp->add_point();
    pnt->set_id      (station.toStdString()); //TODO убрать или нормально заполнять
    pnt->set_count   (count);
    pnt->set_maxlevel(maxlvl);
    pnt->set_lat     (coords.lat());
    pnt->set_lon     (coords.lon());
    pnt->set_name    (station.toStdString());
    pnt->set_dt      (dt.toString(Qt::ISODate).toStdString());
  }

  resp->set_result(true);
  debug_log << "Время выполнения OceanDataService::getAvailable :" << ttt.elapsed();
  return true;
}

bool OceanDataService::getAvailableSynMobSea(const DataRequest *req, OceanAvailableReply *resp)
{
  auto errFunc = [resp](QString err)
  {
    error_log << err;
    resp->set_result(false);
    resp->set_comment(err.toStdString());
    return false;
  };

  resp->set_result(false);

  QTime ttt; ttt.start();
  if(false == req->has_date_start()) {
    return errFunc(QObject::tr("Запрос заполнен не полностью"));
  }

  QDateTime dtEnd = NosqlQuery::datetimeFromString(req->date_start());
  QDateTime dtStart = dtEnd.addSecs(-60*60);

  std::unique_ptr<Dbi> db(meteo::global::dbMeteo());
  if ( nullptr == db.get() )  {errFunc("");}
  auto query = db->queryptrByName("get_available_synmobsea");
  if(nullptr == query) {return errFunc("");}

  query->arg("start_dt",dtStart);
  query->arg("end_dt",dtEnd);
  if ( 0 != req->meteo_descrname_size() ) {
    query->arg("descrname",req->meteo_descrname());
  }

  QString error;
  if(false == query->execInit( &error)){
    return errFunc(error);
  }

  while ( true == query->next()) {
    const DbiEntry& doc = query->entry();
    QString station = doc.valueString("station");
    QDateTime dt = doc.valueDt("dt");
    auto coordinatesArray = doc.valueArray("coord");
    coordinatesArray.next();
    double lon = coordinatesArray.valueDouble();
    coordinatesArray.next();
    double lat = coordinatesArray.valueDouble();
    double count = doc.valueDouble("count");

    OceanAvailablePoint* pnt = resp->add_point();
    pnt->set_id      (station.toStdString()); //TODO убрать или нормально заполнять
    pnt->set_count   (count);
    pnt->set_lat     (lat);
    pnt->set_lon     (lon);
    pnt->set_name    (station.toStdString());
    pnt->set_dt      (dt.toString(Qt::ISODate).toStdString());
  }

  resp->set_result(true);
  debug_log << "Время выполнения OceanDataService::getAvailableSynMobSea :" << ttt.elapsed();
  return true;
}

bool OceanDataService::getById(const SimpleDataRequest* req, DataReply* resp)
{
  auto errFunc = [resp](QString err)
  {
    error_log << err;
    resp->set_result(false);
    resp->set_comment(err.toStdString());
    return false;
  };

  resp->set_result(false);

  QTime ttt; ttt.start();
  if( ! req->has_text()  ||  req->id_size() != 1 ) {
    return errFunc(QObject::tr("Запрос заполнен не полностью"));
  }

  QDateTime dt = NosqlQuery::datetimeFromString(req->text());
  QString station = QString::fromStdString(req->id(0));

  std::unique_ptr<Dbi> db(meteo::global::dbMeteo());
  if ( nullptr == db.get() )  {errFunc("");}
  auto query = db->queryptrByName("get_ocean_by_station");
  if(nullptr == query) {return errFunc("");}
  query->arg("dt",dt);
  query->arg("station",station);
  QString error;
  if(false == query->execInit( &error)){
    return errFunc(error);
  }
  TMeteoData md;
  while ( true == query->next()) {
    const DbiEntry& doc = query->entry();
    auto geopoint = doc.valueGeo("coord");
    auto station = doc.valueString("station");
    md.setStation(station);
    md.setCoord( geopoint.latDeg(),  geopoint.lonDeg() );
    int level = doc.valueDouble("level");
    Array documents  = doc.valueArray("documents");
    TMeteoData& mdbuf = md.addChild();
    TMeteoParam bufparam = TMeteoParam("", level,control::QualityControl::NO_CONTROL);
    mdbuf.add("zn",bufparam);
    while (documents.next()){
      Document currentSubDocument;
      if (!documents.valueDocument(&currentSubDocument)){
        debug_log << "Error while reading document from array";
        continue;
      }
      QString descr = currentSubDocument.valueString("descrname");
      double value = currentSubDocument.valueDouble("value");
      QString code = currentSubDocument.valueString("code");
      int qual = currentSubDocument.valueInt32("quality");
      bufparam = TMeteoParam(code, value, static_cast<control::QualityControl>(qual));
      mdbuf.add(descr, bufparam);
    }
  }
  if ( false == req->asproto() ) {
    QByteArray buf;
    md >> buf;
    resp->set_result(true);
    resp->add_meteodata(buf.data(), buf.size());
  }
  else {
    auto proto = global::meteodata2proto(md);
    auto resproto = resp->add_meteodata_proto();
    resproto->CopyFrom(proto);
    resp->set_result(true);
  }

  debug_log << "Время выполнения OceanDataService::getById :" << ttt.elapsed();
  return true;
}

} // surf
} // meteo
