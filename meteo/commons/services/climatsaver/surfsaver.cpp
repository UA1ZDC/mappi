#include "surfsaver.h"

#include <sql/nosql/nosqlquery.h>
#include <sql/dbi/dbi.h>
#include <sql/dbi/dbiquery.h>


#include <cross-commons/debug/tlog.h>
#include <meteo/commons/rpc/rpc.h>
#include <meteo/commons/global/global.h>

#include <commons/meteo_data/tmeteodescr.h>


using namespace meteo::climat;

const QString kSurfHourCollection = "surf_hour";
const QString kSurfDayCollection = "surf_day";
const QString kSurfAccumCollection = "surf_accumulation";
const int kTimeout = 1000000;

SurfSaver::~SurfSaver()
{
}

QString SurfSaver::dtCheckTableName()
{
  return kSurfHourCollection;
}

void SurfSaver::updateSurfData(rpc::Channel* chan, const QString& station,
			       sprinf::MeteostationType type,
			       const QDate& astart, const QDate& end)
{
  Q_UNUSED(chan);
  
  QDate start = astart;
 
  // if (!start.isValid()) {
  //   start = findDtStart(station, type, kSurfHourCollection);
    //var(start.toString());
  if (start > end || !start.isValid()) {
    error_log << QObject::tr("Ошибка временного интервала start=%1 end=%1")
      .arg(start.toString("yyyy-MM-dd")).arg(end.toString("yyyy-MM-dd"));
    return;
  }
    //  }

  info_log << QObject::tr("Сохранение приземных данных: станция=") << station << start.toString() << end.toString();

  QDateTime dt(start, QTime(0,0));
  bool isadd = false;

  _isRmain = false;
  
  //основные сроки
  while (dt.date() <= end) {
    QDateTime cur = dt;
    //debug_log << "dt:" << dt.date().toString(Qt::ISODate);
    for (int hour = 0; hour < 4; hour++) {
      if (addSurf(chan, station, type, cur, true)) {
  	isadd = true;
	if (!_first.isValid()) {
	  _first = cur;
	} else {
	  _last = cur;
	}
      }
      cur = cur.addSecs(6 * 3600);
    }

    cur = dt;
    cur.setTime(QTime(3, 0));
    for (int hour = 0; hour < 4; hour++) {
      if (addSurf(chan, station, type, cur, !_isRmain)) {
  	isadd = true;
	if (!_first.isValid()) {
	  _first = cur;
	} else {
	  _last = cur;
	}
      }
      cur = cur.addSecs(6 * 3600);
    }
    
    dt = dt.addDays(1);
  }


  // isadd = true;
  // var(isadd);
  
  if (isadd) {
    updateDbSurf();
  }
  
}


//! Сохранение приземных данных
bool SurfSaver::addSurf(rpc::Channel* chan, const QString& station, sprinf::MeteostationType type,
			  const QDateTime& date, bool setR)
{
  if (// nullptr == _db ||
      nullptr == chan) return false;

  ComplexMeteo md;
  bool ok = requestSurfData(chan, station, type, date, setR, &md);
  if (!ok) {
    return false;
  }

  if (setR && !_isRmain && (date.time().hour() % 6) == 0) {
    if (md.hasParam("R")) {
      _isRmain = true;
    }
  }

  QString curStation = TMeteoDescriptor::instance()->stationIdentificator(md);
  if (curStation.isEmpty()) {
    curStation = station;    
  }
  
  bool isAdd = addSurfAccumulation(&md, curStation, type);
  addSurfHour(&md, curStation, type);
  addSurfDay(&md, curStation, type);
  
  return isAdd;
}



//! добавление данных, которые хранятся по срокам
bool SurfSaver::addSurfHour(ComplexMeteo* md, const QString& station, sprinf::MeteostationType type)
{
  //reqdescr <<  "V" << "dd" << "ff" << "T" << "N" << "Nh" << "C" << "h";
  
  if (// nullptr == _db ||
      nullptr == md) return false;
  // if (false == meteo::global::kMongoQueriesNew.contains("cl_insert")) {
  //   error_log << QObject::tr("Не найден запрос 'cl_insert'");
  //   return false;
  // }

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
  
  QString param = createSurfHourJson(md, station, type);
  if (param.isEmpty()) {
    return true;
  }
  param.remove(-1, 1);

  bool ok = false;
  // QString strq = meteo::global::kMongoQueriesNew["cl_insert"];

  // NosqlQuery query;
  // query.setQuery(strq)
  query->argWithoutQuotes("tablename", kSurfHourCollection);
  query->argJson("param", QString("[") + param + "]");

  if (!query->exec()) {
    error_log << QObject::tr("Не удалось выполнить запрос = %1")
      .arg(query->query());
    ok = false;
  }
  
  return ok;
}

