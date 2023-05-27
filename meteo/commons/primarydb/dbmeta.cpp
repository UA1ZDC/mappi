#include "dbmeta.h"

#include <sql/nosql/nosqlquery.h>
#include <sql/nosql/document.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/global/common.h>

using namespace meteo;
using namespace anc;

QString StationInfo::toString() const
{
  return QString( "StationInfo: %1 %2 %3 %4 %5 %6 %7 %8 %9 %10 %11 %12 %13 %14" )
    .arg( QString("\n\t %1").arg(station) )
    .arg( QString("\n\t %1").arg(type) )
    .arg( QString("\n\t %1").arg( coord.toString() ) )
    .arg( QString("\n\t %1").arg(index) )
    .arg( QString("\n\t %1").arg(icao) )
    .arg( QString("\n\t %1").arg(call_sign) )
    .arg( QString("\n\t %1").arg(buoy) )
    .arg( QString("\n\t %1").arg(name) )
    .arg( QString("\n\t %1").arg(name_ru) )
    .arg( QString("\n\t %1").arg(country) )
    .arg( QString("\n\t %1").arg(country_en) )
    .arg( QString("\n\t %1").arg(country_ru) )
    .arg( QString("\n\t %1").arg(satellite) )
    .arg( QString("\n\t %1").arg(instrument) );

//    QString station;                     //!< станция
//    int type = BAD_METEO_ELEMENT_VAL;    //!< тип станции
//    GeoPoint coord;                      //!< координаты
//    int index = BAD_METEO_ELEMENT_VAL;   //!< индекс
//    QString icao;                        //!< ИКАО
//    QString call_sign;                   //!< позывной
//    int buoy = BAD_METEO_ELEMENT_VAL;    //!< номер буя
//    QString name;                        //!< название международное
//    QString name_ru;                     //!< название на русском
//    int country = BAD_METEO_ELEMENT_VAL; //!< номер страны
//    QString country_en;  //!< название страны международное
//    QString country_ru;  //!< название страны на русском
//    QString satellite;   //!< название спутника
//    QString instrument;    //!< название прибора
}

DbMeta::DbMeta()
{

}

DbMeta::~DbMeta()
{

}

//! Проверка наличия и при необходимости перезаполнение из БД инфы
bool DbMeta::checkStations()
{
if (_info.isEmpty() || _aerodrRu.isEmpty() || _types.isEmpty()) {
    bool ok = true;
    ok |= fillTypes();
    ok |= fillStations();
    if (!ok) {
        return false;
      }
  }

return true;
}


bool DbMeta::fillTypes()
{
  _types.clear();

  sprinf::MultiStatementRequest req;
  sprinf::StationTypes types;
  bool ok = meteo::global::loadStationTypes(&types, req);
  if (!ok || types.station_size() == 0) {
    error_log << QObject::tr("Ошибка получения типов станций");
    return false;
  }

  for (sprinf::StationType one : types.station()) {
    if (!one.has_max_gmi_types() || !one.has_type()) {
      return false;
    }
    _types.insert(one.max_gmi_types(),  meteo::sprinf::MeteostationType(one.type()));
  }
  return true;
}

//return nullptr - если данных нет
const StationInfo* DbMeta::stationInfo(int category, const QString& station)
{
  if (!checkStations()) {
    return nullptr;
  }

  //debug_log << category << station;
  meteo::sprinf::MeteostationType type = _types.lowerBound(category).value();
  //var(type);
  StationKey key = StationKey(station, type);
  if ( false == _info.contains(key)) {
    if (category ==  meteo::surf::kSynopSnow) { //кн-24 может передаваться как с синоп, так и с гидропоста
      key = StationKey(station, meteo::sprinf::kStationHydro);
      if (_info.contains(key)) {
        return &_info[key];
      }
    }
    else if (category == meteo::surf::kAeroBufr) { //в BUFR могут идти как данные фиксированных станций, так и подвижных
      key = StationKey(station, meteo::sprinf::kStationAero);
      if (_info.contains(key)) {
        return &_info[key];
      }
    }
    return nullptr;
  }
  return &_info[key];
}

const StationInfo* DbMeta::aerodromeInfo(const QString& stationRu)
{
if (!checkStations()) {
    return nullptr;
  }

QString station = stationRu.toUpper();
station.replace(QRegExp("\\W"), "");
if (!_aerodrRu.contains(station)) {
    return nullptr;
  }
return &_aerodrRu[station];
}

meteo::sprinf::MeteostationType DbMeta::stationType(int category)
{
if (!checkStations()) {
    return   meteo::sprinf::kStationUnk;
  }

return _types.lowerBound(category).value();
}

