#include "climatsurf.h"

#include <qmap.h>
#include <qprocess.h>
#include <qstring.h>
#include <qcoreapplication.h>

#include <cross-commons/debug/tlog.h>
#include <meteo/commons/global/global.h>
#include <commons/mathtools/mnmath.h>

#include <sql/nosql/nosqlquery.h>

using namespace meteo;
using namespace climat;

ClimatSurfService::ClimatSurfService()
{
}

bool ClimatSurfService::getDayValue(const meteo::climat::DataRequest* req,
				    meteo::climat::SurfDayReply* res)
{    
  QString err;
  
  std::unique_ptr<Dbi> db(meteo::global::dbClimat());
  if ( nullptr == db.get() ) {
    res->set_error(QObject::tr("Не удается выполнить запрос в базу климатических данных").toStdString());
    error_log << QObject::tr("Не удается выполнить запрос в базу климатических данных");
    return false;
  }  
  
  std::unique_ptr<DbiQuery> query = db->queryptrByName("cl_get_day");
  if (!query) {
    res->set_error(QObject::tr("Не найден запрос 'cl_get_day'").toStdString());
    error_log << QObject::tr("Не найден запрос 'cl_get_day'");
    return false;
  }

  bool ok = fillMatch(query.get(), req, &err);
  if (!ok) {
    res->set_error(err.toStdString());
    return false;
  }

  //var(query->query());
  
  QString error;
  if ( !query->execInit(&error) ) {
    res->set_error(QObject::tr("Ошибка выполнения запроса").toStdString());
    error_log << QObject::tr("Ошибка выполнения запроса '%1'").arg(query->query());
    return false;
  }
 
  res->set_descrname(req->descrname(0));
  QMap<int, SurfDayValues*> repd;//месяц, данные
  
  while (query->next()) {
    const DbiEntry& doc = query->entry();
    
    int month = doc.valueInt32("month");
    
    if (!repd.contains(month)) {
      repd.insert(month, res->add_val());
      repd[month]->set_month(month);
    }
    
    DayValuesList* mval = repd[month]->add_avglist();
    mval->set_year(doc.valueInt32("year"));
    if (doc.hasField("value")) {
      // Array arr = doc.valueArray("value");
      // while(arr.next()) {
      //   mval->add_avg(arr.valueDouble());
      // }
      QStringList arr = doc.valueString("value").remove(QRegExp("[{}]")).split(",");
      for (auto val : arr) {
	mval->add_avg(val.toDouble());
      }
    }
    if (doc.hasField("day")) {
      // Array arr = doc.valueArray("day");
      // while(arr.next()) {
      // 	mval->add_day(arr.valueDouble());
      // }
      
      QStringList arr = doc.valueString("day").remove(QRegExp("[{}]")).split(",");
      for (auto val : arr) {
      	mval->add_day(val.toDouble());
      }
            
    }
  }
  
  return true;
}

//! Значения за месяц по срокам
bool ClimatSurfService::getMonthValue(const meteo::climat::DataRequest* req,
				      meteo::climat::SurfMonthReply* res)
{
  QString err;

  std::unique_ptr<Dbi> db(meteo::global::dbClimat());
  if ( nullptr == db.get() ) {
    res->set_error(QObject::tr("Не удается выполнить запрос в базу климатических данных").toStdString());
    error_log << QObject::tr("Не удается выполнить запрос в базу климатических данных");
    return false;
  }  
  
  std::unique_ptr<DbiQuery> query = db->queryptrByName("cl_get_month");
  if (!query) {
    res->set_error(QObject::tr("Не найден запрос 'cl_get_month'").toStdString());
    error_log << QObject::tr("Не найден запрос 'cl_get_month'");
    return false;
  }

    bool ok = fillMatch(query.get(), req, &err);
  if (!ok) {
    res->set_error(err.toStdString());
    return false;
  }

  //var(query->query());
  
  QString error;
  if ( !query->execInit(&error) ) {
    res->set_error(QObject::tr("Ошибка выполнения запроса").toStdString());
    error_log << QObject::tr("Ошибка выполнения запроса '%1'").arg(query->query());
    return false;
  }

  res->set_descrname(req->descrname(0));

  QMap<int, SurfHourValues*> repd; //месяц, данные
  while (query->next()) {
    const DbiEntry& doc = query->entry();
    int month = doc.valueInt32("month");
    
    if (!repd.contains(month)) {
      repd.insert(month, res->add_val());
      repd[month]->set_month(month);
    }
    
    ValuesList* val = repd[month]->add_avglist();
    val->set_hour(doc.valueInt32("hour"));

    QStringList arr = doc.valueString("value").remove(QRegExp("[{}]")).split(",");
    for (auto aval : arr) {
      val->add_avg(aval.toDouble());
    }
    // Array arr = doc.valueArray("value");
    // while(arr.next()) {
    //   val->add_avg(arr.valueDouble());
    // }
  }

  return true;
}