QString SurfSaver::createSurfHourJson(ComplexMeteo* md, const QString& station, sprinf::MeteostationType type)
{
  if (nullptr == md) {
    return QString();
  }
  if (false == meteo::global::kMongoQueriesNew.contains("cl_surfhour_param")) {
    error_log << QObject::tr("Не найден запрос 'cl_surfhour_param'");
    return QString();
  }
  QString strq = meteo::global::kMongoQueriesNew["cl_surfhour_param"];
  if (strq.isEmpty()) {
    return QString();
  }

  //  md->printData();
  
  QString param;
  QDateTime dtwr = QDateTime::currentDateTimeUtc();

  QDateTime dt = md->dtBeg();
  dt.setTimeSpec(Qt::UTC);
  QStringList reqdescr;
  reqdescr << "dd" << "ff" << "T" << "V" << "N" << "Tg_";

  bool ddOk = false;
  for (auto name : reqdescr) {
    if (name == "ff" && !ddOk) {
      continue;
    }
    
    descr_t descr = TMeteoDescriptor::instance()->descriptor(name);
    TMeteoData* owner = nullptr;
    TMeteoParam* mp = md->paramPtr(descr, owner, true);
    if (nullptr == mp || nullptr == owner ||
  	(!checkQuality(mp->value(), mp->quality()) &&
  	 !(name == "ff" && mp->quality() == control::SPECIAL_VALUE) && //штиль
  	 !(name == "dd" && mp->quality() == control::SPECIAL_VALUE))
  	) {
      continue;
    }

    float val = mp->value();
    if (name == "dd") {
      int ddcode;
      if (setdd(md->getParam(TMeteoDescriptor::instance()->descriptor("ff")),
  		val, &ddcode)) {
  	val = ddcode;
  	ddOk = true;
      } else {
  	continue;
      }
    } else if (name == "N") {
      val /= 10.f;
    }
    
    //debug_log << name << mp->value() << dt << dtEnd;
    NosqlQuery query;
    query.setQuery(strq);

    query.arg("dt_write", dtwr);
    query.arg("station", station);
    query.arg("station_type", type);
    query.arg("dt", dt);
    query.arg("date", dt.date())
      .arg("hour", dt.time().hour())
      .arg("descrname", name.toStdString())
      .arg("value", val);
    query.removeDogs();
    
    param += query.query() + ",";
  }

  //---
  dt = md->dtBeg();
  dt.setTimeSpec(Qt::UTC);
  reqdescr.clear();
  reqdescr << "Nh" << "CL" << "CM" << "CH" << "h";
  
  QMap<QString, TMeteoParam*> clparam;
  const QList<ComplexMeteo*> clouds = md->findComplChilds(TMeteoDescriptor::instance()->descriptor("cloud"));
  for (int idx = clouds.size() - 1; idx >= 0; idx--) {
    for (auto name : reqdescr) {
      TMeteoParam* mp = clouds.at(idx)->meteoParamPtr(name, false);
      if (nullptr == mp || !checkQuality(mp->value(), mp->quality())) {
  	continue;
      }
      if (!clparam.contains(name)) {
  	clparam.insert(name, mp);
      } else if (clparam.value(name)->quality() > mp->quality()) {
  	clparam[name] = mp;
      }
    }    
  }

  QMapIterator<QString, TMeteoParam*> it(clparam);
  while (it.hasNext()) {
    it.next();
    NosqlQuery query;
    query.setQuery(strq);

    QString descrname = it.key();
    float value = it.value()->value();
    if (descrname == "Nh") {
      if (!clparam.contains("CL")) { //только Nh, которая для CL
  	continue;
      }
      //из кодового числа (окт) в баллы
      if (qFuzzyCompare(value, 6)) {
  	value = 6.5;
      } else if (value > 6) {
  	value += 2;
      } else if (qFuzzyCompare(value, 2)) {
  	value = 2.5;
      } else if (value > 2) {
  	value += 1;
      }
    }

    if (descrname == "CL" || descrname == "CM" || descrname == "CH") {
      bool isset = setCLMH(MnMath::ftoi_norm(value), &value);
      if (!isset) continue;
      descrname = "C";
    }

    query.arg("dt_write", dtwr);
    query.arg("station", station);
    query.arg("station_type", type);
    query.arg("dt", dt);
    query.arg("date", dt.date())
      .arg("hour", dt.time().hour())
      .arg("descrname", descrname.toStdString())
      .arg("value", value);
    query.removeDogs();
    
    param += query.query() + ",";
  }
  
  return param;
}


