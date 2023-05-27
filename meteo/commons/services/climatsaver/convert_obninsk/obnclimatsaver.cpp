#include "obnclimatsaver.h"

#include <sql/nosql/nosqlquery.h>
#include <sql/dbi/dbi.h>
#include <sql/dbi/dbiquery.h>

#include <cross-commons/debug/tlog.h>
#include <meteo/commons/rpc/rpc.h>
#include <meteo/commons/global/global.h>

#include <sql/psql/psql.h>

#include <qfile.h>
#include <qsettings.h>

using namespace meteo::climat;

const int kTimeout = 1000000;

ObnClimatSaver::ObnClimatSaver(const QDate& start, const QDate& end, const QString& station,
			       std::shared_ptr<Psql> db):
  ClimatSaver(start, end,
	      std::make_unique<ObnSurfSaver>(ObnSurfSaver(db)),
	      std::make_unique<ObnAeroSaver>(ObnAeroSaver(db))),
  _db(db)
{
  _oldStations = { {"22113", "22008"},
		   {"26063", "26075"},
		   {"27612", "27713"},
		   {"24125", "24122"},
		   {"24944", "24947"},
		   {"25400", "25403"},
		   {"28698", "28695"},
		   {"30554", "30557"} };   

  _station = station;
}

ObnClimatSaver::~ObnClimatSaver()
{
}

//! Дополнение климатической базы новыми данными
bool ObnClimatSaver::updateClimatData(rpc::Channel* chan)
{
  // std::unique_ptr<Psql> db(new Psql("10.10.11.76", 5433, "clidb_gis", "postgres", "1"));
  if ( nullptr == _db.get() )  {return false;}

  bool ok = false;


  var("one");
  //surf ru
  sprinf::MeteostationType cltype = sprinf::kStationSynop;
  QString squery = "SELECT cldtot as \"N\", qcldtot as \"qN\", cldlow as \"Nh\", qcldlow as \"qNh\", "			
    "high_low as h, qhigh_low as qh, visby as \"V\", qvisby as \"qV\","			
    "cldver as \"CL\", qcldver as \"qCL\", cldsl as \"CL1\", qcldsl as \"qCL1\", "
    "cldmean as \"CM\" , qcldmean as \"qCM\", cldrain as \"CM1\" , qcldrain as \"qCM1\", "
    "cldhigh as \"CH\", qcldhigh as \"qCH\", wnddir as dd, qwnddir as qdd, "			
    "wndspdm as ff, qwndspdm as qff, tempdb as \"T\", qtempdb as \"qT\", "
    "relhum as \"U\", qrelhum as \"qU\", presst as \"P0\", qpresst as \"qP0\", "
    "soiltmp as \"Tg_\", qsoiltmp as \"qTg_\", precip as \"R\", qprecip as \"qR\", "
    "station as wmo_id, "
    "jsonb_set(ST_AsGeoJson(meteo.stations.location)::jsonb,'{alt}', to_jsonb(meteo.stations.alt)) as location, "
    "name->'ru' as ru , name->'international' as en, meteo.stations.country "
    "FROM \"CLIDATA\".tmc_sroki as cli, \"CLIADM\".passport as adm, meteo.stations "
    "where cli.station_id = adm.station_id and lpad(station, 5, '0') = adm.wmo_id and station_type = 0  "
    "and adm.wmo_id = '%1' and date_obs = '%2'";

  static_cast<ObnSurfSaver*>(getSurf())->setStrQuery(squery);
  
  QString str = QString("SELECT adm.wmo_id as station_id FROM \"CLIDATA\".tmc_sroki as cli "
			"left join \"CLIADM\".passport as adm on cli.station_id = adm.station_id ");
  if (!_station.isEmpty()) {
    str += QString("where adm.wmo_id = '%1' ").arg(_station);
  }
  str += "group by adm.wmo_id";
  
  ok |= updateClimatData(_db.get(), chan, str, cltype);
  
   //surf en
  var("two");
  cltype = sprinf::kStationSynop;

  squery = "SELECT cldtot as \"N\", qcldtot as \"qN\", cldlow as \"Nh\", qcldlow as \"qNh\", "			
    "high_low as h, qhigh_low as qh, visby as \"V\", qvisby as \"qV\", "			
    "cldver as \"CL\", qcldver as \"qCL\", cldsl as \"CL1\", qcldsl as \"qCL1\", "
    "cldmean as \"CM\" , qcldmean as \"qCM\", cldrain as \"CM1\" , qcldrain as \"qCM1\", "
    "cldhigh as \"CH\", qcldhigh as \"qCH\", wnddir as dd, qwnddir as qdd, "			
    "wndspdm as ff, qwndspdm as qff, tempdb as \"T\", qtempdb as \"qT\", "
    "relhum as \"U\", qrelhum as \"qU\", presst as \"P0\", qpresst as \"qP0\", "
    "soiltmp as \"Tg_\", qsoiltmp as \"qTg_\", precip as \"R\", qprecip as \"qR\", "
    "station as wmo_id, jsonb_set(ST_AsGeoJson(meteo.stations.location)::jsonb,'{alt}', to_jsonb(meteo.stations.alt)) as location, "
    "name->'ru' as ru , name->'international' as en, meteo.stations.country "
    "FROM \"CLIDATA\".synop96_z as cli, meteo.stations where "
    "lpad(station, 5, '0') = cli.station_id and station_type = 0 and cli.station_id = '%1' and date_obs = '%2'";
  
  static_cast<ObnSurfSaver*>(getSurf())->setStrQuery(squery);
  
  str = QString("SELECT station_id FROM \"CLIDATA\".synop96_z ");
  if (!_station.isEmpty()) {
    str += QString("where station_id = '%1' ").arg(_station); //04351
  }	  
  str += "group by station_id";
  ok |= updateClimatData(_db.get(), chan, str, cltype);
  

  var("three");
  //aero
  cltype = sprinf::kStationAero;
  str = QString("SELECT station_id FROM \"CLIDATA\".air_srok ");
  if (!_station.isEmpty()) {
    str += QString("where station_id = '%1' ").arg(_station); //01001
  }	  
  str += "group by station_id";

  ok |= updateClimatData(_db.get(), chan, str, cltype);
  

  return ok;
}

