#include "punktdb.h"
#include <sql/nosql/nosqlquery.h>
#include <sql/nosql/document.h>
#include <meteo/commons/global/global.h>

#include <meteo/commons/proto/forecast.pb.h>

namespace meteo {
namespace forecast {

static QString dbMeteo = "meteodb";

bool PunktDB::getForecastPunkts(PunktResponce *responce){

  std::unique_ptr<Dbi> db(meteo::global::dbForecast());
  if ( nullptr == db.get() ) {
    error_log << QObject::tr("Ошибка при подключении к базе данных mongodbConfForecast");
    return false;
    }
  auto query = db->queryptrByName("get_all_punkts");
  if(nullptr == query) {return false;}

  QString error;
  if(false == query->execInit( &error)){
    error_log << error;
      return false;
    }

  while ( true == query->next()) {
    const DbiEntry& doc = query->entry();
    QString id = doc.valueOid("_id");
    QString name = doc.valueString("name");
    double fi = MnMath::deg2rad( doc.valueDouble("fi") );
    double la = MnMath::deg2rad( doc.valueDouble("la") );
    double height = doc.valueDouble("height");
    QString stationID = doc.valueString("station_id");
    bool isActive = doc.valueBool("is_active");
    int stationType = doc.valueInt32("station_type");
    auto punkt = responce->add_punkts();
    punkt->set_id(id.toStdString());
    punkt->set_name(name.toStdString());
    punkt->set_fi(fi);
    punkt->set_la(la);
    punkt->set_height(height);
    punkt->set_stationid(stationID.toStdString());
    punkt->set_isactive(isActive);
    punkt->set_station_type(stationType);
  }
  return true;
}

bool PunktDB::removeForecastPunkts(const PunktRequest *request){
  QStringList oids;
  for (auto punkt : request->punkts()){
    if (true == punkt.has_id()){
      oids.append(QString::fromStdString(punkt.id()));
    }
  }
  if ( meteo::forecast::kPunktRemoveRequest != request->requestcode() ||
       true == oids.isEmpty() ){
    error_log << QObject::tr("Неверные параметры запроса");
    return false;
  }

  std::unique_ptr<Dbi> db(meteo::global::dbForecast());
  if ( nullptr == db.get() ) {
    error_log << QObject::tr("Ошибка при подключении к базе данных mongodbConfForecast");
    return false;
    }
  auto query = db->queryptrByName("delete_punkt");
  if(nullptr == query) {return false;}

  query->argOid("id",oids);

  if (!query->exec()){
    error_log << QObject::tr("Не удается выполнить запрос в базу данных");
    return false;
  }

  const meteo::DbiEntry& result = query->result();

  if ( (oids.size() != result.valueInt32("n"))){
    error_log << QObject::tr("Неизвестная ошибка - часть данных не была удалена");
    return false;
  }
  return true;
}


bool PunktDB::addForecastPunkts(  QString punktName, double fi, double la, double height,
				  QString stationId, int stationType, bool isActive )
{
  std::unique_ptr<Dbi> db(meteo::global::dbForecast());
  if ( nullptr == db.get() ) {
    error_log << QObject::tr("Ошибка при подключении к базе данных mongodbConfForecast");
    return false;
    }
  auto query = db->queryptrByName("insert_punkt");
  if(nullptr == query) {return false;}

  query->arg("name",punktName);
  query->arg("fi",float(fi));
  query->arg("la",float(la));
  query->arg("height",float(height));
  query->arg("station_id",stationId);
  query->arg("is_active",isActive);
  query->arg("station_type",stationType);

  if (!query->exec()){
    error_log << QObject::tr("Не удается выполнеть запрос в базу данных");
    return false;
  }

  const meteo::DbiEntry& result = query->result();
  if (1 != result.valueDouble("ok") ){
    error_log << "Ошибка записи в базу данных";
    return false;
  }
  return true;
}

bool PunktDB::addForecastPunkts(const PunktRequest *request){
  if (meteo::forecast::kPunktAddRequest != request->requestcode()) return false;
  bool allOk = true;
  for (auto punkt: request->punkts()){
    if ( !punkt.has_name() ||
         !punkt.has_fi() ||
         !punkt.has_la() ||
         !punkt.has_height() ||
         !punkt.has_stationid() ||
         !punkt.has_isactive() ||
         !punkt.has_station_type() ) {
      allOk = false;
      error_log << QObject::tr("Отсуствует часть обязательных полей");
      continue;
    }
    QString punktName = QString::fromStdString(punkt.name());
    double fi = punkt.fi();
    double la = punkt.la();
    double height = punkt.height();
    QString stationId = QString::fromStdString(punkt.stationid());
    bool isActive = punkt.isactive() == 1;
    int stationType = punkt.station_type();

    allOk &= addForecastPunkts(punktName, fi, la, height, stationId, isActive, stationType);
  }

  return allOk;
}

bool PunktDB::changeActiveForecastPunkts(const PunktRequest *request){
  if ( meteo::forecast::kPunktEnabledChangeRequest != request->requestcode() ){
    error_log << QObject::tr("Некорректный вызов запроса");
    return false;
  }
  bool allOk = true;
  for (auto punkt : request->punkts()){
    if (!punkt.has_stationid() ||
        !punkt.has_isactive()){
      error_log<< QObject::tr("Отсутствует часть обязательных полей");
      allOk = false;
      continue;
    }

  std::unique_ptr<Dbi> db(meteo::global::dbForecast());
  if ( nullptr == db.get() ) {
    error_log << QObject::tr("Ошибка при подключении к базе данных mongodbConfForecast");
    return false;
    }
  auto query = db->queryptrByName("update_punkts_active_by_id");
  if(nullptr == query) {return false;}

    query->argOid("id",QString::fromStdString(punkt.stationid()));
    query->arg("is_active",punkt.isactive() == 1? true: false);

   if (!query->exec()){
    error_log << QObject::tr("Не удается выполнеть запрос в базу данных");
    return false;
  }

  const meteo::DbiEntry& result = query->result();
  if (1 != result.valueDouble("ok") ){
      error_log << "Ошибка записи в базу данных";
      allOk = false;
      continue;
    }
  }
  return allOk;
}

  const PunktValue PunktDB::getPunktByStationIndex(const QString& stationIndex, int stationType)
{
  std::unique_ptr<Dbi> db(meteo::global::dbForecast());
  if ( nullptr == db.get() ) {
    error_log << QObject::tr("Ошибка при подключении к базе данных mongodbConfForecast");
    return PunktValue();
    }
  auto query = db->queryptrByName("get_punkt_by_station_id");
  if(nullptr == query) {return PunktValue();}

  query->arg("station_id",  stationIndex);
  query->arg("station_type", stationType);

  QString error;
  if(false == query->execInit( &error)){
      //error_log << error;
      return PunktValue();
    }

  PunktValue value;
  int resultCnt = 0;
  while ( true == query->next()) {
    ++resultCnt;
    const meteo::DbiEntry& doc = query->entry();
    QString id = doc.valueOid("_id");
    QString name = doc.valueString("name");
    double fi = doc.valueDouble("fi");
    double la = doc.valueDouble("la");
    double height = doc.valueDouble("height");
    bool isActive = doc.valueBool("is_active");

    value.set_id(id.toStdString());
    value.set_name(name.toStdString());
    value.set_fi(fi);
    value.set_la(la);
    value.set_height(height);
    value.set_stationid(stationIndex.toStdString());
    value.set_isactive(isActive);
    value.set_station_type(stationType);
  }

  if (1 != resultCnt){
    return PunktValue();
  }

  return value;
}

}
}