//! возвращает код для текстового значения (явления w_w_)
int DbMeta::bufrcode(const QString& text)
{
bool ok = false;

if (_bufrcode.isEmpty()) {
    ok = loadBufrCode();
    if (!ok) {
        return BAD_METEO_ELEMENT_VAL;
      }
  }

if (!_bufrcode.contains(text.toLower())) {
    return BAD_METEO_ELEMENT_VAL;
  }

return _bufrcode.value(text.toLower());
}

//! списко всех станций со странами
bool DbMeta::fillStations()
{
  _info.clear();
  _aerodrRu.clear();
  info_log << QObject::tr("Загрузка из БД данных по станциям");


  sprinf::MultiStatementRequest req;
  sprinf::Stations stations;
  bool ok = meteo::global::loadStations(req,&stations);
  if (!ok || stations.station_size() == 0) {
    error_log << QObject::tr("Ошибка получения типов станций");
    return false;
  }

  for ( sprinf::Station one : stations.station() ) {
    StationInfo info;
    if (one.has_position()) {
      info.coord  =  pbgeopoint2geopoint(one.position());
    }
    if (one.has_name()&&one.name().has_international() ) {
      info.name = QString::fromStdString(one.name().international());
    }
    if (one.has_name()&&one.name().has_rus() ) {
      info.name_ru = QString::fromStdString(one.name().rus());
    }
    if (one.has_country() ) {
      info.country = one.country().number();
    }

    info.country_en = QString::fromStdString( one.country().name_en() );
    info.country_ru = QString::fromStdString( one.country().name_ru() );
    if (one.has_cccc() ) {
      info.icao = QString::fromStdString(one.cccc());
    }
    if (one.has_index()) {
      info.index = one.index();
    }
    QString station;
    if ( one.has_station()) {
      station = QString::fromStdString(one.station());
    }
    else {
      error_log << "NO STATION DATA IN Station";
    }
    meteo::sprinf::MeteostationType type =   meteo::sprinf::kStationUnk;

    if (one.has_type()) {
      type = meteo::sprinf::MeteostationType(one.type());
    }
    else {
      error_log << "NO MeteostationType IN Station";
    }
    info.station = station;
    info.type = type;
    _info.insert(StationKey(station, type), info);
    if (type == meteo::sprinf::kStationAerodrome) { //в сводках имена на русском
      QString name = info.name_ru.toUpper().replace(QObject::tr("Ё"), QObject::tr("Е"));
      name.replace(QRegExp("\\W"), "");
      _aerodrRu.insert(name, info);
    }
  }
  return true;
}

bool DbMeta::loadBufrCode()
{

  std::unique_ptr<Dbi> db(meteo::global::dbSprinf());
  if ( nullptr == db.get() ) {
    error_log << QObject::tr("Не удается выполнить подключение к базе данных dbSprinf");
    return false;
  }
  auto query = db->queryptrByName("find_bufr_code");
  if(nullptr == query){ return false; }
  
  info_log << QObject::tr("Загрузка из БД кодовых таблиц");
  
  
  int wdescr = 5520019;
  
  query->arg("bufr_code", wdescr);
  
  QString error;
  if ( false == query->execInit(&error) ) {
    //error_log << error;
    return false;
  }
  
  
  const DbiEntry& doc = query->result();
  
  if (!doc.hasField("values")) {
    error_log << QObject::tr("Не найдена таблица для дескриптора %1").arg(wdescr);
    return false;
  }
  
  Array values = doc.valueArray("values");
  while (values.next()) {
    Document item;
    if (!values.valueDocument(&item)) {
      continue;
    }
    bool okval;
    int32_t code = item.valueInt32("code", &okval);
    if (!okval) continue;
    QString descr = item.valueString("description", &okval);
    if (!okval) continue;
    _bufrcode.insert(descr.toLower(), code);
  }
  
  return true;
}



bool DbMeta::loadEsimo()
{
  std::unique_ptr<Dbi> db(meteo::global::dbSprinf());
  if ( nullptr == db.get() ) {
    error_log << QObject::tr("Не удается выполнить подключение к базе данных dbSprinf");
    return false;
  }
  auto query = db->queryptrByName("get_esimo_elements");
  if(nullptr == query) { return false; }
  
  info_log << QObject::tr("Загрузка из БД таблицы ЕСИМО");
  
  QString error;
  if ( false == query->execInit(&error) ) {
    //error_log << error;
    return false;
  }
  
  while ( true == query->next() ) {
    const DbiEntry& doc = query->entry();
    QString name = doc.valueString("esimo_name");
    descr_t descr = doc.valueInt32("bufr_descr");
    
    _esimoDescr.insert(name, descr);
  }
  
  return true;
}

