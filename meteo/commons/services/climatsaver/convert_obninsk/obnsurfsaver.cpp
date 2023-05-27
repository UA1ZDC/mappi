#include "obnsurfsaver.h"

#include <QElapsedTimer>

#include <cross-commons/debug/tlog.h>
#include <meteo/commons/rpc/rpc.h>
#include <meteo/commons/global/global.h>
#include <commons/meteo_data/tmeteodescr.h>

#include <sql/nosql/nosqlquery.h>
#include <sql/psql/psql.h>

using namespace meteo::climat;

ObnSurfSaver::ObnSurfSaver(std::shared_ptr<Psql> db):
  _db(db)  
{
}

ObnSurfSaver::~ObnSurfSaver()
{
}

bool ObnSurfSaver::requestSurfData(rpc::Channel*, const QString& station, sprinf::MeteostationType type,
				   const QDateTime& date, bool setR, ComplexMeteo* cm)
{
  QList<QString> reqdescr;
  reqdescr << "N"   << "Nh" << "h"  << "V"   << "CL" << "CM" << "CH"
	   << "w"   << "S6" << "dd" << "ff"  << "T"  << "U"
	   << "sss" << "Ef" << "P0" << "Tg_" << "R24";
  
  if (setR) {
    reqdescr << "R";
  }
  QList<float> Ncoder = { 0., 10., 25., 40., 50., 60., 75., 90., 100., 113};
  QList<float> Nhcoder = { 0, 1, 3, 4, 5, 6, 8, 9, 10, 113 };
			     
  QElapsedTimer ttt; ttt.start();
  
  //  std::unique_ptr<Psql> db(new Psql("10.10.11.76", 5433, "clidb_gis", "postgres", "1"));
  if ( nullptr == _db.get() )  {return false;}

  QString str = _squery.arg(station).arg(date.toString(Qt::ISODate));

  
  //var(str);
  std::unique_ptr<DbiQuery> query(_db->query(str));

  //var(query->query());
  
  if(false == query->execInit( nullptr)){
    error_log << "err";
    return false;
  }

  TMeteoData md;

  bool ok = false;
  bool flStation = false;
  while (true == query->next()) {
    ok = true;
    const DbiEntry& doc = query->entry();
    //var(doc.jsonExtendedString());

    if (!flStation) {
      QString wmo_station = doc.valueString("wmo_id");
      fillStationObn(wmo_station, type, doc);
      md.setStation(wmo_station);
      flStation = true;
    }
    
    for (auto descrname : reqdescr) {
      if (!doc.hasField("q" + descrname) || !doc.hasField(descrname)) {
	continue;
      }
      
      auto qual = doc.valueDouble("q" + descrname);
      if (qual == 0 || qual == 1 || qual == 4 || qual == 5) {
	auto val = doc.valueDouble(descrname);
	if (descrname == "N") {
	  if (int(val) < Ncoder.size()) {	
	    val = Ncoder.at(val);
	  } else {
	    continue;
	  }
	} else if (descrname == "Nh") {
	  if (int(val) < Nhcoder.size()) {
	    val = Nhcoder.at(val);
	  } else {
	    continue;
	  }
	} else if (descrname == "CL" || descrname == "CM") {
	  if (val == 0 && doc.hasField("q" + descrname + "1") && doc.hasField(descrname + "1")) {
	    qual = doc.valueDouble("q" + descrname + "1");
	    if ((qual == 0 || qual == 1 || qual == 4 || qual == 5) && val != 0) {
	      val = doc.valueDouble(descrname + "1");
	    }
	  }
	  if (descrname == "CL") {
	    val += 30;
	  } else if (descrname == "CM") {
	    val += 20;
	  }		      
	} else if (descrname == "CH") {
	  val += 10;
	}
      
	md.add(descrname, TMeteoParam("", val, control::RIGHT)); 
      }
    }

    
  }

  md.setDateTime(date);
  //md.printData();
 
  cm->fill(md);
  cm->setStation(TMeteoDescriptor::instance()->stationIdentificator(md));

  //cm->printData();
  // cm->setDtType(meteodescr::kNormalTime);
  // cm->setDt1(date);
  // cm->setDt2(date);
  
  
  int cur =  ttt.elapsed();
  if(1000 < cur){
    warning_log << msglog::kServiceRequestTime.arg(query->query()).arg(cur);
  }
  
  return ok;
}


void ObnSurfSaver::fillStationObn(const QString& station, sprinf::MeteostationType type, const DbiEntry& doc)
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

