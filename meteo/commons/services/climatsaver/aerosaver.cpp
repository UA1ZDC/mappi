#include "aerosaver.h"


#include <sql/nosql/nosqlquery.h>
#include <sql/dbi/dbi.h>
#include <sql/dbi/dbiquery.h>

#include <cross-commons/debug/tlog.h>
#include <meteo/commons/rpc/rpc.h>
#include <meteo/commons/global/global.h>

#include <meteo/commons/proto/surface_service.pb.h>
#include <meteo/commons/zond/zond.h>
#include <commons/textproto/tprototext.h>


using namespace meteo;
using namespace climat;

const QString kAeroCollection = "aero";

const int kTimeout = 1000000;

AeroSaver::~AeroSaver()
{
}

QString AeroSaver::dtCheckTableName()
{
  return kAeroCollection;
}


void AeroSaver::updateAeroData(rpc::Channel* chan, const QString& station, sprinf::MeteostationType type,
				 const QDate& astart, const QDate& dtEnd)
{
  // var(astart.toString());
  // var(_dtEnd.toString());
  
  QDate start = astart;
  
  // if (!start.isValid()) {
  //   start = findDtStart(station, type, kAeroCollection);
    //var(start.toString());
    if (start > dtEnd || !start.isValid()) {
      error_log << QObject::tr("Ошибка временного интервала start=%1 end=%1")
	.arg(start.toString("yyyy-MM-dd")).arg(dtEnd.toString("yyyy-MM-dd"));
      return;
    }
  // }

  info_log << QObject::tr("Сохранение аэрологических данных: станция=") << station << start.toString() << dtEnd.toString();

  QDateTime dt = QDateTime(start, QTime(0,0), Qt::UTC);
  
  while (dt.date() <= dtEnd) {
    for (int hour = 0; hour < 2; hour++ ) {
      // var(dt.toString("yyyy-MM-ddThh:mm:ss:zzz"));
      if (addAeroAccumulation(chan, station, type, dt)) {
	if (!_first.isValid()) {
	  _first = dt;
	} else {
	  _last = dt;
	}
      }
      dt = dt.addSecs(12 * 3600);
    }
  }
  
}

surf::TZondValueReply* AeroSaver::requestAeroData(rpc::Channel* chan, const QString& station,
						    const QDateTime& date, const QStringList& reqdescr)
{
  trc;
  surf::DataRequest req;
  Q_UNUSED(reqdescr);
  
  req.add_station(station.toStdString());
  req.add_type(surf::kAeroType);
  req.set_date_start(date.toString(Qt::ISODate).toStdString());
  req.set_date_end(date.toString(Qt::ISODate).toStdString());
  req.set_only_last(1);
  
  //var(req.DebugString());
  
  surf::TZondValueReply* aero = chan->remoteCall(&::meteo::surf::SurfaceService::GetTZondOnStation, req, kTimeout);
  if (aero == nullptr ) return nullptr;

  //var(aero->Utf8DebugString());
  
  if (!aero->IsInitialized() || aero->data_size() == 0 || aero->data(0).has_meteodata() == 0) {
    delete aero;
    return 0;
  }
  
  return aero;
}

bool AeroSaver::checkQuality(float value, int quality)
{
  if (quality < control::SPECIAL_VALUE && !MnMath::isEqual(value, BAD_METEO_ELEMENT_VAL)) {
    return true;
  }

  return false;
}

int AeroSaver::ddtoCode(float dd, float ff)
{
  if (qFuzzyIsNull(dd) && qFuzzyIsNull(ff)) return 0; //штиль

  int ddCode = 0;

  if (dd <= 22.5f || dd > 337.5f) {
    ddCode = 8; //С
  } else if (dd <= 67.5f) {
    ddCode = 1; //СВ
  } else if (dd <= 112.5f) {
    ddCode = 2; //В
  } else if (dd <= 157.5f) {
    ddCode = 3; //ЮВ
  } else if (dd <= 202.5f) {
    ddCode = 4; //Ю
  } else if (dd <= 247.5f) {
    ddCode = 5; //ЮЗ
  } else if (dd <= 292.5f) {
    ddCode = 6; //З
  } else if (dd <= 337.5f) {
    ddCode = 7; //СЗ
  }    

  return ddCode;
}