bool ObnClimatSaver::updateClimatData(Psql* db, rpc::Channel* chan, const QString& str,				     
				      sprinf::MeteostationType cltype)
{
  std::unique_ptr<DbiQuery> query(db->query(str)); 
  if(false == query->execInit( nullptr)) {
    error_log << "err";
    return false;
  }
  
  bool ok = false;
  while (true == query->next()) {
    ok = true;
    const DbiEntry& doc = query->entry();
    QString station_id = doc.valueString("station_id");    
    if (!station_id.isEmpty()) {
      if (cltype == sprinf::kStationAero && _oldStations.contains(station_id)) {
	station_id = _oldStations.value(station_id);
      }
     
      QDateTime first, last; //дата первой и последней записи
      ClimatSaver::updateClimatData(chan, station_id, cltype, cltype, &first, &last);

      if (cltype == sprinf::kStationSynop) {
	const ClimatStation& curStation = static_cast<ObnSurfSaver*>(getSurf())->currentStation();
	updateStation(curStation, first, last); 	
      } else if (cltype == sprinf::kStationAero) {
	const ClimatStation& curStation = static_cast<ObnAeroSaver*>(getAero())->currentStation();
	updateStation(curStation, first, last); 
      }

    }
  }

  return ok;
}


// "N"  cldtot код
// "Nh" cldlow код
// "h"  high_low м
// "V"  visby код
// "CL" cldver, cldsl
// "CM" cldmean, cldrain код
// "CH" cldhigh код
// "w"   -  
// "S6"  - 
// "dd" wnddir градусы
// "ff" wndspdm м/с
// "T" tempdb 
// "U" relhum
// "sss" - 
// "Ef"  -
// "P0"  presst
// "Tg_" soiltmp
// "R24";
 
// "R"; precip мм

