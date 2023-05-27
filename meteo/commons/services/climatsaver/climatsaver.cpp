#include "climatsaver.h"

#include <sql/nosql/nosqlquery.h>
#include <sql/dbi/dbi.h>
#include <sql/dbi/dbiquery.h>

#include <cross-commons/debug/tlog.h>
#include <meteo/commons/rpc/rpc.h>
#include <meteo/commons/global/global.h>


#include <meteo/commons/proto/locsettings.pb.h>
#include <meteo/commons/proto/surface_service.pb.h>
#include <commons/textproto/tprototext.h>
#include <commons/meteo_data/complexmeteo.h>
#include <meteo/commons/proto/sprinf.pb.h>
#include <meteo/commons/settings/settings.h>
#include <sql/psql/psql.h>

#include <qfile.h>
#include <qsettings.h>

using namespace meteo::climat;

const int kTimeout = 1000000;


const int kDefaultPeriod = 10; //дней


ClimatSaver::ClimatSaver(const QDate& start, const QDate& end):
  _dtStart(start),
  _dtEnd(end)
{
  _surf = std::make_unique<SurfSaver>(SurfSaver());
  _aero = std::make_unique<AeroSaver>(AeroSaver());  
}


ClimatSaver::ClimatSaver(const QDate& start, const QDate& end,
			 std::unique_ptr<SurfSaver> surf,
			 std::unique_ptr<AeroSaver> aero):
  _dtStart(start),
  _dtEnd(end),
  _surf(std::move(surf)),
  _aero(std::move(aero))
{
}

ClimatSaver::~ClimatSaver()
{
}

//! Дополнение климатической базы новыми данными
bool ClimatSaver::updateClimatData(rpc::Channel* chan)
{ 
  const meteo::climat::ClimatSaveProp& opt = meteo::global::Settings::instance()->climat();
   
  if (!opt.IsInitialized()) {
    error_log << QObject::tr("Ошибка получения настроек");
    return false;
  }

  //var(opt.DebugString());

  for (int i = 0; i < opt.stations_size(); i++) {
    sprinf::MeteostationType cltype = sprinf::kStationUnk;
    if (opt.stations(i).has_cltype()) {
      cltype = opt.stations(i).cltype();
    }
    QDateTime first, last; //дата первой и последней записи
    updateClimatData(chan, QString::fromStdString(opt.stations(i).station().station()),
		     sprinf::MeteostationType(opt.stations(i).station().type()), cltype, &first, &last);
    updateStation(opt.stations(i), first, last);
  }

  return true;
}


//! Дополнение климатической базы новыми данными для станции с индексом index и типом type. cltype - тип формируемых данных климата
bool ClimatSaver::updateClimatData(rpc::Channel* chan, const QString& station, sprinf::MeteostationType type,
				   sprinf::MeteostationType cltype, QDateTime* first, QDateTime* last)
{
  if (nullptr == chan || station.isEmpty()) {
    return false;
  }

  if (!_dtEnd.isValid()) {
    _dtEnd = QDate::currentDate();
  }

  QDateTime afirst, alast;

  //var(station);
  // var(type);
  
  if (cltype == sprinf::kStationUnk || cltype == sprinf::kStationSynop) {
    QDate start = _dtStart;
    if (!start.isValid()) {
      start = findDtStart(station, type, _surf->dtCheckTableName());
    }
    
    _surf->updateSurfData(chan, station, type, start, _dtEnd);
    *first = _surf->first();
    *last = _surf->last();
  }

  
  if (cltype == sprinf::kStationUnk || cltype == sprinf::kStationAero) {
    QDate start = _dtStart;
    if (!start.isValid()) {
      start = findDtStart(station, type, _aero->dtCheckTableName());
    }
    
    _aero->updateAeroData(chan, station, type, start, _dtEnd);
    afirst = _aero->first();
    alast  = _aero->last();
  }
  
  if (!first->isValid() || (afirst.isValid() && afirst < *first)) {
    *first = afirst;
  }
  if (!last->isValid()  || (alast.isValid() && alast > *last)) {
    *last = alast;
  }

  return true;
}


//! Определение даты, с которой запрашивать данные для сохранения в климатической БД
QDate ClimatSaver::findDtStart(const QString& station, sprinf::MeteostationType type,
			       const QString& collection)
{
  std::unique_ptr<Dbi> db(meteo::global::dbClimat());
  if ( nullptr == db.get() ) {
    error_log << QObject::tr("Не удается выполнить запрос в базу климатических данных");
    return QDate::currentDate().addDays(-kDefaultPeriod);
  }

  std::unique_ptr<DbiQuery> query = db->queryptrByName("cl_lastdate");
  if ( !query ) {
    error_log << QObject::tr("Не найден запрос cl_lastdate");
    return QDate::currentDate().addDays(-kDefaultPeriod);
  }

  query->argWithoutQuotes("tablename", collection);
  query->arg("station", station);
  query->arg("station_type", type);

  //debug_log << query->query();
  
  QString error;
  if ( !query->execInit(&error) ) {
    error_log << QObject::tr("Не удается выполнить запрос в базу климатических данных") << error;
    return QDate::currentDate().addDays(-kDefaultPeriod);
  }
  
  if (false == query->next()) {
    return QDate::currentDate().addDays(-kDefaultPeriod);
  }

  QDate start;
  const DbiEntry& doc = query->entry();
  bool ok;
  if (doc.hasField("dt")) {
    start = doc.valueDt("dt", &ok).date();
  } else {
    start = QDate::currentDate().addDays(-kDefaultPeriod);
  }

  return start;
}


void ClimatSaver::updateStation(const ClimatStation& station, const QDateTime& first, const QDateTime& last)
{
  if (!first.isValid() || !last.isValid()) {
    return;
  }

  std::unique_ptr<Dbi> db(meteo::global::dbClimat());
  if ( nullptr == db.get() ) {
    error_log << QObject::tr("Не удается выполнить запрос в базу климатических данных");
    return;
  }

  std::unique_ptr<DbiQuery> query = db->queryptrByName("cl_upd_station");
  if ( !query ) {
    error_log << QObject::tr("Не найден запрос cl_upd_station");
    return;
  }

  // QString strq = meteo::global::kMongoQueriesNew["cl_upd_station"];
  // if (strq.isEmpty()) {
  //   return;
  // }

  // NosqlQuery query;
  // query.setQuery(strq)
  query->arg("station", station.station().station());
  query->arg("type", station.station().type());
  query->arg("cltype", station.cltype());
  query->arg("utc", station.utc());
  query->arg("name_en", station.station().name().international());
  query->arg("name_ru", station.station().name().rus());
  query->arg("location", GeoPoint(station.station().position().lat_radian(), station.station().position().lon_radian(),
				  station.station().position().height_meters()));
  query->arg("country", station.station().country().number());
  query->arg("first", first);
  query->arg("last", last);
      
  if (!query->exec()) {
    error_log << QObject::tr("Не удалось выполнить запрос = %1")
      .arg(query->query());
    //  ok = false;
  }
  
}