//! выделение нужных явлений
bool SurfSaver::updWvko(int ww, float Ef, int* Wvko)
{
  bool isset = false;
  //гроза
  if (ww ==  17 || ww == 29 || (ww >= 91 && ww <= 99) ||
      ww == 126 || (ww >= 190 && ww <= 196) || ww == 217 || 
      ww == 292 || ww == 293) {
    *Wvko |= 0x1; 
    isset = true;
  } 
  //туман
  if (ww ==  11 || ww == 12 || ww == 28 || (ww >= 40 && ww <= 49) ||
	     ww == 120 || (ww >= 130 && ww <= 135) || (ww >= 241 && ww <= 249)) {
    *Wvko |= 0x2; 
    isset = true;
  } 
  //метель
  if (ww == 38 || ww == 39 || ww == 239) {
    *Wvko |= 0x4; 
    isset = true;
  }
  //град
  if (ww ==  27 || (ww >= 87 && ww <= 90 ) || ww == 93 ||
	     ww ==  94 || ww == 96 || ww == 99 ||
	     ww == 189 || ww == 193 || ww == 196 || (ww >= 284 && ww <= 291)) {
    *Wvko |= 0x8; 
    isset = true;
  }
  //гололед (кроме ww, ещё по S6)
  if (ww == 24 || ww == 66 || ww == 67 ||
      ww == 125 || ww == 147 || ww == 148 || (ww >= 164 && ww <= 166) ||
      (ww == 224 || ww == 225 || ww == 227 || ww == 228)) {
    *Wvko |= 0x10; 
    isset = true;
  }
  //пыль
  if ((ww >= 30 && ww <= 35) ||
      ww == 104 || ww == 105 ||
      ww == 206 || ww == 208 || ww == 209 || ww == 230) {
    *Wvko |= 0x20; 
    isset = true;
  }
  //метель или пыль
  if (ww >= 127 && ww <= 129) {
    //если есть инфа о состояниии поверхности
    if (Ef >= 0 && Ef <= 9.5) { //если поверхность без снега
      *Wvko |= 0x20; 
      isset = true;
    } else if (Ef > 0) { //если поверхность со снегом
      *Wvko |= 0x4; 
      isset = true;
    }
  }
  //мгла
  if (ww == 5 || ww == 211) {
    *Wvko |= 0x40; 
    isset = true;
  }
  //шквал
  if (ww == 18 || ww == 19 || ww == 118) {
    *Wvko |= 0x80; 
    isset = true;
  }

  //debug_log << ww << *Wvko << isset;
  
  return isset;
}

//! выбор интересующих типов облачности
bool SurfSaver::setCLMH(int clmh, float* res)
{
  bool isset = true;
  *res = 0;

  switch (clmh) {
  case 30:
    *res = 0;
    break;
  case 31:
    *res = 1;
    break;
  case 32:
    *res = 2;
    break;
  case 33: case 39:
    *res = 3;
    break;
  case 34: case 35:
    *res = 4;
    break;
  case 36:
    *res = 5;
    break;
  case 22:
    *res = 6;
    break;
  case 37:
    *res = 7;
    break;
  default:
    isset = false;
  }
  
  return isset;
}