//! Все значения за месяц
bool ClimatSurfService::getAvgMonthValue(const meteo::climat::DataRequest* req,
					 meteo::climat::SurfMonthAvgReply* res)
{
  QString err;
  
  std::unique_ptr<Dbi> db(meteo::global::dbClimat());
  if ( nullptr == db.get() ) {
    res->set_error(QObject::tr("Не удается выполнить запрос в базу климатических данных").toStdString());
    error_log << QObject::tr("Не удается выполнить запрос в базу климатических данных");
    return false;
  }  
  
  std::unique_ptr<DbiQuery> query = db->queryptrByName("cl_get_avg_month");
  if (!query) {
    res->set_error(QObject::tr("Не найден запрос 'cl_get_avg_month'").toStdString());
    error_log << QObject::tr("Не найден запрос 'cl_get_avg_month'");
    return false;
  }

  bool ok = fillMatch(query.get(), req, &err);
  if (!ok) {
    res->set_error(err.toStdString());
    return false;
  }

  //var(query->query());
  
  QString error;
  if ( !query->execInit(&error) ) {
    res->set_error(QObject::tr("Ошибка выполнения запроса").toStdString());
    error_log << QObject::tr("Ошибка выполнения запроса '%1'").arg(query->query());
    return false;
  }

  
  res->set_descrname(req->descrname(0));
  while (query->next()) {
    const DbiEntry& doc = query->entry();
    SurfAvgValues* val = res->add_val();
    val->set_month(doc.valueInt32("month"));
    
    // Array arr = doc.valueArray("value");
    // while(arr.next()) {
    //   val->add_avg(arr.valueDouble());
    // }
    QStringList arr = doc.valueString("value").remove(QRegExp("[{}]")).split(",");
    for (auto aval : arr) {
      val->add_avg(aval.toDouble());
    }
  }

  return true;
}

//! Значения за месяц по годам
bool  ClimatSurfService::getYearValue(const meteo::climat::DataRequest* req,
				      meteo::climat::SurfYearReply* res)
{
  QString err;

  std::unique_ptr<Dbi> db(meteo::global::dbClimat());
  if ( nullptr == db.get() ) {
    res->set_error(QObject::tr("Не удается выполнить запрос в базу климатических данных").toStdString());
    error_log << QObject::tr("Не удается выполнить запрос в базу климатических данных");
    return false;
  }  
  
  std::unique_ptr<DbiQuery> query = db->queryptrByName("cl_get_year");
  if (!query) {
    res->set_error(QObject::tr("Не найден запрос 'cl_get_year'").toStdString());
    error_log << QObject::tr("Не найден запрос 'cl_get_year'");
    return false;
  }

  bool ok = fillMatch(query.get(), req, &err);
  if (!ok) {
    res->set_error(err.toStdString());
    return false;
  }

  //var(query->query());
  
  QString error;
  if ( !query->execInit(&error) ) {
    res->set_error(QObject::tr("Ошибка выполнения запроса").toStdString());
    error_log << QObject::tr("Ошибка выполнения запроса '%1'").arg(query->query());
    return false;
  }
  
  res->set_descrname(req->descrname(0));
  while (query->next()) {
    const DbiEntry& doc = query->entry();
    SurfMonthValues* val = res->add_val();
    val->set_month(doc.valueInt32("month"));
    
    Array arr = doc.valueArray("value");
    //var(arr.jsonString());
    while(arr.next()) {
      Document param;
      arr.valueDocument(&param);
      YearValuesList* avglist = val->add_avglist();
      avglist->set_avg(param.valueDouble("value"));
      avglist->set_year(param.valueDouble("year"));
      if (param.hasField("hour")) {
  	avglist->set_hour(param.valueDouble("hour"));
      }
    }
  }

  return true;
}

// bool ClimatSurfService::getAllValue(const meteo::climat::DataRequest* req,
// 				    meteo::climat::SurfMonthAllReply* res)
// {
//   Q_UNUSED(req);
//   Q_UNUSED(res);

//   // QString dtStart, dtEnd, err;
  
//   // bool ok = checkRequest(req, &dtStart, &dtEnd, &err);
//   // if (!ok) {
//   //   res->set_error(err.toStdString());
//   //   return false;
//   // }
  
//   // res->set_descr(req->descr(0));
//   // res->set_adddescr(req->adddescr(0));

//   // QString query = QString("SELECT * FROM get_surf_hour(%1, %2, %3, %4, %5)")
//   //   .arg(req->station())
//   //   .arg(req->descr(0))
//   //   .arg(req->adddescr(0))
//   //   .arg(dtStart)
//   //   .arg(dtEnd);
        
//   // var(query);
  
