#include "obnaerosaver.h"

using namespace meteo::climat;

#include <sql/nosql/nosqlquery.h>
#include <sql/dbi/dbi.h>
#include <sql/dbi/dbiquery.h>
#include <sql/psql/psql.h>

#include <cross-commons/debug/tlog.h>
#include <meteo/commons/rpc/rpc.h>
#include <meteo/commons/global/global.h>

#include <meteo/commons/proto/locsettings.pb.h>
#include <meteo/commons/proto/surface_service.pb.h>
#include <meteo/commons/zond/zond.h>



using namespace meteo;
using namespace climat;

ObnAeroSaver::ObnAeroSaver(std::shared_ptr<Psql> db):
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
}

ObnAeroSaver::~ObnAeroSaver()
{
}

surf::TZondValueReply* ObnAeroSaver::requestAeroData(rpc::Channel* , const QString& station,
						     const QDateTime& date, const QStringList& )
{ 
  zond::Zond znd;
  if(! getOneZondData( station, date.toString(Qt::ISODate), &znd)) {
    return nullptr;
  }
  znd.restoreUrList();
  
  meteo::surf::TZondValueReply* res = new meteo::surf::TZondValueReply();
  meteo::surf::OneTZondValueOnStation* onezond = res->add_data();

  onezond->mutable_point()->set_fi(znd.coord().fi());
  onezond->mutable_point()->set_la(znd.coord().la());
  onezond->mutable_point()->set_height(znd.coord().alt());
  onezond->mutable_point()->set_id(station.toStdString());
  onezond->set_date(znd.dateTime().toString(Qt::ISODate).toUtf8().constData());
  onezond->set_hour(0);
  
  QByteArray buf;
  znd >> buf;
  onezond->set_meteodata(buf.data(), buf.size());
  res->set_comment(msglog::kServerAnswerOK.arg("getManyAeroData").toUtf8().constData());
  res->set_result(true);
  
  return res;
}



bool ObnAeroSaver::getOneZondData(const QString & station, const QString &dts, zond::Zond *znd)
{
  QElapsedTimer ttt; ttt.start();

  QString astation(station);
  
  if (!_oldStations.key(station).isEmpty()) {
    astation = _oldStations.key(station);
  }
  
  
  //  std::unique_ptr<Psql> db(new Psql("10.10.11.76", 5433, "clidb_gis", "postgres", "1"));
  if ( nullptr == _db.get() )  {return false;}

  QString str = QString("SELECT p, qp, h, qh, t, qt, dir, qdir, vm, qvm, station, "
			"jsonb_set(ST_AsGeoJson(location)::jsonb,'{alt}', to_jsonb(alt)) as location, "
			"name->'ru' as ru , name->'international' as en, country "
			"FROM \"CLIDATA\".air_srok as cli, meteo.stations "
			"WHERE lpad(station, 5, '0') = '%1' and station_type = 3 AND "
			"station_id = '%2' and date_obs = '%3'")
    .arg(station)
    .arg(astation)
    .arg(dts);

  //var(str);
  std::unique_ptr<DbiQuery> query(_db->query(str));
  if(false == query->execInit( nullptr)){
    error_log << "err";
    return false;
  }

  QList<QPair<QString, zond::ValueType>> descr =
    {{"p", zond::UR_P},
     {"h", zond::UR_H},
     {"t", zond::UR_T},
     {"dir", zond::UR_dd},
     {"vm", zond::UR_ff}
    };
 

  bool ok = false;
  bool flStation = false;
  zond::CloudData cld;
  
  while ( true == query->next() ) {
    ok = true;
    const DbiEntry& doc = query->entry();

    //    var(doc.jsonExtendedString());

    if (!flStation) {
      QString station = doc.valueString("station");
      fillStationObn(station, sprinf::kStationAero, doc);
      flStation = true;
    }
    
    
    zond::Uroven aur;
    for (auto param : descr) {
      auto qual = doc.valueDouble("q" + param.first);
      if (qual == 0 || qual == 1 || qual == 4 || qual == 5) {
	auto val = doc.valueDouble(param.first);
	aur.set(param.second, val, control::RIGHT);      
      }      
    }   

    aur.setLevelType(meteodescr::kIsobarLevel);
    aur.to_uv();
    if (!aur.isGood( zond::UR_Td))      { aur.to_Td();}
    else if (!aur.isGood( zond::UR_D))  { aur.to_D(); }

    float p = aur.value(zond::UR_P);
      
    znd->setData(meteodescr::kIsobarLevel, p, aur);
    
    QDateTime dt;
    dt = dt.fromString(dts,Qt::ISODate);
    dt.setTimeSpec(Qt::UTC);
    znd->setDateTime(dt);
    znd->setStIndex(doc.valueString("station"));
    meteo::GeoPoint gp= doc.valueGeo("location");
    gp.setAlt(doc.valueInt32("alt"));
    znd->setCoord(gp);
  }

  int cur =  ttt.elapsed();
  if(1000 < cur){
    warning_log << msglog::kServiceRequestTime.arg(query->query()).arg(cur);
  }
  return ok;
}

void ObnAeroSaver::fillStationObn(const QString& station, sprinf::MeteostationType type, const DbiEntry& doc)
{
  _curStation.mutable_station()->set_station(station.toStdString());
  _curStation.mutable_station()->set_type(type);
  _curStation.set_cltype(type);
  _curStation.set_utc(0);

  _curStation.mutable_station()->mutable_name()->set_international(doc.valueString("en").toStdString());
  _curStation.mutable_station()->mutable_name()->set_rus(doc.valueString("ru").toStdString());

  meteo::GeoPoint gp= doc.valueGeo("location");
  gp.setAlt(doc.valueInt32("alt"));
  auto pos = _curStation.mutable_station()->mutable_position();
  pos->set_lat_radian(gp.lat());
  pos->set_lon_radian(gp.lon());
  pos->set_height_meters(gp.alt());
  
  _curStation.mutable_station()->mutable_country()->set_number(doc.valueInt32("country"));
}