bool SurfSaver::setdd(const TMeteoParam& ffp, float dd, int* ddCode)
{
  //проверка, если штиль
  if (qFuzzyIsNull(dd)) {
    float ff = 0;
    if (ffp.isValid()) {
      ff = ffp.value();
    }
    
    if (qFuzzyIsNull(ff)) {
      *ddCode = 0;
      return true;
    }
  }
    
  *ddCode = ddtoCode(dd, 1);

  return true;
}


int SurfSaver::ddtoCode(float dd, float ff)
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



bool SurfSaver::requestSurfData(rpc::Channel* chan, const QString& station, sprinf::MeteostationType type,
				  const QDateTime& date, bool setR, ComplexMeteo* md)
{
  QList<std::string> reqdescr;
  reqdescr << "N"   << "Nh" << "h"  << "V"   << "CL" << "CM" << "CH"
	   << "w"   << "S6" << "dd" << "ff"  << "T"  << "U"
	   << "sss" << "Ef" << "P0" << "Tg_" << "R24";
  
  if (setR) {
    reqdescr << "R";
  }
  
  surf::DataRequest req;
  req.set_date_start(date.toString(Qt::ISODate).toStdString());
  req.add_station(station.toStdString());
  req.add_station_type(type);
  req.set_type_level(meteodescr::kSurfaceLevel);
  for (auto dname : reqdescr) {
    req.add_meteo_descrname(dname);
  }
  
  surf::DataReply* surf =  chan->remoteCall(&::meteo::surf::SurfaceService::GetMeteoDataTree, req, kTimeout);
  if (surf == nullptr ) return false;
  //var(surf->Utf8DebugString());
  if (!surf->IsInitialized() || surf->meteodata_size() == 0) {
    delete surf;
    return false;
  }

  QByteArray ba(surf->meteodata(0).data(), surf->meteodata(0).size());
  *md << ba;

  return true;
}


//! Сохранение приземных данных, которые надо накапливать для последюущего усреднения
bool SurfSaver::addSurfAccumulation(ComplexMeteo* md, const QString& station, sprinf::MeteostationType type)
{
  //reqdescr << "R" << "ff" << "U" << "sss" << "E'" << "w" << "S6"; 
  if (// nullptr == _db ||
      nullptr == md) return false;

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

  
  QString param = createSurfAccumJson(md, station, type);
  if (param.isEmpty()) {
    return false;
  }
  param.remove(-1, 1);

  bool ok = true;

  query->argWithoutQuotes("tablename", kSurfAccumCollection);
  query->argJson("param", QString("[ ") + param + "]");

  //debug_log << query->query();
  
  if (!query->exec()) {
    error_log << QObject::tr("Не удалось выполнить запрос = %1")
      .arg(query->query());
    ok = false;
  }
  
  return ok;
}