//   // if (!db()->ExecQuery(query)) {
//   //   error_log<<"Error read from db"<<db()->ErrorMessage();
//   //   res->set_error("Error read from db");
//   //   return false;
//   // }

//   // if (db()->RecordCount() == 0) {
//   //   res->set_error(QObject::tr("Нет данных").toStdString());
//   //   return true;
//   // }

//   // int month = -1;
//   // SurfMonthAllValues* hv = 0;

//   // for (int i = 0, sz =db()->RecordCount(); i < sz; ++i) {
//   //   int dmonth = db()->GetCell( i, 0 )->AsInt();
//   //   if (month != dmonth || hv == 0) {
//   //     month = dmonth;
//   //     hv = res->add_val();
//   //     hv->set_month(dmonth);
//   //   }
//   //   SurfAllValuesList* vals = hv->add_avglist();
//   //   vals->set_day(db()->GetCell( i, 1 )->AsInt());
//   //   vals->set_hour(db()->GetCell( i, 2 )->AsInt());
//   //   vals->set_year(db()->GetCell( i, 3 )->AsInt());
//   //   vals->set_avg(db()->GetCell( i, 4 )->AsDouble());
//   // }

//   return true;
// }

bool  ClimatSurfService::getDecadeValue(const meteo::climat::DataRequest* req,
					meteo::climat::SurfDecadeReply* res)
{ 
  QString err;
  std::unique_ptr<Dbi> db(meteo::global::dbClimat());
  if ( nullptr == db.get() ) {
    res->set_error(QObject::tr("Не удается выполнить запрос в базу климатических данных").toStdString());
    error_log << QObject::tr("Не удается выполнить запрос в базу климатических данных");
    return false;
  }  
  
  std::unique_ptr<DbiQuery> query = db->queryptrByName("cl_get_decade");
  if (!query) {
    res->set_error(QObject::tr("Не найден запрос 'cl_get_decade'").toStdString());
    error_log << QObject::tr("Не найден запрос 'cl_get_decade'");
    return false;
  }
  
  bool ok = fillMatch(query.get(), req, &err);
  if (!ok) {
    res->set_error(err.toStdString());
    return false;
  }

  //var(query->query());
    
  QString error;
  if ( !query->execInit(&error) ) {
    res->set_error(QObject::tr("Ошибка выполнения запроса").toStdString());
    error_log << QObject::tr("Ошибка выполнения запроса '%1'").arg(query->query());
    return false;
  }
   
  res->set_descrname(req->descrname(0));
  while (query->next()) {
    const DbiEntry& doc =  query->entry();
    SurfDecadeValues* val = res->add_val();
    val->set_month(doc.valueInt32("month"));
    
    Array arr = doc.valueArray("value");
    while(arr.next()) {
      Document param;
      arr.valueDocument(&param);
      DecadeValuesList* mval = val->add_avglist();
      mval->set_avg(param.valueDouble("value"));
      mval->set_year(param.valueDouble("year"));
      mval->set_decade(param.valueDouble("decade"));
    }
  }

  return true;
}

bool ClimatSurfService::getAllValueList(const meteo::climat::DataRequest* req,
					meteo::climat::SurfAllListReply* allres)
{  
  QString err;

  std::unique_ptr<Dbi> db(meteo::global::dbClimat());
  if ( nullptr == db.get() ) {
    allres->set_error(QObject::tr("Не удается выполнить запрос в базу климатических данных").toStdString());
    error_log << QObject::tr("Не удается выполнить запрос в базу климатических данных");
    return false;
  }  
  
  std::unique_ptr<DbiQuery> query = db->queryptrByName("cl_get_surf_all");
  if (!query) {
    allres->set_error(QObject::tr("Не найден запрос 'cl_get_surf_all'").toStdString());
    error_log << QObject::tr("Не найден запрос 'cl_get_surf_all'");
    return false;
  }
  
  bool ok = fillMatch(query.get(), req, &err);
  if (!ok) {
    allres->set_error(err.toStdString());
    return false;
  }

  //var(query->query());
  
  QString error;
  if ( !query->execInit(&error) ) {
    allres->set_error(QObject::tr("Ошибка выполнения запроса").toStdString());
    error_log << QObject::tr("Ошибка выполнения запроса '%1'").arg(query->query());
    return false;
  }

  
  QMap<QString, SurfMonthAllReply*> repd;//дескриптор, данные
  while (query->next()) {
    const DbiEntry& doc = query->entry();
        
    QString descrname = doc.valueString("descrname");
    if (descrname.isEmpty()) {
      continue;
    }
    
    if (!repd.contains(descrname)) {
      repd.insert(descrname, allres->add_all());
      repd[descrname]->set_descrname(descrname.toStdString());
    }
    
    SurfMonthAllValues* mval = repd[descrname]->add_val();
    mval->set_month(doc.valueInt32("month"));

    if (doc.hasField("value")) {
      Array arr = doc.valueArray("value");
      //var(arr.jsonString());
      while(arr.next()) {
	Document param;
	arr.valueDocument(&param);
	SurfAllValuesList* avglist = mval->add_avglist();
	avglist->set_avg(param.valueDouble("value"));
	avglist->set_year(param.valueInt32("year"));
	avglist->set_day(param.valueInt32("day"));
	avglist->set_hour(param.valueInt32("hour"));
      }
    }
  }
  
  return true;
}