//! Сохранение аэрологических данных, которые надо накапливать для последюущего усреднения
bool AeroSaver::addAeroAccumulation(rpc::Channel* chan, const QString& station, sprinf::MeteostationType type, const QDateTime& dt)
{
  std::unique_ptr<Dbi> db(meteo::global::dbClimat());
  if ( nullptr == db.get() ) {
    error_log << QObject::tr("Не удается выполнить запрос в базу климатических данных");
    return false;
  }

  std::unique_ptr<DbiQuery> query = db->queryptrByName("cl_insert");
  if ( !query ) {
    error_log << QObject::tr("Не найден запрос cl_insert");
    return false;
  }

  
  QStringList reqdescr;
  reqdescr << "dd" << "ff" << "T";

  //var(station);
  surf::TZondValueReply* aero = requestAeroData(chan, station, dt, reqdescr);
  if (nullptr == aero) return false;
  
  QByteArray buf(aero->data(0).meteodata().data(), aero->data(0).meteodata().size());
  zond::Zond zond;
  zond << buf;
  zond.preobr();

  //var(aero->Utf8DebugString());

  //  return false;
    
 
  bool ok = true;
  QString param = createAeroJson(&zond, zond.stIndex(), type);
  if (param.isEmpty()) {
    return false;
  }
  //var(query);

  //QString strq = meteo::global::kMongoQueriesNew["cl_insert"];
  //добавляем документ
  //NosqlQuery query;
  query->argWithoutQuotes("tablename", kAeroCollection);
  query->argJson("param", QString("[ ") + param + "]");
  
  if (!query->exec()) {
    error_log << QObject::tr("Не удалось выполнить запрос = %1")
      .arg(query->query());
    ok = false;
  }
  
  //var(ok);
  
  delete aero;
  return ok;
}


QString AeroSaver::createAeroJson(zond::Zond* zond, const QString& station,
				    sprinf::MeteostationType type)
{
  // zond::Zond zond;
  // zond.setCoord(GeoPoint(aero.data(0).point().fi(), aero.data(0).point().la(), 3));
  // zond.setDataNoPreobr(aero.data(0));
  // debug_log << zond.coord();
  QString dt = zond->dateTime().toString(Qt::ISODate);

  QString param = createAeroWindJson(zond, station, type);
  param += createAeroTropoJson(zond, station, type, dt);
  param += createAeroLevelJson(*zond, station, type, dt);
  param += createAeroIsotermJson(zond, station, type, dt);
  
  if (param.isEmpty()) {
    return QString();
  }

  param.remove(-1, 1);
  
  return param;
}

//! Ветер по слоям
QString AeroSaver::createAeroWindJson(zond::Zond* zond, const QString& station, sprinf::MeteostationType type)
{
  if (false == meteo::global::kMongoQueriesNew.contains("cl_aero_param")) {
    error_log << QObject::tr("Не найден запрос 'cl_aero_param'");
    return QString();
  }
  
  QString strq = meteo::global::kMongoQueriesNew["cl_aero_param"];
  if (strq.isEmpty()) {
    return QString();
  }
  
  if (nullptr == zond) {
    return QString();
  }
  
  //float min = 1200;//декаметры
  float min = 12000;//метры
  float max = 0;
  const QMap<zond::Level, zond::Uroven>& url = zond->urovenList();

  
  QMapIterator<zond::Level, zond::Uroven> it(url);
  while (it.hasNext()) {
    it.next();
    if (checkQuality(it.value().value(zond::UR_H),  it.value().quality(zond::UR_H))  &&
  	checkQuality(it.value().value(zond::UR_dd), it.value().quality(zond::UR_dd)) &&
  	checkQuality(it.value().value(zond::UR_ff), it.value().quality(zond::UR_ff)) ) {
      max = it.value().value(zond::UR_H);
      break;
    }
  }
  
  it.toBack();
  while (it.hasPrevious()) {
    it.previous();
    if (checkQuality(it.value().value(zond::UR_H), it.value().quality(zond::UR_H)) &&
  	checkQuality(it.value().value(zond::UR_dd), it.value().quality(zond::UR_dd)) &&
  	checkQuality(it.value().value(zond::UR_ff), it.value().quality(zond::UR_ff)) ) {
      min = it.value().value(zond::UR_H);
      break;
    }
  }

  // var(max);
  // var(min);
  
  // for (int idx = 0; idx < aero.data(0).ur_size(); idx++) {
  //   if (checkQuality(aero.data(0).ur(idx).h().value(),  aero.data(0).ur(idx).h().quality())  &&
  // 	checkQuality(aero.data(0).ur(idx).ff().value(), aero.data(0).ur(idx).ff().quality()) &&
  // 	checkQuality(aero.data(0).ur(idx).dd().value(), aero.data(0).ur(idx).dd().quality())) {

  //     if (aero.data(0).ur(idx).h().value() < min) {
  // 	min = aero.data(0).ur(idx).h().value();
  //     }
  //     if (aero.data(0).ur(idx).h().value() > max) {
  // 	max = aero.data(0).ur(idx).h().value();
  //     }
  //   }
  // }

  // min *= 10;
  // max *= 10;

  QDateTime dtwr = QDateTime::currentDateTimeUtc();
  QString param;
  
  QList<int> h2;
  h2 << 1500 << 3000 << 6000 << 12000;
  if (max >= h2.at(0)) {
    for (int idx = 0; idx < h2.size(); idx++) {
      float dd, ff;
      if (max < h2.at(idx)) break;
      // debug_log << min << max << h2.at(idx);
      if (zond->getAverWindDF(min, h2.at(idx), &dd, &ff)) {
  	int ddCode = ddtoCode(dd, ff);
	
	NosqlQuery querydd;
	querydd.setQuery(strq);
	  
	querydd.arg("dt_write", dtwr)
	  .arg("station", station)
	  .arg("station_type", type)
	  .arg("dt", zond->dateTime())
	  .arg("level_type", meteodescr::kHeightLevel)
  	  .arg("level", h2.at(idx))
  	  .arg("descrname", "dd")
  	  .arg("value", ddCode);
  	param += querydd.query() + ",";

  	NosqlQuery queryff;
  	queryff.setQuery(strq);
	queryff.arg("dt_write", dtwr)
	  .arg("station", station)
	  .arg("station_type", type)
	  .arg("dt", zond->dateTime())
  	  .arg("level_type", meteodescr::kHeightLevel)
  	  .arg("level", h2.at(idx))
  	  .arg("descrname", "ff")
  	  .arg("value", ff);
  	param += queryff.query() + ",";
  	//debug_log << "average wind ok" << min << h2.at(idx) << dd << ff;
      } else {
  	//debug_log << "error average wind";
      }
    }
  }
 
  return param;
}