QString SurfSaver::createSurfAccumJson(ComplexMeteo* md, const QString& station, sprinf::MeteostationType type)
{
  if (nullptr == md) {
    return QString();
  }
  if (false == meteo::global::kMongoQueriesNew.contains("cl_surfaccum_param")) {
    error_log << QObject::tr("Не найден запрос 'cl_surfaccum_param'");
    return QString();
  }
  QString strquery = meteo::global::kMongoQueriesNew["cl_surfaccum_param"];
  if (strquery.isEmpty()) {
    return QString();
  }

  QString param;
  QDateTime dtwr = QDateTime::currentDateTimeUtc();

  QDateTime dt = md->dtBeg();
  dt.setTimeSpec(Qt::UTC);
  QDateTime dtEnd = md->dtEnd();
  dtEnd.setTimeSpec(Qt::UTC);
  QStringList reqdescr;
  reqdescr << "ff" << "U" << "sss" << "Ef" << "R" << "P0";

  //  QString strq = NosqlQuery::removeArg(strquery, "_id.dtend");
  
  for (auto name : reqdescr) {
    descr_t descr = TMeteoDescriptor::instance()->descriptor(name);
    TMeteoData* owner = nullptr;
    TMeteoParam* mp = md->paramPtr(descr, owner, true);
    if (nullptr == mp || nullptr == owner ||
	(!checkQuality(mp->value(), mp->quality()) && !(name == "ff" && mp->quality() == control::SPECIAL_VALUE))
	) {
      continue;
    }

    if (name == "Ef" && mp->value() < 10) { //интересует только поверхность со снегом
      continue;
    }
    
    ComplexMeteo* cowner = static_cast<ComplexMeteo*>(owner);
    if (nullptr != cowner) {
      dt = cowner->dtBeg();
      dt.setTimeSpec(Qt::UTC);
      dtEnd = cowner->dtEnd();
      dtEnd.setTimeSpec(Qt::UTC);
    }
    
    //debug_log << name << mp->value() << dt << dtEnd;
    NosqlQuery query;
    query.setQuery(strquery);
    
    if (name != "R") {
      dt = dtEnd;
    }
    
    query.arg("dt_write", dtwr);
    query.arg("station", station);
    query.arg("station_type", type);
    query.arg("dt", dt);
    if (name == "R") {
      query.arg("dtend", dtEnd);
    }
    query.arg("descrname", name.toStdString())
      .arg("value", mp->value());
    query.removeDogs();
    
    param += query.query() + ",";
  }

  int val = 0;
  bool isset = false;
  float Ef = md->getValue(TMeteoDescriptor::instance()->descriptor("Ef"), -1);
  descr_t descr = TMeteoDescriptor::instance()->descriptor("w");
  const QMap<int, TMeteoParam> wp = md->getParamList(descr);
  QMapIterator<int, TMeteoParam> it(wp);
  while (it.hasNext()) {
    it.next();
    if (checkQuality(it.value().value(), it.value().quality())) {
      isset |= updWvko(MnMath::ftoi_norm(it.value().value()), Ef, &val);
    }
  }

  descr = TMeteoDescriptor::instance()->descriptor("Sf");
  QMapIterator<int, TMeteoParam> it1(md->getParamList(descr));
  while (it1.hasNext()) {
    it1.next();
    if (checkQuality(it1.value().value(), it1.value().quality())) {
      if (it1.value().value() == 91) {
	isset = true;
	//гололед
	val |= 0x16;
      }
    }
  }

  if (isset) {
    NosqlQuery query;
    query.setQuery(strquery)
      .arg("dt_write", dtwr)
      .arg("station", station)
      .arg("station_type", type)
      .arg("dt", md->dtBeg())
      .arg("descrname", "Wvko")
      .arg("value", val);
    query.removeDogs();
    
    param += query.query() + ",";
  }


  //var(param);
  
  return param;
}



//! Сохранение приземных суточных данных
bool SurfSaver::addSurfDay(ComplexMeteo* md, const QString& station, sprinf::MeteostationType type)
{
  //reqdescr << "R24";
  if (// nullptr == _db ||
      nullptr == md) return false;
  // if (false == meteo::global::kMongoQueriesNew.contains("cl_insert")) {
  //   error_log << QObject::tr("Не найден запрос 'cl_insert'");
  //   return false;
  // }

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

  
  QString param = createDayJson(md, station, type);
  if (param.isEmpty()) {
    return false;
  }
  param.remove(-1, 1);

  bool ok = true;
  // QString strq = meteo::global::kMongoQueriesNew["cl_insert"];

  // NosqlQuery query;
  // query.setQuery(strq)
  query->argWithoutQuotes("tablename", kSurfDayCollection);
  query->argJson("param", QString("[") + param + "]");

 if (!query->exec()) {
    error_log << QObject::tr("Не удалось выполнить запрос = %1")
      .arg(query->query());
    ok = false;
  }
    
  return ok;
}