bool ClimatSurfService::getDateValue(const meteo::climat::DataRequest* req,
				     meteo::climat::SurfDateReply* res)
{ 
  if (!req->has_collection() || !req->has_limit() || !req->has_less_then() ||
      !req->has_lmonth_start() || !req->has_lmonth_end()) {
    res->set_error("Invalid params");
    return false;
  }

  QString err;
  std::unique_ptr<Dbi> db(meteo::global::dbClimat());
  if ( nullptr == db.get() ) {
    res->set_error(QObject::tr("Не удается выполнить запрос в базу климатических данных").toStdString());
    error_log << QObject::tr("Не удается выполнить запрос в базу климатических данных");
    return false;
  }  
  
  std::unique_ptr<DbiQuery> query = db->queryptrByName("cl_get_date");
  if (!query) {  
    err = QObject::tr("Не найден запрос 'cl_get_date'");
    res->set_error(err.toStdString());
    error_log << err;
    return false;
  }

  bool ok = fillMatch(query.get(), req, &err);
  if (!ok) {
    res->set_error(err.toStdString());
    return false;
  }

  if (req->less_then()) {
    query->arg("condition", "$lt");
  } else {
    query->arg("condition", "$gt");
  }
  query->arg("collection", req->collection())
        .arg("vlimit",  req->limit())
        .arg("month_start", req->lmonth_start())
        .arg("month_end",   req->lmonth_end());

  QString error;
  if ( !query->execInit(&error) ) {
    res->set_error(QObject::tr("Ошибка выполнения запроса").toStdString());
    error_log << QObject::tr("Ошибка выполнения запроса '%1'").arg(query->query());
    return false;
  }   
 
  res->set_descrname(req->descrname(0));
  QMap<int, SurfDateValues*> yearval;

  //var(query->query());
  
  while (query->next()) {
    const DbiEntry& doc =  query->entry();
    Array arrs = doc.valueArray("first");
    //var(arrs.jsonString());
    while(arrs.next()) {
      Document param;
      arrs.valueDocument(&param);
      int year = param.valueInt32("year");
      if (!yearval.contains(year)) {
  	yearval.insert(year, res->add_val());
      }
      yearval[year]->set_start_date(param.valueDt("date").toString(Qt::ISODate).toStdString());
    }
    
    Array arre = doc.valueArray("last");
    while(arre.next()) {
      Document param;
      arre.valueDocument(&param);
      int year = param.valueInt32("year");
      if (!yearval.contains(year)) {
  	yearval.insert(year, res->add_val());
      }
      yearval[year]->set_end_date(param.valueDt("date").toString(Qt::ISODate).toStdString());
    }    
  }  
   
  return true;
}

bool ClimatSurfService::checkRequest(const meteo::climat::DataRequest* req,
				     QDateTime* dtStart, QDateTime* dtEnd, QString* err)
{
  if (req == 0 || req->descrname_size() == 0) {
    if (0 != err) {
      *err = "Invalid params";
    }
    return false;
  }
  
  QDate start;
  if (req->has_date_start()) {
    start = QDate::fromString(QString::fromStdString(req->date_start()), Qt::ISODate);
  }
  QDate end;
  if (req->has_date_end()) {
    end = QDate::fromString(QString::fromStdString(req->date_end()), Qt::ISODate);
  }

  if (0 != dtStart) {
    *dtStart = QDateTime(start, QTime(0,0));
  }
  if (0 != dtEnd) {
    *dtEnd = QDateTime(end, QTime(23,59));
  }

  return true;
}


bool ClimatSurfService::fillMatch(meteo::DbiQuery* query, const meteo::climat::DataRequest* req, QString* err)
{ 
  if (nullptr == err || nullptr == query) {
    return false;
  }
    
  QDateTime dtStart, dtEnd;
  
  bool ok = checkRequest(req, &dtStart, &dtEnd, err);
  if (!ok) {
    return false;
  }

  query->arg("station", req->station())
    .arg("station_type", req->station_type())
    .arg("descrname", req->descrname());
  if (req->has_month()) {
    query->arg("month", req->month());
  }
  if (dtStart.isValid()) {
    query->arg("start_dt", dtStart);
  }
  if (dtEnd.isValid()) {
    query->arg("end_dt", dtEnd);
  }

  return true;
}
