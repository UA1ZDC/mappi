#include "tsurface_data_service.h"
#include <commons/meteo_data/tmeteodescr.h>

#include <qmap.h>
#include <qprocess.h>
#include <qstring.h>
#include <qcoreapplication.h>
#include <qjsondocument.h>
#include <qjsonarray.h>
#include <qjsonobject.h>

#include <cross-commons/debug/tlog.h>

#include <sql/nosql/nosqlquery.h>

#include <commons/meteo_data/meteo_data.h>
#include <commons/mathtools/mnmath.h>

#include <meteo/commons/global/global.h>

namespace meteo {
namespace surf {


static const QString coll_meteodata = "meteodata";
static const std::string msgQueryExecutionError = "Ошибка при выполнении запроса в базу данных";

TSurfaceDataService::TSurfaceDataService(const DataRequest* req, ValueReply* res, Closure* done)
  : TBaseDataService(done), req_(req), res_(res) { }

TSurfaceDataService::~TSurfaceDataService() { }

void TSurfaceDataService::run()
{
  QTime ttt; ttt.start();

  getValue(req_, res_);

  int cur = ttt.elapsed();
  if(1000 < cur){
    warning_log << QObject::tr("Время обработки запроса GetValueOnStation: %1 мсек").arg(cur);
  }

  emit finished(this);
}


bool TSurfaceDataService::getValue(const DataRequest* req, ValueReply* res)
{
  QTime ttt; ttt.start();
  if ( 0 == req->type_size() ) {
    error_log << QObject::tr("Запрос заполнен не полностью");
    res->set_result(false);
    res->set_comment( QObject::tr("Запрос заполнен не полностью").toStdString() );
    return false;
  }
  int datatype = req->type(0);

  res->set_type(datatype);
  //if(1 == req->meteo_descr_size()) { res->add_meteo_descr(req->meteo_descr(0)); }

  //одно значение у земли - по индексу, дескриптору и  времени
  if(1 == req->station_size() && req->has_date_start() &&
     false == req->has_date_end() && 1 == req->meteo_descr_size())
  {
    return getOneSurfaceData1(req,res);
  }

  // одно или несколько значений у земли - по индексу(ам ), дескриптору и  времени
  if(1 == req->station_size() && req->has_date_start() &&
     false == req->has_date_end() && 0 < req->meteo_descr_size())
  {
    return getManySurfaceData3(req,res);
  }

  //куча значений у земли - по дескриптору и  времени
  if(0 == req->station_size() && req->has_date_start() && false == req->has_date_end() ) {
    int descr_size = req->meteo_descr_size();
    if(1 == descr_size) {
      return getManySurfaceData1(req,res);
    }
    else if(1 < descr_size) {
      error_log << QObject::tr("Для получения значений набора величин по множеству станций необходимо использовать другой метод");
    }
  }

  // одно или несколько значений у земли - по индексу, дескриптору и интервалу времени  времени
  if(1 == req->station_size() && req->has_date_start() &&
     req->has_date_end() && 1 == req->meteo_descr_size())
  {
    return getManySurfaceData2(req,res);
  }
  return false;
}

// одно или несколько значений у земли - по индексу, дескриптору и интервалу времени  времени

bool TSurfaceDataService::getManySurfaceData2(const meteo::surf::DataRequest* req,
                                              meteo::surf::ValueReply* resp)
{
  auto return_func = [resp](QString err)
  {
    error_log << err;
    resp->set_result(false);
    resp->set_comment(err.toStdString());
    return false;
  };

  resp->set_result(false);

  if (!req->has_date_start() ||
      !req->has_date_end() ||
      req->station_size() != 1 ||
      req->meteo_descr_size() != 1){
    return return_func(QObject::tr("Запрос заполнен не полностью"));
  }

  QDateTime dtStart = NosqlQuery::datetimeFromString(req->date_start());
  QDateTime dtEnd = NosqlQuery::datetimeFromString(req->date_end());
  QString station = QString::fromStdString(req->station(0));
  QString meteoDescriptorName = TMeteoDescriptor::instance()->name(req->meteo_descr(0));

  QString queryName = "t_surface_data_service_get_many_surface_data_2";

  std::unique_ptr<Dbi> db(meteo::global::dbMeteo());
  if ( nullptr == db.get() )  {return return_func("");}
  auto query = db->queryptrByName(queryName);
  if(nullptr == query) {return return_func("");}

  query->arg("station",station);
  query->arg("descrname",meteoDescriptorName);
  query->arg("start_dt", dtStart);
  query->arg("end_dt",  dtEnd);

  QString error;
  if(false == query->execInit( &error)){
    return return_func(error);
  }

  while ( true == query->next() ) {
    const DbiEntry& doc = query->entry();
    QDateTime dt = doc.valueDt("dt");
    QString code = doc.valueString("param.code");
    double value = doc.valueDouble("param.value");
    int qualiry = doc.valueInt32("param.quality");
    QString descriptorName = doc.valueString("param.descrname");
    int descriptor = TMeteoDescriptor::instance()->descriptor(descriptorName);
    FullValue * afval = resp->add_values();

    afval->set_date(dt.toString(Qt::ISODate).toStdString());
    afval->set_code( code.toStdString() );
    afval->set_value(value);
    afval->set_quality(qualiry);
    afval->set_meteo_descr(descriptor);
    afval->mutable_point()->set_index( station.toStdString() );
  }
  resp->set_result(true);
  return false;
}

//куча значений у земли - по дескриптору и  времени
bool TSurfaceDataService::getManySurfaceData1(const DataRequest* req, ValueReply* resp)
{
  auto return_func = [resp](QString err)
  {
    error_log << err;
    resp->set_result(false);
    resp->set_comment(err.toStdString());
    return false;
  };

  resp->set_result(false);

  QTime ttt; ttt.elapsed();
  if (!req->has_level_p() ||
      !req->has_type_level() ||
      !req->has_date_start() ||
      (req->meteo_descrname_size() != 1 && req->meteo_descr_size() != 1)){
    return return_func(QObject::tr("Некорректно заполненый запрос: отсутствует часть обязательных полей"));
  }

  QTime functionWorkingTime; functionWorkingTime.start();
  QDateTime dt = NosqlQuery::datetimeFromString(req->date_start());
  int levelType = req->type_level();
  int level = req->level_p();

  QString descrname = req->meteo_descrname_size() == 1?
        QString::fromStdString(req->meteo_descrname(0)):
        TMeteoDescriptor::instance()->name(req->meteo_descr(0));

  QString queryName = "t_surface_data_service_get_many_surface_data_1";
  std::unique_ptr<Dbi> db(meteo::global::dbMeteo());
  if ( nullptr == db.get() )  {return return_func("");}
  auto query = db->queryptrByName(queryName);
  if(nullptr == query) {return return_func("");}

  query->arg("level_type",levelType);
  query->arg("descrname",descrname);
  query->arg("dt", dt);
  query->arg("level",  level);

  QString error;
  if(false == query->execInit( &error)){
    return return_func(error);
  }

  while ( true == query->next() ) {
    const DbiEntry& doc = query->entry();
    FullValue* afval = resp->add_values();
    afval->set_date(req->date_start());
    meteo::surf::Point* pnt = afval->mutable_point();

    QString id = doc.valueOid("_id");
    afval->set_id(id.toStdString());
    QString station = doc.valueString("station");
    pnt->set_index(station.toStdString());
    Array coordinates = doc.valueArray("location.coordinates");
    coordinates.next();
    pnt->set_la(coordinates.valueDouble());
    coordinates.next();
    pnt->set_fi(coordinates.valueDouble());
    afval->set_value(doc.valueDouble("param.value"));
    afval->set_quality(doc.valueInt32("param.quality"));
    afval->set_code(doc.valueString("param.code").toStdString());
    QString meteoDescriptorString = doc.valueString("param.descrname");
    int meteoDescrValue = TMeteoDescriptor::instance()->descriptor(meteoDescriptorString);
    afval->set_meteo_descr(meteoDescrValue);
  }

  resp->set_comment(msglog::kServerAnswerOK.arg("getManySurfaceData1").toUtf8().constData());
  resp->set_result(true);
  int cur =  functionWorkingTime.elapsed();
  if(1000 < cur) { warning_log << msglog::kServiceRequestTime.arg(query->query()).arg(cur); }
  warning_log << "Время выполнения TSurfaceDataService::getManySurfaceData1 :" << ttt.elapsed();
  return true;
}

// одно или несколько значений у земли - по индексу(ам ), дескриптору и  времени
bool TSurfaceDataService::getManySurfaceData3(const meteo::surf::DataRequest* req,
                                              meteo::surf::ValueReply* resp)
{
  auto return_func = [resp](QString err)
  {
    error_log << err;
    resp->set_result(false);
    resp->set_comment(err.toStdString());
    return false;
  };

  resp->set_result(false);

  if ( req->station_size() != 1 ||
       req->meteo_descr_size() <= 0 ||
       ! req->has_date_start() ){
    return return_func(QObject::tr("Некорректно заполненый запрос: отсутствует часть обязательных полей"));
  }

  QString station = QString::fromStdString( req->station(0) );
  QStringList descriptorNames ;
  for (int i  =0, sz = req->meteo_descr_size(); i < sz; ++i){
    descriptorNames << TMeteoDescriptor::instance()->name(req->meteo_descr(i));
  }
  QDateTime dt = NosqlQuery::datetimeFromString(req->date_start());

  QString queryName = "t_surface_data_service_get_many_surface_data_3";
  std::unique_ptr<Dbi> db(meteo::global::dbMeteo());
  if ( nullptr == db.get() )  {return return_func("");}
  auto query = db->queryptrByName(queryName);
  if(nullptr == query) {return return_func("");}

  query->arg("station",station);
  query->arg("dt", dt);
  query->arg("level",  descriptorNames);

  QString error;
  if(false == query->execInit( &error)){
    return return_func(error);
  }

  while ( true == query->next() ) {
    const DbiEntry& doc = query->entry();
    double value = doc.valueDouble("param.value");
    int quality = doc.valueInt32("param.quality");
    QString code = doc.valueString("param.code");
    QString meteoDescriptorName = doc.valueString("param.descrname");
    int meteoDescriptor = TMeteoDescriptor::instance()->descriptor(meteoDescriptorName);
    FullValue * afval = resp->add_values();
    afval->set_date(req->date_start());
    afval->mutable_point()->set_index (station.toStdString());
    afval->set_value (value);
    afval->set_quality (quality);
    afval->set_code (code.toStdString());
    afval->set_meteo_descr ( meteoDescriptor );
  }
  resp->set_result(true);
  return true;

}

// одно значение у земли - по индексу, дескриптору и  времени
bool TSurfaceDataService::getOneSurfaceData1(const meteo::surf::DataRequest* req,
                                             meteo::surf::ValueReply* resp)
{
  auto return_func = [resp](QString err)
  {
    error_log << err;
    resp->set_result(false);
    resp->set_comment(err.toStdString());
    return false;
  };

  if (!req->has_date_start() ||
      req->station_size() != 1 ||
      (req->meteo_descr_size() != 1 && req->meteo_descrname_size() != 1)){
    return return_func(QObject::tr("Получен некорректный запрос. Отсуствует часть обязательных для заполнения полей."));
  }

  QDateTime dt = NosqlQuery::datetimeFromString(req->date_start());
  QString station = QString::fromStdString(req->station(0));
  QString meteoDescrName = req->meteo_descrname_size() == 1? QString::fromStdString(req->meteo_descrname(0)) :
                                                             TMeteoDescriptor::instance()->name(req->meteo_descr(0));
  QString queryName = "t_surface_data_service_get_one_surface_data";
  std::unique_ptr<Dbi> db(meteo::global::dbMeteo());
  if ( nullptr == db.get() )  {return return_func("");}
  auto query = db->queryptrByName(queryName);
  if(nullptr == query) {return return_func("");}


  query->arg("station",station);
  query->arg("dt", dt);
  query->arg("level",  meteoDescrName);

  QString error;
  if(false == query->execInit( &error)){
    return return_func(error);
  }

  while ( true == query->next() ) {
    const DbiEntry& doc = query->entry();
    int meteoDescr = TMeteoDescriptor::instance()->descriptor(doc.valueString("param.descrname"));
    double value = doc.valueDouble("param.value");
    int quality = doc.valueDouble("param.quality");

    FullValue * afval = resp->add_values();
    afval->set_date(req->date_start());
    afval->set_meteo_descr (meteoDescr);
    afval->set_value(value);
    afval->set_quality(quality);

    Array paramArray = doc.valueArray("location.coordinates");
    paramArray.next();
    double fi = paramArray.valueDouble();
    paramArray.next();
    double la = paramArray.valueDouble();
    int alt = doc.valueInt32("location.alt");
    QString station = doc.valueString("station");

    meteo::surf::Point* p = afval->mutable_point();
    p->set_fi(fi);
    p->set_la(la);
    p->set_height(alt);
    p->set_index(station.toStdString());
  }
  resp->set_result(true);
  return true;
}

}
}