QString SurfSaver::createDayJson(ComplexMeteo* md, const QString& station, sprinf::MeteostationType type)
{
  if (nullptr == md) {
    return QString();
  }
  if (false == meteo::global::kMongoQueriesNew.contains("cl_surfhour_param")) {
    error_log << QObject::tr("Не найден запрос 'cl_surfhour_param'");
    return QString();
  }
  QString strquery = meteo::global::kMongoQueriesNew["cl_surfhour_param"];
  if (strquery.isEmpty()) {
    return QString();
  }

  QString param;
  QDateTime dtwr = QDateTime::currentDateTimeUtc();

  QDateTime dt = md->dtBeg();
  dt.setTimeSpec(Qt::UTC);
  dt.setTime(QTime(0,0));
  QStringList reqdescr;
  reqdescr << "R24";

  // QString strq = NosqlQuery::removeArg(strquery, "_id.dt");
  // strq = NosqlQuery::removeArg(strq, "_id.hour");
  
  for (auto name : reqdescr) {
    descr_t descr = TMeteoDescriptor::instance()->descriptor(name);
    TMeteoData* owner = nullptr;
    TMeteoParam* mp = md->paramPtr(descr, owner, true);
    if (nullptr == mp || nullptr == owner ||
  	(!checkQuality(mp->value(), mp->quality()))) {
      continue;
    }
   
   
    //debug_log << name << mp->value() << dt << dtEnd;
    NosqlQuery query;
    query.setQuery(strquery);
    
    query.arg("dt_write", dtwr);
    query.arg("station", station);
    query.arg("station_type", type);
    query.arg("dt", dt);
    query.arg("descrname", "R");
    query.arg("value", mp->value());
    query.removeDogs();
    
    param += query.query() + ",";
  }

  return param;
}


//! обновление приземных данных, которым необходимо усреднение
bool SurfSaver::updateDbSurf()
{
  //  if (nullptr == _db) { return false; }

  // day << "ff" << "Wvko" << "Ef";
  bool ok = updateSurfDayMax();
  ok = updateSurfDayBitmask();
  // decade << "sss";
  ok &= updateSurfDecade();
  // month << "R" << "ff" << "U";
  ok &= updateSurfMonthAvg();
  ok &= updateSurfMonthDays();
  ok &= updateSurfMonthSum();
  
  return ok;
}

bool SurfSaver::updateSurfDayMax()
{
  //Ef, ff

  std::unique_ptr<Dbi> db(meteo::global::dbClimat());
  if ( nullptr == db.get() ) {
    error_log << QObject::tr("Не удается выполнить запрос в базу климатических данных");
    return false;
  }

  std::unique_ptr<DbiQuery> query = db->queryptrByName("cl_mapred_max");
  if ( !query ) {
    error_log << QObject::tr("Не найден запрос cl_mapred_max");
    return false;
  }
   
  if (!meteo::global::kMongoFuncsNew.contains("map_cl_day")) {
    error_log << QObject::tr("Не найдены функции обработки климатических данных");
    return false;
  }

  bool ok = true;

  QString mapf = meteo::global::kMongoFuncsNew["map_cl_day"];
  QStringList descrname = QStringList() << "ff" << "Ef";
    
  query->argJsFunc("map_func", mapf).
    argWithoutQuotes("tablename", "surf_day").
    arg("descrname", descrname);
  
  //  var( query->query());
  
  if (!query->exec()) {
    error_log << QObject::tr("Не удалось выполнить запрос = %1")
      .arg(query->query());
    ok = false;
  }

  return ok;
}
  
  //----

bool SurfSaver::updateSurfDayBitmask()
{
  // Wvko

  std::unique_ptr<Dbi> db(meteo::global::dbClimat());
  if ( nullptr == db.get() ) {
    error_log << QObject::tr("Не удается выполнить запрос в базу климатических данных");
    return false;
  }

  std::unique_ptr<DbiQuery> query = db->queryptrByName("cl_mapred_bitsum");
  if ( !query ) {
    error_log << QObject::tr("Не найден запрос cl_mapred_bitsum");
    return false;
  }

  bool ok = true;
  QString mapf = meteo::global::kMongoFuncsNew["map_cl_day"];

  query->argJsFunc("map_func", mapf).
    argWithoutQuotes("tablename", "surf_day").
    arg("descrname",  QStringList("Wvko"));
  
  //var( query.query());
  
  if (!query->exec()) {
    error_log << QObject::tr("Не удалось выполнить запрос = %1")
      .arg(query->query());
    ok = false;
  }
  
  return ok;
}