//! Температура на уровне тропопаузы
QString AeroSaver::createAeroTropoJson(zond::Zond* zond, const QString& station, sprinf::MeteostationType type, const QString& dt)
{
  if (false == meteo::global::kMongoQueriesNew.contains("cl_aero_param")) {
    error_log << QObject::tr("Не найден запрос 'cl_aero_param'");
    return QString();
  }
  QString strq = meteo::global::kMongoQueriesNew["cl_aero_param"];
  if (strq.isEmpty()) {
    error_log << QObject::tr("Не найден запрос 'cl_aero_param'");
    return QString();
  }
  if (nullptr == zond) {
    return QString();
  }

  QList<zond::Uroven> urTropo;
  if (!zond->getTropo(&urTropo)) {
    return QString();
  }
  //самый нижний
  int loIdx = -1;
  float loP = -1;
  for (int idx = 0; idx < urTropo.size(); idx++) {
    if (loP < urTropo.at(idx).value(zond::UR_P) &&
  	checkQuality(urTropo.at(idx).value(zond::UR_T), urTropo.at(idx).quality(zond::UR_T)) &&
  	checkQuality(urTropo.at(idx).value(zond::UR_H), urTropo.at(idx).quality(zond::UR_H))
  	) {
      loP = urTropo.at(idx).value(zond::UR_P);
      loIdx = idx;
    }
  }
  if (loIdx == -1) {
    return QString();
  }

  QDateTime dtwr = QDateTime::currentDateTimeUtc();
  QString param;
  
  NosqlQuery query;
  query.setQuery(strq);
  query.arg("dt_write", dtwr)
    .arg("station", station)
    .arg("station_type", type) 
    .argDt("dt", dt)
    .arg("level_type", meteodescr::kTropoLevel)
    .arg("level", round(urTropo.at(loIdx).value(zond::UR_H)))
    .arg("descrname", "T")
    .arg("value", urTropo.at(loIdx).value(zond::UR_T));
  param += query.query() + ",";

   return param;
}

