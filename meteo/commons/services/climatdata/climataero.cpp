#include "climataero.h"

#include <cross-commons/debug/tlog.h>
#include <meteo/commons/global/global.h>
#include <sql/nosql/nosqlquery.h>

#include <meteo/commons/zond/zond.h>
#include <commons/mathtools/mnmath.h>

#include <qmap.h>
#include <qprocess.h>
#include <qstring.h>
#include <qcoreapplication.h>

using namespace meteo;
using namespace climat;


ClimatAeroService::ClimatAeroService()
{
}

//! Значения за месяц по срокам
bool ClimatAeroService::getMonthValue(const meteo::climat::DataRequest* req,
				      meteo::climat::AeroLayerListReply* allres)
{
  QDateTime dtStart, dtEnd;
  QString err;
  
  bool ok = checkRequest(req, &dtStart, &dtEnd, &err);
  if (!ok) {
    allres->set_error(err.toStdString());
    return false;
  }

  std::unique_ptr<Dbi> db(meteo::global::dbClimat());
  if ( nullptr == db.get() ) {
    allres->set_error(QObject::tr("Не удается выполнить запрос в базу климатических данных").toStdString());
    error_log << QObject::tr("Не удается выполнить запрос в базу климатических данных");
    return false;
  }  

  std::unique_ptr<DbiQuery> query = db->queryptrByName("cl_get_aero");
  if (!query) {
    allres->set_error(QObject::tr("Не найден запрос 'cl_get_aero'").toStdString());
    error_log << QObject::tr("Не найден запрос 'cl_get_aero'");
    return false;
  }
  
  QStringList descrnames;
  for (int idx = 0; idx < req->descrname_size(); idx++) {
    descrnames << QString::fromStdString(req->descrname(idx));
  }
  
  query->arg("station", req->station())
    .arg("station_type", req->station_type());
  
  if (req->has_levtype()) {
    query->arg("level_type", req->levtype());
  }
  query->arg("descrname", descrnames);
  if (req->has_month()) {
    query->arg("month", req->month());
  }
  if (dtStart.isValid()) {
    query->arg("start_dt", dtStart);
  }
  if (dtEnd.isValid()) {
    query->arg("end_dt", dtEnd);
  }

  // var(query->query()); 

  QString error;
  if ( !query->execInit(&error) ) {
    allres->set_error(QObject::tr("Ошибка выполнения запроса").toStdString());
    error_log << QObject::tr("Ошибка выполнения запроса '%1'").arg(query->query());
    return false;
  }
   

  QMap<QString, AeroLayerReply*> repd; //дескр
  float level = -1;
  QMap<QPair<QString, int>, AeroLayerValues*> hv; //дескр, уровень
  
  while (query->next()) {
    const DbiEntry& doc = query->entry();

    //var(doc.jsonExtendedString());

    QString descrname = doc.valueString("descrname");
    if (descrname.isEmpty()) {
      continue;
    }
    
    if (!repd.contains(descrname)) {
      repd.insert(descrname, allres->add_all());
      repd[descrname]->set_descrname(descrname.toStdString());
      level = -1;
    }
   
    int dlevel = doc.valueDouble("level");
    QPair<QString, int> key(descrname, dlevel);
    if (!hv.contains(key)) {
      level = dlevel;
      hv.insert(key, repd[descrname]->add_val());
      hv[key]->set_layer(level); 
    }    

    AeroValuesList* vals = hv[key]->add_avglist();
    vals->set_avg(doc.valueDouble("value"));
    vals->set_date(doc.valueDt("dt").toString(Qt::ISODate).toStdString());    

    //debug_log <<descrname << dlevel << vals->avg() << vals->date();

  }
 
  //var(allres->DebugString());

  return ok;
}

bool ClimatAeroService::checkRequest(const meteo::climat::DataRequest* req,
				     QDateTime* dtStart, QDateTime* dtEnd, QString* err)
{
  if (req == 0 || req->descrname_size() == 0) {
    if (0 != err) {
      *err = "Invalid params";
    }
    return false;
  }
  
  QDate start;
  QDate end;

  if (req->has_date_start()) {
    start = QDate::fromString(QString::fromStdString(req->date_start()), Qt::ISODate);
  }
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