bool SurfSaver::updateSurfDecade()
{
  //sss
  std::unique_ptr<Dbi> db(meteo::global::dbClimat());
  if ( nullptr == db.get() ) {
    error_log << QObject::tr("Не удается выполнить запрос в базу климатических данных");
    return false;
  }

  std::unique_ptr<DbiQuery> query = db->queryptrByName("cl_mapred_avg");
  if ( !query ) {
    error_log << QObject::tr("Не найден запрос cl_mapred_avg");
    return false;
  }

  if (!meteo::global::kMongoFuncsNew.contains("map_cl_decade")) {
    error_log << QObject::tr("Не найдены функции обработки климатических данных'");
    return false;
  }

  bool ok = true;
  QString mapf = meteo::global::kMongoFuncsNew["map_cl_decade"];

  query->argJsFunc("map_func", mapf)
    .argWithoutQuotes("tablename", "surf_decade")
    .arg("descrname", QStringList("sss"));

  if (!query->exec()) {
    error_log << QObject::tr("Не удалось выполнить запрос = %1")
      .arg(query->query());
    ok = false;
  }
  
  return ok;
}

bool SurfSaver::updateSurfMonthAvg()
{
  //ff, U, P0
  std::unique_ptr<Dbi> db(meteo::global::dbClimat());
  if ( nullptr == db.get() ) {
    error_log << QObject::tr("Не удается выполнить запрос в базу климатических данных");
    return false;
  }

  std::unique_ptr<DbiQuery> query = db->queryptrByName("cl_mapred_avg");
  if ( !query ) {
    error_log << QObject::tr("Не найден запрос cl_mapred_avg");
    return false;
  }

   
  if (!meteo::global::kMongoFuncsNew.contains("map_cl_hour") ||
      !meteo::global::kMongoFuncsNew.contains("map_cl_precip") ||
      !meteo::global::kMongoFuncsNew.contains("map_cl_day_precip")) {
    error_log << QObject::tr("Не найдены функции обработки климатических данных'");
    return false;
  }

  bool ok = true;
  QString mapf = meteo::global::kMongoFuncsNew["map_cl_hour"];
  QStringList descrname = QStringList() << "ff" << "U" << "P0";
  
  query->argJsFunc("map_func", mapf)
    .argWithoutQuotes("tablename", "surf_month")
    .arg("descrname", descrname);

  //var(query->query());
  
  if (!query->exec()) {
    error_log << QObject::tr("Не удалось выполнить запрос = %1")
      .arg(query->query());
    ok = false;
  }

  return ok;
}


bool SurfSaver::updateSurfMonthDays()
{
  //YR01
  std::unique_ptr<Dbi> db(meteo::global::dbClimat());
  if ( nullptr == db.get() ) {
    error_log << QObject::tr("Не удается выполнить запрос в базу климатических данных");
    return false;
  }
  
  std::unique_ptr<DbiQuery> query = db->queryptrByName("cl_mapred_days");
  if ( !query ) {
    error_log << QObject::tr("Не найден запрос cl_mapred_days");
    return false;
  }
  
  bool ok = true;
  QString mapf = meteo::global::kMongoFuncsNew["map_cl_day_precip"];
  
  query->argJsFunc("map_func", mapf)
    .argWithoutQuotes("tablename", "surf_month")
    .arg("descrname", QStringList("R"));

  //var(query->query());
  
  if (!query->exec()) {
    error_log << QObject::tr("Не удалось выполнить запрос = %1")
      .arg(query->query());
    ok = false;
  }

  return ok;
}

bool SurfSaver::updateSurfMonthSum()
{
  //R
  std::unique_ptr<Dbi> db(meteo::global::dbClimat());
  if ( nullptr == db.get() ) {
    error_log << QObject::tr("Не удается выполнить запрос в базу климатических данных");
    return false;
  }

  std::unique_ptr<DbiQuery> query = db->queryptrByName("cl_mapred_sum");
  if ( !query ) {
    error_log << QObject::tr("Не найден запрос cl_mapred_sum");
    return false;
  }
  
  bool ok = true;
  QString mapf = meteo::global::kMongoFuncsNew["map_cl_precip"];
  
  query->argJsFunc("map_func", mapf)
    .argWithoutQuotes("tablename", "surf_month")
    .arg("descrname", QStringList("R"));

  //var(query->query());
  
  if (!query->exec()) {
    error_log << QObject::tr("Не удалось выполнить запрос = %1")
      .arg(query->query());
    ok = false;
  }
  
  return ok;
}


bool SurfSaver::checkQuality(float value, int quality)
{
  if (quality < control::SPECIAL_VALUE && !MnMath::isEqual(value, BAD_METEO_ELEMENT_VAL)) {
    return true;
  }

  return false;
}