//! Температура по уровням
QString AeroSaver::createAeroLevelJson(zond::Zond& zond, const QString& station,
					 sprinf::MeteostationType type, const QString& dt)
{
  if (false == meteo::global::kMongoQueriesNew.contains("cl_aero_param")) {
    error_log << QObject::tr("Не найден запрос 'cl_aero_param'");
    return QString();
  }
  QString strq = meteo::global::kMongoQueriesNew["cl_aero_param"];
  if (strq.isEmpty()) {
    return QString();
  }

  QDateTime dtwr = QDateTime::currentDateTimeUtc();
  QString param;
  
  QList<int> levels;
  zond::Uroven ur;
  levels << 1000 << 925 << 850 << 700 << 500 << 400 << 300 << 250
  	 << 200  << 150 << 100 <<  70 <<  50 <<  30 <<  20 <<  10;
  for (int lev= 0; lev < levels.size(); lev++) {
    if (zond.getUrPoP( levels.at(lev), &ur)) {
      if (checkQuality(ur.value(zond::UR_T), ur.quality(zond::UR_T))) {
     	NosqlQuery query;
  	query.setQuery(strq);
	query.arg("dt_write", dtwr)
	  .arg("station", station)
	  .arg("station_type", type) 
	  .argDt("dt", dt)
  	  .arg("level_type", meteodescr::kIsobarLevel)
  	  .arg("level", levels.at(lev))
  	  .arg("descrname", "T")
  	  .arg("value", ur.value(zond::UR_T));
  	param += query.query() + ",";
      }

      if (checkQuality(ur.value(zond::UR_dd), ur.quality(zond::UR_dd)) &&
  	  checkQuality(ur.value(zond::UR_ff), ur.quality(zond::UR_ff)) ) {
  	NosqlQuery query;
  	query.setQuery(strq);
	query.arg("dt_write", dtwr)
	  .arg("station", station)
	  .arg("station_type", type) 
	  .argDt("dt", dt)
	  .arg("level_type", meteodescr::kIsobarLevel)
  	  .arg("level", levels.at(lev))
  	  .arg("descrname", "dd")
  	  .arg("value", ur.value(zond::UR_dd));
  	param += query.query() + ",";
	
  	query.setQuery(strq);
	query.arg("dt_write", dtwr)
	  .arg("station", station)
	  .arg("station_type", type) 
	  .argDt("dt", dt)
  	  .arg("level_type", meteodescr::kIsobarLevel)
  	  .arg("level", levels.at(lev))
  	  .arg("descrname", "ff")
  	  .arg("value", ur.value(zond::UR_ff));
  	param += query.query() + ",";
      }
    }
  }

  if (zond.getUrz(&ur)) {
    if (checkQuality(ur.value(zond::UR_dd), ur.quality(zond::UR_dd)) &&
  	checkQuality(ur.value(zond::UR_ff), ur.quality(zond::UR_ff)) ) {
      NosqlQuery query;
      query.setQuery(strq);
      query.arg("dt_write", dtwr)
  	.arg("station", station)
  	.arg("station_type",type)
  	.argDt("dt", dt)
  	.arg("level_type", meteodescr::kIsobarLevel)
  	.arg("level", 0)
  	.arg("descrname", "dd")
  	.arg("value", ur.value(zond::UR_dd));
      param += query.query() + ",";
      
      query.setQuery(strq);
      query.arg("dt_write", dtwr)
	.arg("station", station)
	.arg("station_type", type) 
	.argDt("dt", dt)
  	.arg("level_type", meteodescr::kIsobarLevel)
  	.arg("level", 0)
  	.arg("descrname", "ff")
  	.arg("value", ur.value(zond::UR_ff));
      param += query.query() + ",";
    }    
  }
  
  return param;
}

//! Высота изотерм -20, -10, 0
QString AeroSaver::createAeroIsotermJson(zond::Zond* zond, const QString& station,
					   sprinf::MeteostationType type, const QString& dt)
{
  if (false == meteo::global::kMongoQueriesNew.contains("cl_aero_param")) {
    error_log << QObject::tr("Не найден запрос 'cl_aero_param'");
    return QString();
  }
  
  QString strq = meteo::global::kMongoQueriesNew["cl_aero_param"];
  if (strq.isEmpty()) {
    return QString();
  }

  QDateTime dtwr = QDateTime::currentDateTimeUtc();
  QString param;

  for (int t = -20; t <= 0; t = t+10) {
    QList<float> P, H;
    //zond->oprPPoParam(zond::UR_T, t, P);
    zond->oprHPoParam(zond::UR_T, t, &H);
    //debug_log << t << H << P;
    if (H.size() != 0) {
      float h = H.at(0);
      for (int idx = 1; idx < H.size(); idx++) {
  	if (h < H.at(idx)) {
  	  h = H.at(idx);
  	}
      }

      NosqlQuery query;
      query.setQuery(strq);
      query.arg("dt_write", dtwr)
  	.arg("station", station)
  	.arg("station_type", type)
  	.argDt("dt", dt)
  	.arg("level_type", meteodescr::kIsotermLevel)
  	.arg("level", round(h))
  	.arg("descrname", "T")
  	.arg("value", t);
      param += query.query() + ",";
      
    }
  }
  
  return param;
}



