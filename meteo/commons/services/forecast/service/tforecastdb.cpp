#include "tforecastdb.h"

#include "tgradaciidb.h"

#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>

#include <meteo/commons/global/global.h>
#include <commons/meteo_data/tmeteodescr.h>

#include <qdom.h>
#include <qfile.h>
#include <qdir.h>
#include <qdatetime.h>

#include <qjsonobject.h>
#include <QJsonDocument>
#include <QDateTime>
#include <QJsonArray>
#include <commons/mathtools/mnmath.h>

#include <bson.h>
#include <sql/nosql/document.h>

#include <sql/nosql/nosqlquery.h>
#include <punktdb.h>

namespace meteo{
namespace forecast{


static const QString collectionForecast = "forecast";
// static const QString collectionPunkts = "punkts";
// static const QString collectionForecastOpr = "forecast_orp";
// static const QString collectionDopuskOpr = "dopusk_opr";
// static const QString collectionStations = "stations";

//  static bool forecast_success( int descriptor_, int level_, int type_level_, double value_, double freal_value_, int fhour_ = 24)
//  {
//    Q_UNUSED(descriptor_);
//    Q_UNUSED(level_);
//    Q_UNUSED(type_level_);
//    Q_UNUSED(value_);
//    Q_UNUSED(freal_value_);
//    Q_UNUSED(fhour_);
//    not_impl;
//    return false;
//    double _delta;
//    QVector<QVector<int>> _gradacii;
//    bool _success;
//    int kol_grad;
//    int min_1;
//    int max_1;
//    int min_0;
//    int max_2;
//    int _dopusk_grad;
//    bool has_delta;
//    bool has_gradacii;
//    {
//      QStringList dopuskFindList;
//      dopuskFindList << NoSqlField("descriptor", descriptor_).toString()
//                     << NoSqlField::in<int>("level", { level_ }).toString()
//                     << NoSqlField::in<int>("type_level", { type_level_ } ).toString()
//                     << NoSqlField("fhour", fhour_).toString();
//      Find findRequest = Find(collectionDopuskOpr, dopuskFindList);
//      auto db = meteo::global::dbForecast();
//      if (nullptr == db){
//        error_log << "Can not connect to database";
//        return false;
//      }
//      auto cursor = db->find( findRequest);
//      if (nullptr == cursor){
//        error_log << "Can not receive data from database";
//        return false;
//      }

//      int cnt = 0;
//      while (cursor->next()){
//        ++cnt;
//        QJsonObject obj = QJsonDocument::fromJson(cursor->getDocument()).object();
//        _delta = obj["delta"].toDouble();
//        QJsonArray arr = obj["gradacii"].toArray();
//        if (0 == arr.size() ){
//          error_log << QObject::tr("Ошибка: нет данных о точности прогнозов");
//          return  false;
//        }
//        for (QJsonValue value : arr){
//          _gradacii << QVector<int>();
//          QVector<int>& dataVector = _gradacii.last();
//          for (QJsonValue gradaciiValue : value.toArray()){
//            dataVector << gradaciiValue.toInt();
//          }
//        }
//        _dopusk_grad = obj["dopusk_grad"].toInt();
//        has_delta = obj["delta"].type() != QJsonValue::Type::Null;
//        has_gradacii = obj["gradacii"].type() != QJsonValue::Type::Null;
//      }
//      if (cnt != 1) {
//        error_log << QString("Database data corrupted - fount %1 field, expected 1").arg(cnt);
//        return false;
//      }

//      if (0 == cnt){
//        error_log << QObject::tr("Не найдено поле в базе данных");
//        return false;
//      }
//    }

//    if (has_delta){
//      _success = (freal_value_ + _delta* (1+ _dopusk_grad) >= value_
//                  &&  freal_value_ - _delta* (1+ _dopusk_grad) <= value_  ) ;
//      return  _success;
//    }
//    if (has_gradacii){
//      kol_grad =  _gradacii.size();
//      for( int i = 0; i < kol_grad; ++i ){
//        min_1 = _gradacii[i][0];
//        max_1 = _gradacii[i][1];

//        if (i - _dopusk_grad < 0 || _gradacii[i].size() < 2) {
//          min_0 = min_1;
//        } else {
//          min_0 = _gradacii[i-_dopusk_grad][0];
//        }
//        if (i+_dopusk_grad >= kol_grad || _gradacii[i + _dopusk_grad].size() < 2 ) {
//          max_2 = max_1;
//        } else {
//          max_2 = _gradacii[i+_dopusk_grad][1];
//        }
//        if ( value_ >=min_1 and value_ <= max_1 ) {
//          if( freal_value_ >=min_0 and freal_value_ <= max_2 ) {
//            _success = true;
//          }
//          else {
//            _success =false;
//          }
//        }
//      }
//    }
//    return _success;
//  }


void accuracy_calc( QString forecastID, QString punkt_id_, int center_, int descriptor_,
                    int level_, int type_level_, QString ftype_method_, int fhour_, double value_, double freal_value_)
{
  Q_UNUSED(forecastID);
  Q_UNUSED(punkt_id_);
  Q_UNUSED(center_);
  Q_UNUSED(descriptor_);
  Q_UNUSED(level_);
  Q_UNUSED(type_level_);
  Q_UNUSED(ftype_method_);
  Q_UNUSED(fhour_);
  Q_UNUSED(value_);
  Q_UNUSED(freal_value_);
  not_impl;
  return;
  //    bool isopr;
  //    int opr_;
  //    int not_opr_;

  //    isopr = false;
  //    isopr = forecast_success(descriptor_,level_,type_level_,value_,freal_value_);

  //    opr_ = 0;
  //    not_opr_ =1;
  //    if (true == isopr) {
  //      opr_ = 1;
  //      not_opr_ =0;
  //    }
  //    auto db = NoSqlManager::instance().getClient(global::mongodbConfForecast().host(), global::mongodbConfForecast().port().toInt());
  //    //UPDATE forecast SET fkach_pr = opr_  WHERE id = id_;
  //    {
  //      QVector<NoSqlField> forecastIdMatcher;
  //      forecastIdMatcher << NoSqlField::oid("_id", forecastID);
  //      FindAndModify forecastModifyQUery = FindAndModify::getUpdateRequest(collectionForecast, true, false, forecastIdMatcher);
  //      forecastModifyQUery.fieldSet(NoSqlField("fkach_pr", opr_));
  //      QJsonDocument doc;
  //      db->findAndModify( forecastModifyQUery, &doc);
  //    }

  //    {
  //      QVector<NoSqlField> forecastOprMatcher;
  //      forecastOprMatcher << NoSqlField("punkt_id", punkt_id_)
  //                         << NoSqlField("flevel", level_)
  //                         << NoSqlField("type_level", type_level_)
  //                         << NoSqlField("val_descr",descriptor_)
  //                         << NoSqlField("center", center_)
  //                         << (ftype_method_.isEmpty()? NoSqlField::exists("ftype_method", false) : NoSqlField("ftype_method",ftype_method_))
  //                         << NoSqlField("fhour", fhour_);
  //      FindAndModify findAndModifyOprMatcher = FindAndModify::getUpdateRequest(collectionForecastOpr, true, true, forecastOprMatcher);
  //      findAndModifyOprMatcher.fieldInc("opr", opr_);
  //      findAndModifyOprMatcher.fieldInc("not_opr", not_opr_);
  //      QJsonDocument doc;
  //      db->findAndModify( findAndModifyOprMatcher, &doc);
  //    }
}


TForecastDb::TForecastDb()
{
}

TForecastDb::~TForecastDb()
{
}

bool TForecastDb::getPunktId(const QString & station_number, QString *punkt_id){
  Q_UNUSED(station_number);
  Q_UNUSED(punkt_id);
  not_impl;
  return false;
  //  QStringList filters;
  //  filters << NoSqlField("station_id", station_number).toString();

  //  auto db = NoSqlManager::instance().getClient();
  //  if (nullptr == db) {
  //    error_log << "Не удается установить соединение с базой данных";
  //    return false;
  //  }
  //  auto cursor = db->find( Find(collectionPunkts, filters) );
  //  if (nullptr == cursor){
  //    error_log << db->getLastError();
  //    error_log <<  "Не удается получить данные из базы данных";
  //    return false;
  //  }

  //  int cnt = 0;
  //  QTime ttt; ttt.start();
  //  while( cursor->next() ){
  //    ++cnt;
  //    QJsonObject obj = QJsonDocument::fromJson(cursor->getDocument()).object();
  //    *punkt_id = obj["_id"].toObject()["$oid"].toString();
  //  }
  //  int cur =  ttt.elapsed();
  //  if(1000<cur){
  //    warning_log << msglog::kServiceRequestTime.arg(filters.join(" , ")).arg(cur);
  //  }
  //  return cnt == 1;
}


bool TForecastDb::getForecastFResult(const ForecastResultRequest* repl, ForecastResultReply* res)
{
  Q_UNUSED(repl);
  Q_UNUSED(res);
  not_impl;
  return false;
  //  auto client = meteo::global::dbForecast();
  //  if(nullptr == client) {
  //    error_log << QObject::tr("Нет подключения к БД");
  //    return false;
  //  }
  //  if ( false == client->isConnected() && false == client->connect() ) {
  //     error_log << QObject::tr("Нет подключения к БД");
  //     return false;
  //   }
  //  QString punktID;
  //  QStringList filters;
  //  if ( true == repl->has_coord() ) {
  //    const meteo::surf::Point& p = repl->coord();
  //    if (true == p.has_index()) {
  //      filters << util::field("point", { p.index() });
  //      getPunktId(QString::fromStdString(p.index()), &punktID);
  //    }
  //    if (true == p.has_name())
  //      filters << util::field("name", {p.name()});
  //    if (true == p.has_eng_name())
  //      filters << util::field("eng_name", {p.eng_name()});
  //    if (true == p.has_fi())
  //      filters << util::field("fi", p.fi());
  //    if (true == p.has_la())
  //      filters << util::field("la", p.la());
  //    if (true == p.has_height())
  //      filters << util::field("heigth", p.height());
  //  }
  //  if (repl->has_method_name())
  //    filters << util::field("method", {repl->method_name()});
  //  if (repl->has_time_start())
  //    filters << util::field("time_start", {repl->time_start()});
  //  if (repl->has_time_end())
  //    filters << util::field("time_end", {repl->time_end()});

  //  auto cursor = client->find( Find(collectionForecast, filters));

  //  if (nullptr != cursor){
  //    while (cursor->next()){
  //      QString document = cursor->getDocument();
  //      QJsonObject obj = QJsonDocument::fromJson(document.toUtf8()).object();
  //      meteo::forecast::ForecastResult* fresult = res->add_data();

  //      QString str = obj["dt"].toString();
  //      fresult->set_time_prod( str.toStdString() );//время разработки прогноза
  //      str = obj["val_descr"].toString();
  //      fresult->set_val_descr( str.toInt() );//дескриптор величины
  //      str = obj["flevel"].toString();
  //      fresult->set_flevel( str.toInt() );//изобарический уровень
  //      str = obj["type_level"].toString();
  //      fresult->set_type_level( str.toInt() );//тип уровня
  //      str = obj["dt_start"].toString();
  //      fresult->set_time_start( str.toStdString() );//время начала действия прогноза
  //      str = obj["dt_end"].toString();
  //      fresult->set_time_end( str.toStdString() );//время окончания действия прогноза
  //      str = obj["fvalue"].toString();
  //      fresult->set_value( str.toDouble() );//прогнозируемое значение
  //      str = obj["freal_value"].toString();
  //      fresult->set_freal_value( str.toDouble() );//реальное значение величины
  //      str = obj["freal_field_value"].toString();
  //      fresult->set_freal_field_value( str.toDouble() );//реальное значение из obanal
  //      str = obj["center"].toString();
  //      fresult->set_center( str.toInt() );
  //      str = obj["ftype_method"].toString();
  //      fresult->set_method_name( str.toStdString() );
  //      str = obj["forecast_text"].toString();
  //      fresult->set_forecast_text( str.toStdString() );//текст прогноза
  //      str = obj["ftype_element"].toString();
  //      fresult->set_ftype_element( str.toStdString() );
  //      str = obj["fkach_pr"].toString();
  //      fresult->set_fkach_pr( str.toInt() );
  //      str = obj["fflag_obr"].toString();
  //      fresult->set_fflag_obr( str.toInt() );
  //      str = obj["fhour"].toString();
  //      fresult->set_fhour( str.toInt() );
  //      str = obj["accuracy_field_date"].toString();
  //      fresult->set_accuracy_field_date( str.toStdString() );
  //      str = obj["accuracy_data_date"].toString();//срок поля с реальными данными
  //      fresult->set_accuracy_data_date( str.toStdString() );//срок реальных данных наблюдений

  //    }
  //    return true;
  //  } else {
  //    return false;
  //  }
  //  return false;
  /*
NS_PGBase* db = meteo::global::dbForecast();
if (0 == db || false == db->Connected() ) {
error_log<<msglog::kDbForecastNotReady;
return false;
}
if(!repl->has_coord()){
return false;
}
const meteo::surf::Point& p = repl->coord();
QString stindex = "NULL";
if( p.has_index()){
stindex =QString::fromStdString(p.index());
}
QString st_name = "NULL";
if(p.has_name()){
st_name =QString::fromStdString(p.name());
} else {
if(p.has_eng_name()){
  st_name =QString::fromStdString(p.eng_name());
}
}
QString stfi = "NULL";
if(p.has_fi()){
stfi =QString::number(p.fi());
}
QString stla = "NULL";
if(p.has_la()){
stla =QString::number(p.la());
}
QString sth = "NULL";
if(p.has_height()){
sth =QString::number(p.height());
}
QString punkt_id;
if(false == getPunktId(stindex, &punkt_id)){
return false;
}

QString method_name = "NULL";
if(repl->has_method_name()){
method_name =QString::fromStdString(repl->method_name());
}

QString start("NULL");
if ( true == repl->has_time_start() ) {
start = QString("'%1'")
.arg(QString::fromStdString( repl->time_start() ));
}

QString end("NULL");
if ( true == repl->has_time_start() ) {
end = QString("'%1'")
.arg(QString::fromStdString( repl->time_end() ));
}

TSqlQuery query(db);

query.setQuery("SELECT * from forecast where punkt_id = %1 and dt>= %2 and dt <= %3 and ftype_method = '%4';")
.arg(punkt_id)
.arg(start)
.arg(end)
.arg(method_name);

if ( false == query.exec() ) {
error_log << msglog::kDbRequestFailedArg.arg(query.query());
return false;
}

int result_size = query.size();
if(0 == result_size){
error_log << msglog::kForecastNoStation.arg("нет информации");
return false;
}


for ( int i = 0; i < result_size; ++i ) {
meteo::forecast::ForecastResult* fresult = res->add_data();
QString str = query.value( i, "dt" );
fresult->set_time_prod( str.toStdString() );//время разработки прогноза
str = query.value( i, "val_descr" );
fresult->set_val_descr( str.toInt() );//дескриптор величины
str = query.value( i, "flevel" );
fresult->set_flevel( str.toInt() );//изобарический уровень
str = query.value( i, "type_level" );
fresult->set_type_level( str.toInt() );//тип уровня
str = query.value( i, "dt_start" );
fresult->set_time_start( str.toStdString() );//время начала действия прогноза
str = query.value( i, "dt_end" );
fresult->set_time_end( str.toStdString() );//время окончания действия прогноза
str = query.value( i, "fvalue" );
fresult->set_value( str.toDouble() );//прогнозируемое значение
str = query.value( i, "freal_value" );
fresult->set_freal_value( str.toDouble() );//реальное значение величины
str = query.value( i, "freal_field_value" );
fresult->set_freal_field_value( str.toDouble() );//реальное значение из obanal
str = query.value( i, "center" );
fresult->set_center( str.toInt() );
str = query.value( i, "ftype_method" );
fresult->set_method_name( str.toStdString() );
str = query.value( i, "forecast_text" );
fresult->set_forecast_text( str.toStdString() );//текст прогноза
str = query.value( i, "ftype_element" );
fresult->set_ftype_element( str.toStdString() );
str = query.value( i, "fkach_pr" );
fresult->set_fkach_pr( str.toInt() );
str = query.value( i, "fflag_obr" );
fresult->set_fflag_obr( str.toInt() );
str = query.value( i, "fhour" );
fresult->set_fhour( str.toInt() );
str = query.value( i, "accuracy_field_date" );
fresult->set_accuracy_field_date( str.toStdString() );
str = query.value( i, "accuracy_data_date" );//срок поля с реальными данными
fresult->set_accuracy_data_date( str.toStdString() );//рок реальных данных наблюдений

}

// punkt_id bigint, -- id пункта из таблицы punkts
*/
}

//проверка совпадения прогнозов с реальными значениями
void TForecastDb::setOpravd(Array& reals, meteo::forecast::ForecastResult* forecast)
{
  if (reals.isEmpty()) return;
  	    
  Gradacii grresp;
  float delta = 999990.;
  // если у нас не дельты, а реальные градации из базы
  bool  gradation = false;

  if ( false == TGradaciiDB::getGradacii(forecast->flevel(), forecast->type_level(),
					 forecast->val_descr(), forecast->fhour(),
					 &grresp) ) {
    return;
  }

  // получаем дельты
  delta = grresp.delta();
  // увеличиваем их на величину соседней градации
  if ( grresp.dopusk_grad() > 0 ){
    delta += grresp.dopusk_grad();
  }
    
  // если находим градации, то выставляем флаг и обрабатываем дальше как градации
  // а не дельты
  if ( grresp.gradacii_min_size() > 0 ){
    gradation = true;
  }
  
  int count = 0;
  float near_value   = 0;
  float value_obs    = 0;
  QDateTime near_dt;
  QDateTime rdt;
  float forecast_value = forecast->value();
  
  forecast->set_opr(false);
  
  while ( reals.next() ) {
    Document item;
    // заполняем документ
    if (!reals.valueDocument(&item)) {
      continue;
    }
    
    if (false == item.hasField("dt")) {
      continue;
    }
    
    rdt = item.valueDt("dt");
	    
    if ( true == item.hasField("observ_value")) {
      value_obs  = item.valueDouble("observ_value");
    } else if (true == item.hasField("field_value")) {
      value_obs  = item.valueDouble("field_value");
    } else {
      continue;
    }

    count += 1;	   	    
       
    // если проверяем градации, а не дельты
    if ( gradation ) {
      int forecast_index = -1; // номер в который попал прогноз
      int real_index     = -1; // номер градации в который попало реальное значение
      
      //
      // проходимся по массиву верхних и нижних границ градации
      // находим попадания для прогнозов и для реальных значений
      // и если они отличаются больше чем на 1 - то не попали в прогноз
      //
      for (int i = 0; i < grresp.gradacii_min_size(); i++) {
	if ( grresp.gradacii_min(i) <= forecast_value &&
	     grresp.gradacii_max(i) >= forecast_value &&
	     forecast_index == -1 ){
	  forecast_index = i;
	}
	if ( grresp.gradacii_min(i) <= value_obs &&
	     grresp.gradacii_max(i) >= value_obs &&
	     real_index == -1 ){
	  real_index = i;
	}
      }
      
      //
      // если отличается на  градацию, то ок
      //
      if ( fabs(forecast_index-real_index) <= grresp.dopusk_grad() ){
	forecast->set_opr(true);
      }

    }
    // это дельты проверяем
    else{
      //var(delta);
      float real_delta = fabs(value_obs - forecast_value);
      
      //переход через 0 для направления ветра
      if (forecast->val_descr() == TMeteoDescriptor::instance()->descriptor("dd") &&
	  real_delta > 180) {
	real_delta = 360 - real_delta;
      }
      
      if (real_delta <= delta) {
	near_value = value_obs;
	near_dt    = rdt;
	delta      = real_delta;
	forecast->set_opr(true);
      }          
    }
  }
  
  if( !forecast->has_opr() || false == forecast->opr()){
    near_value = value_obs; //берем последнюю запись. может лучше взять середину интервала?
    near_dt    = rdt;
  }
  forecast->set_accuracy_data_date(near_dt.toString(Qt::ISODate).toStdString());
  forecast->set_freal_value( near_value );
  forecast->set_fflag_obr(count);

}



/**
* получаем результаты прогнозов
* по станции
*
* @param  repl [description]
* @param  res  [description]
* @return      [description]
*/
bool  TForecastDb::getForecastResult(const ForecastResultRequest* req, ForecastResultReply* reply)
{
  std::unique_ptr<Dbi> db(meteo::global::dbForecast());
  if ( nullptr == db.get() ) {
    error_log << QObject::tr("Ошибка при подключении к базе данных mongodbConfForecast");
    return false;
  }

  //var(req->Utf8DebugString());
  if (!req->has_coord() ||
      !req->coord().has_index() ||
      !req->coord().has_type()) {
    error_log << QObject::tr("Запрос getForecastResult заполнен не полностью");
    return false;
  }
  
  auto query = db->queryptrByName("get_forecast_result");
  if(nullptr == query) {return  false;}
  // устанавливаем значение координаты
  query->arg("station", req->coord().index()  );
  query->arg("station_type", req->coord().type());
 
  if ( req->has_time_start() ) {
    query->argDt("start_dt",req->time_start());
  }
  if ( req->has_time_end() ) {
    query->argDt("end_dt",req->time_end());
  }
  if ( 0 != req->has_method_name() ) {
    query->arg("method", req->method_name() );
  }
  if ( req->has_without_opravd() ) {
    query->arg("opravd", req->without_opravd());
  }

  //debug_log << query->query();
  
  QString error;
  if(false == query->execInit( &error)){
    //error_log << error;
    return false;
  }

  while ( true == query->next()) {
    const DbiEntry& doc = query->entry();
    // добавляем элемент
    int level      = doc.valueInt32("level");
    int type_level = doc.valueInt32("level_type");
    int hour       = doc.valueInt32("hour")/3600;
    // если есть параметры на станции
    if ( true == doc.hasField("param")) {
      // создаем новую структуру в протоответе
      auto forecast = reply->add_data();
      // заполняем ее

      // создаем документ, чтобы добыть параметры из элемента массива
      bool ok=false;
      Document paramDoc = doc.valueDocument("param",&ok);
      if(false == ok) { continue; }
      //debug_log << paramDoc.jsonExtendedString();
      // заполняем
      if ( true == paramDoc.hasField("method") ) {
	forecast->set_method_name(paramDoc.valueString("method").toStdString());
      }
      float forecast_value = -9999;
      if ( true == paramDoc.hasField("value") ) {
	forecast_value = paramDoc.valueDouble("value");
      }
      forecast->set_value(forecast_value);
      
      int descr =-1;
      if ( true == paramDoc.hasField("descr") ) {
	descr = paramDoc.valueInt32("descr");
      }
      forecast->set_val_descr(descr);
      
      if ( true == paramDoc.hasField("code") ) {
	forecast->set_forecast_text(paramDoc.valueString("code").toStdString());
      }

      forecast->set_flevel(level);
      forecast->set_type_level(type_level);
      forecast->set_fhour(hour);
      forecast->set_time_start(doc.valueDt("dt_beg").toString(Qt::ISODate).toStdString());
      forecast->set_time_end(doc.valueDt("dt_end").toString(Qt::ISODate).toStdString());
      forecast->set_time_prod(doc.valueDt("dt").toString(Qt::ISODate).toStdString());
      forecast->set_center(doc.valueInt32("center"));
      forecast->set_id(doc.valueOid("id").toStdString());

      ::meteo::surf::Point*  station = forecast->mutable_coord();
      Document stationDoc = doc.valueDocument("station_info",&ok);
      if(ok){
	station->set_name(stationDoc.valueString("name").toStdString());
	station->set_index( stationDoc.valueString("station").toStdString());
	station->set_type(stationDoc.valueInt32("station_type"));
	GeoPoint coords = stationDoc.valueGeo("location",&ok);
	if(ok){
	  station->set_fi(coords.fi());
	  station->set_la(coords.la());
	  station->set_height(coords.alt());
	}
      }
      // количество насчитанных значений
      if ( paramDoc.hasField("real") ) {
	Array reals = paramDoc.valueArray("real");
	setOpravd(reals, forecast);
      }
      // var(forecast->Utf8DebugString());      
    }

  }
  return true;
}


/*
bool TForecastDb::loadPunkts(::meteo::field::DataRequest *request){
  //NOT TESTED!
  auto db = meteo::global::dbForecast();
  if(nullptr == db ){
      warning_log << msglog::kDbForecastNotReady;
      return false;
    }
  if ( false == db->isConnected() && false == db->connect() ) {
      error_log << QObject::tr("Нет подключения к БД");
      delete db;
      return false;
    }
  NosqlQuery query;
  if ( false == meteo::global::kMongoQueriesNew.contains("get_all_punkts") ) {
      error_log << QObject::tr("Не удается найти шаблон запроса в базу данных: get_all_punkts");
      delete db;
      return false;
    }
  query.setQuery(meteo::global::kMongoQueriesNew["get_all_punkts"]);
  bool queryRes = db->execQuery( query.query());
  if (false == queryRes) {
      error_log << "Запрос get_all_punkts выполнен с ошибкой";
      delete db;
      return false;
    }

  while ( true == db->next() ) {
      Document document = db->document();
      meteo::surf::Point* pnt = request->add_coords();
      QString station_id = document.valueString("station_id");
      QString name = document.valueString("name");
      double fi = document.valueDouble("fi");
      double la = document.valueDouble("la");
      double height = document.valueDouble("height");
      //int id = ;

      pnt->set_index(station_id.toStdString());
      pnt->set_name(name.toStdString());
      pnt->set_fi(fi);
      pnt->set_la(la);
      pnt->set_height(height);
      //pnt->set_height(doc.object()["id"].toInt());
    }
  delete db;
  return true;
}
*/

void tryCalculateAccuracy(QJsonDocument doc){

  QJsonObject obj = doc.object();
  if (!obj.contains("punkt_id") ||
      !obj.contains("center") ||
      !obj.contains("val_descr") ||
      !obj.contains("flevel") ||
      !obj.contains("type_level") ||
      !obj.contains("fhour") ||
      !obj.contains("fvalue") ||
      !obj.contains("freal_field_value") ) {
      //No need to calculate accuracy
      return;
    }

  QString oid = obj["_id"].toObject()["$oid"].toString();
  QString newPunktId = obj["punkt_id"].toObject()["$oid"].toString();
  int newDocCenter = obj["center"].toInt();
  int docDescriptor = obj["val_descr"].toInt();
  int docLevel = obj["flevel"].toInt();
  int docTypeLevel = obj["type_level"].toInt();
  QString docTypeMethod = obj["ftype_method"].toString();
  int newDocHour = obj["fhour"].toInt();
  double newDocFValue = obj["fvalue"].toDouble();
  double newDocFrealValue = obj["freal_field_value"].toDouble();

  accuracy_calc(oid, newPunktId, newDocCenter, docDescriptor, docLevel, docTypeLevel, docTypeMethod, newDocHour, newDocFValue, newDocFrealValue);
  return;
}

bool TForecastDb::saveManyForecasts(const ForecastManyPointsData *data){
  QTime ttt; ttt.start();
  QStringList allForecasts;
  for (int i = 0; i < data->points_size(); ++i){
      QString singleForecast = generateSaveForecastQUitem(&data->points(i));
      if (!singleForecast.isEmpty()) {
          allForecasts << singleForecast;
        } else {
          warning_log << QObject::tr("При разборе значения forecast возникла ошибка");
        }
    }

  if (allForecasts.isEmpty()){
      error_log << QObject::tr("Нет данных для запроса, выход");
      return false;
    }


  std::unique_ptr<Dbi> db(meteo::global::dbForecast());
  if ( nullptr == db.get() ) {
    error_log << QObject::tr("Ошибка при подключении к базе данных Forecast");
    return false;
    }
  auto query = db->queryptrByName("insert_forecast");
  if(nullptr == query) {return  false;}

  query->arg("collection",collectionForecast);
  query->argJson("updates",allForecasts.join(" , "));


  if (!query->exec( )) {
      error_log << QObject::tr("Ошибка при выполнении запроса в базе данных");
      return false;
    }

  //int cur =  ttt.elapsed();
  //debug_log << msglog::kServiceRequestTime.arg(query.query()).arg(cur);
  return true;
}

QString TForecastDb::generateSaveForecastQUitem(const meteo::forecast::ForecastPointData* d){
  if ( !d->has_coord() ||
       !d->coord().has_index() ||
       !d->has_fdesc() ||
       !d->fdesc().has_date() ||
       !d->fdesc().has_meteodescr() ||
       !d->fdesc().has_level() ||
       !d->fdesc().has_level_type() ||
       !d->has_value() ||
       !d->has_fdesc() ||
       !d->fdesc().has_center() ||
       !d->fdesc().has_hour() ){
      error_log << QObject::tr("Отсуствует одно из обязательных полей");
      return QString();
    }
  QString dtStart, dtEnd;

  if ( d->fdesc().has_dt1() && d->fdesc().has_dt2() ){
      dtStart = QString::fromStdString(d->fdesc().dt1());
      dtEnd = QString::fromStdString(d->fdesc().dt2());
    } else if ( d->has_time_start() && d->has_time_end()) {
      dtStart = QString::fromStdString(d->time_start());
      dtEnd = QString::fromStdString(d->time_end());
    } else {
      error_log << QObject::tr("Отсуствует одно из обязательных полей");
      return QString();
    }
  QTime ttt; ttt.start();

  double value = d->value();
  QString text_value = d->has_text_value() ? QString::fromStdString(d->text_value()) :
                                             QString::number( value,'f',2 );

  bool ok(false);
  text_value.toDouble(&ok);
  if ( ok ){
      text_value = QString::number( text_value.toDouble(),'f',2 );
    }

  int val_descr        = d->fdesc().meteodescr();
  QString name_descr   = TMeteoDescriptor::instance()->name(val_descr);
  int level            = d->fdesc().level();
  int level_type       = d->fdesc().level_type();
  int center           = d->fdesc().center();
  int hour             = d->fdesc().hour();
  QString station_indx = QString::fromStdString(d->coord().index());
  int station_type = d->coord().type();

  QString method_name = d->has_method_name()? QString::fromStdString(d->method_name()) :
                                              QString("%1 %2 %3 %4").arg(center).arg(level).arg(level_type).arg(hour);


  auto punkt = PunktDB::getPunktByStationIndex(station_indx, station_type);
  if (!punkt.has_id()) {  //if(!getPunktId(station_indx, &punkt_id )){
    info_log << msglog::kForecastNoStation.arg(station_indx);
    
    QString punktName = QString::fromStdString(d->coord().has_name()? d->coord().name() : d->coord().eng_name());
    double fi = MnMath::rad2deg( d->coord().fi() );
    double la = MnMath::rad2deg( d->coord().la() );
    double height = d->coord().height();
    
    bool isActive = false;
    
    if (!PunktDB::addForecastPunkts(punktName, fi, la, height, station_indx, station_type, isActive)) {
      error_log << msglog::kForecastNoStation.arg(station_indx);
      return QString();
    }
  }
  QString punkt_id   = QString::fromStdString(punkt.id());
  QDateTime dt       = QDateTime::fromString(QString::fromStdString(d->fdesc().date()),Qt::ISODate);
  QDateTime dt_start = NosqlQuery::datetimeFromString(dtStart);
  QDateTime dt_end   = NosqlQuery::datetimeFromString(dtEnd);
  meteo::GeoPoint gp = meteo::GeoPoint::fromDegree(punkt.fi(), punkt.la(), punkt.height());

  std::unique_ptr<Dbi> db(meteo::global::dbForecast());
  if ( nullptr == db.get() ) {
    error_log << QObject::tr("Ошибка при подключении к базе данных Forecast");
    return QString();
    }

  if ( false == meteo::global::kMongoQueriesNew.contains("insert_forecast_qu") ) {
    error_log << QObject::tr("Не удается найти шаблон запроса в базу данных: insert_forecast_qu");
    return QString();
  }
  NosqlQuery query;
  query.setQuery(meteo::global::kMongoQueriesNew["insert_forecast_qu"]);
  //  auto query = db->queryptrByName("insert_forecast_qu");
  //  if(nullptr == query) {return QString();}

  query.arg("punkt_id",punkt_id);
  query.arg("dt",dt);
  query.arg("dt_beg",dt_start);
  query.arg("dt_end",dt_end);
  query.arg("level",level);
  query.arg("level_type",level_type);
  query.arg("center",center);
  query.arg("hour",hour);
  query.arg("name",QString::fromStdString(punkt.name()));
  query.arg("station",QString::fromStdString(punkt.stationid()));
  query.arg("location",gp);
  query.arg("station_type",punkt.station_type());
  query.arg("descrname",name_descr);
  query.arg("descr",val_descr);
  query.arg("value",value);
  query.arg("code",text_value);

  if (d->has_method_name()){
    query.arg("method",method_name);
  } else {
    query.arg("method",QString(""));
  }

  if (d->has_method_title()){
      query.arg("method_title",QString::fromStdString(d->method_title()));
    }else{
      query.arg("method_title",QString(""));
    }

  if (d->fdesc().has_model()) {
    query.arg("model",d->fdesc().model());
  } 

  query.removeDogs();
  
  return query.query();
}

bool TForecastDb::saveForecast(const meteo::forecast::ForecastPointData* d){
  QTime ttt; ttt.start();
  QString singleForecast = generateSaveForecastQUitem(d);
  if (singleForecast.isEmpty()){
      return false;
    }
  QStringList allForecasts;
  allForecasts << singleForecast;

  std::unique_ptr<Dbi> db(meteo::global::dbForecast());
  if ( nullptr == db.get() ) {
    error_log << QObject::tr("Ошибка при подключении к базе данных Forecast");
    return false;
    }
  auto query = db->queryptrByName("insert_forecast");
  if(nullptr == query) {return false;}

  query->arg("collection",collectionForecast);
  query->argJson("updates",allForecasts.join(" , "));

  if (!query->exec()) {
      error_log << QObject::tr("Ошибка при выполнении запроса в базе данных");
      return false;
    }

  // const meteo::DbiEntry& result = query->result();
  // if ( !result.hasField("ok") || 1 != result.valueDouble("ok") ){
  //     error_log << QObject::tr("Ошибка при выполнении запроса в базу данных");
  //     return false;
  //   }
  int cur =  ttt.elapsed();
  if (cur > 100) {
      debug_log << msglog::kServiceRequestTime.arg(query->query()).arg(cur);
    }

  return true;
}


bool TForecastDb::savecAccuracyForecast(const meteo::field::OnePointData* d){
  if ( !d->has_value() ||
       !d->has_fdesc() ||
       !d->fdesc().has_meteodescr() ||
       !d->fdesc().has_level_type() ||
       !d->has_coord() ||
       !d->coord().has_index() ||
       !d->coord().has_type()) {
      error_log << "Error: missing required fields";
      return false;
    }
  QTime ttt; ttt.start();

  double fvalue = d->value();
  int val_descr = d->fdesc().meteodescr();
  int flevel = d->fdesc().level();
  int type_level = d->fdesc().level_type();
  QDateTime dt = QDateTime::fromString(QString::fromStdString(d->fdesc().date()),Qt::ISODate);
  QString station_indx = QString::fromStdString(d->coord().index());
  int station_type = d->coord().type();
  QString descrname = TMeteoDescriptor::instance()->name(val_descr);

  if (!d->fdesc().has_center()){
    return false;
  }

 std::unique_ptr<Dbi> db(meteo::global::dbForecast());
  if ( nullptr == db.get() ) {
    error_log << QObject::tr("Ошибка при подключении к базе данных Forecast");
    return false;
    }
  auto query = db->queryptrByName("insert_forecast_accuracy");
  if(nullptr == query) {return false;}

 query->arg("center",d->fdesc().center());
 query->arg("dt_beg",dt);
 query->arg("dt_end",dt);
 query->arg("level",flevel);
 query->arg("level_type",type_level);
 query->arg("station",station_indx);
 query->arg("station_type",station_type);
 query->arg("descrname",descrname);
 query->arg("field_value",fvalue);
 query->arg("dt",dt);
  //   if (!d->fdesc().has_center())   {//!FIXME
  //    if (false == meteo::global::kMongoQueriesNew.contains("insert_forecast_accuracy_without_center") ){
  //      error_log << QObject::tr("Не удается найти шаблоны запроса в базу данных: insert_forecast_accuracy_without_center");
  //      return false;
  //    }
  //      query.setQuery(meteo::global::kMongoQueriesNew["insert_forecast_accuracy_without_center"]);
  //      query->arg("",punkt_id)
  //          .arg(dt.addSecs(FORECAST_TIME))
  //          .arg(dt)
  //          .arg(flevel)
  //          .arg(type_level)
  //          .arg(val_descr)
  //          .arg(fvalue);
  //    }
  QTime ttt_insert; ttt_insert.start();
  if (!query->exec()) {
      error_log << QObject::tr("Ошибка при выполнении запроса в базе данных");
      return false;
    }

  const meteo::DbiEntry& result = query->result();
  if (1 != result.valueDouble("ok")){
      error_log << "Запрос в базу данных завершился с ошибкой";
      return false;
    }
  //debug_log << QObject::tr("savecAccuracyForecast выполнен за %1 мсек").arg(ttt.elapsed());
  return true;
}


bool TForecastDb::setOpr( const QStringList& ids, bool opr)  {
  if (ids.isEmpty()) return  false;
 std::unique_ptr<Dbi> db(meteo::global::dbForecast());
  if ( nullptr == db.get() ) {
    error_log << QObject::tr("Ошибка при подключении к базе данных Forecast");
    return false;
    }
  auto query = db->queryptrByName("update_forecast_set_opr_value");
  if(nullptr == query) {return false;}

  query->argOid("id",ids);
  query->arg("opr",opr);

  if (!query->exec()) {
      error_log << QObject::tr("Ошибка при выполнении запроса в базе данных");
      return false;
    }

  const meteo::DbiEntry&  result = query->result();
  if (1 != result.valueDouble("ok")){
      error_log << "Запрос в базу данных завершился с ошибкой";
      return false;
    }
  return true;
}


bool TForecastDb::setError(const QStringList& ids)  {
 std::unique_ptr<Dbi> db(meteo::global::dbForecast());
  if ( nullptr == db.get() ) {
    error_log << QObject::tr("Ошибка при подключении к базе данных Forecast");
    return false;
    }
  auto query = db->queryptrByName("update_forecast_set_error");
  if(nullptr == query) {return false;}
   query->argOid("id",ids);

  if (query->exec()) {
      error_log << QObject::tr("Ошибка при выполнении запроса в базе данных");
      return false;
    }

  const meteo::DbiEntry& result = query->result();
  if (!result.hasField("ok") || 1 != result.valueDouble("ok")){
      error_log << "Запрос в базу данных завершился с ошибкой";
      return false;
    }
  return true;
}

bool TForecastDb::getForecastAV12( const ForecastAv12Request* req, ForecastAv12Responce* res)
{
  if (!req->has_dt() ||
      !req->has_hour() ||
      !req->has_station_index()){
      QString msg = QObject::tr("Отсутствует часть обязательных полей.");
      res->set_msg(msg.toStdString());
      res->set_success(false);
      error_log << msg;
      return false;
    }

  int hour = req->hour();
  QDateTime dt = NosqlQuery::datetimeFromString(req->dt());
  QString stationIndex = QString::fromStdString(req->station_index());

  if (!dt.isValid()) {
      QString msg = QObject::tr("Неверный формат даты");
      res->set_msg(msg.toStdString());
      res->set_success(false);
      error_log << msg;
      return false;
    }

  QString reply = QString("Прогноз на %1 сроком на %2 по станции %3: [Грозы местами]")
      .arg(dt.toString(Qt::ISODate))
      .arg(hour)
      .arg(stationIndex);
  res->set_msg(reply.toStdString())  ;
  res->set_success(true);
  return true;
}


/**
 * @brief Сохранение ручных прогнозов, составленных синоптиком
 * 
 * @param data 
 * @return true 
 * @return false 
 */
bool TForecastDb::saveManualForecast( const ForecastManualData*             data, 
                                      ForecastManualReply* response){
  if ( !data->has_user_id()         ||
       !data->has_dt()              ||
       !data->has_dt_start()        ||
       !data->has_dt_end()          ||
       !data->has_forecast_hour()   ||
       !data->has_forecast_text()   
        ){
      error_log << "Error: missing required fields";
      return false;
    }

  QTime ttt; ttt.start();  

  std::unique_ptr<Dbi> db(meteo::global::dbForecast());
  if ( nullptr == db.get() ) {
    error_log << QObject::tr("Ошибка при подключении к базе данных Forecast");
    return false;
  }

  auto query = db->queryptrByName("insert_manual_forecast");
  if(nullptr == query) {return false;}

  // если иммется в запросе айдишник - значит пытаемся 
  // изменить запись, а не создавать новую
  // это прописано в функции постгреса meteo.insert_manual_forecast
  if ( data->has_id() ){
    query->arg("id"      , data->id()       );
  }
  if ( data->has_region_id() ){
    query->arg("region_id"      , data->region_id()       );
  }
  if ( data->has_station_id() ){
    query->arg("station_id"     , data->station_id()      );
  }
  query->arg("user_id"        , data->user_id()         );
  QDateTime dt = QDateTime::fromString(QString::fromStdString(data->dt()),Qt::ISODate);
  query->arg("dt"             , dt                      );
  dt = QDateTime::fromString(QString::fromStdString(data->dt_start()),Qt::ISODate);
  query->arg("dt_start"       , dt                      );
  dt = QDateTime::fromString(QString::fromStdString(data->dt_end()),Qt::ISODate);
  query->arg("dt_end"         , dt                      );
  query->arg("forecast_hour"  , data->forecast_hour()   );
  query->arg("forecast_code"  , data->forecast_code()   );
  query->arg("forecast_text"  , data->forecast_text()   );
  query->argJson("forecast_params", QString::fromStdString(data->forecast_params()) );
  
  if ( !query->exec() ) {
    error_log << QObject::trUtf8("Ошибка при выполнении запроса в базе данных");
    return false;
  }

  // забираем последний адишник
  int last_id = 0;
  if ( query->next()){
    const DbiEntry& doc = query->entry();
    if (doc.hasField("last_id")) {
      last_id = doc.valueInt32("last_id");
    }
  }
  
  // получаем добавленную запись
  ForecastManualRequest request_get = ForecastManualRequest();
  request_get.set_id(last_id);
  getManualForecastResult(&request_get,response);

  int cur =  ttt.elapsed();
  if (cur > 100) {
    debug_log << msglog::kServiceRequestTime.arg(query->query()).arg(cur);
  }

  return true;
}

/**
 * @brief Get the Manual Forecast Result object
 * Получение ручных прогнозов, составленных синоптиком
 * 
 * @param request 
 * @param response 
 * @return true 
 * @return false 
 */
bool TForecastDb::getManualForecastResult(  const ForecastManualRequest* request, 
                                            ForecastManualReply* response){

  std::unique_ptr<Dbi> db(meteo::global::dbForecast());
  if ( nullptr == db.get() ) {
    error_log << QObject::tr("Ошибка при подключении к базе данных Forecast");
    return false;
  }

  auto query = db->queryptrByName("get_manual_forecast_result");
  if(nullptr == query) { return  false; }
  // устанавливаем значение координаты
  
  if ( request->has_id() ) {
    query->arg("id", request->id());
  }else{
    if ( request->region_id_size()>0 ) {
      query->arg("region_id", request->region_id() );
    }
    if ( request->has_user_id() ) {
      query->arg("user_id", request->user_id());
    }
    if ( request->has_only_last() ) {
      query->arg("only_last", request->only_last());
    }
    if ( request->has_dt() ) {
      query->argDt("dt", request->dt());
    }
    if ( request->has_dt_start() ) {
      query->argDt("dt_start", request->dt_start());
    }
    if ( request->has_dt_end() ) {
      query->argDt("dt_end", request->dt_end());
    }
    if ( request->has_forecast_hour() ) {
      query->arg("forecast_hour", request->forecast_hour());
    }
    // это даты, для поиска от и до, за заданный период
    // даты для поиска  срока начала действия прогноза
    // AND ( @from_dt@       IS NULL OR dt_start     >= @from_dt@       )
    // AND ( @to_dt@         IS NULL OR dt_start     <= @to_dt@         )
    if ( request->has_from_dt() ) {
      query->argDt("from_dt", request->from_dt());
    }
    if ( request->has_to_dt() ) {
      query->argDt("to_dt", request->to_dt());
    }
    
    if ( request->station_size()>0 ) {
      QList<QString> stations;
      QList<int> stations_types;
      for (int i = 0; i < request->station_size(); ++i){
        QString st = QString::fromStdString(request->station(i).station());
        if ( !st.isEmpty() ){
          stations.append( QString::fromStdString(request->station(i).station()) );
          stations_types.append( request->station(i).type() );
        }
      }
      query->arg("stations" ,  stations  );
      query->arg("stations_types" , stations_types  );
    }
  }

  // debug_log<<query->query();

  QString error;
  if(false == query->execInit( &error ) ){
    error_log << error;
    return false;
  }

  while ( true == query->next()) {
    const DbiEntry& doc = query->entry();
    // если есть параметры на станции
    if ( true == doc.hasField("id")) {
      // создаем новую структуру в протоответе
      auto forecast = response->add_data();
      // заполняем ее
      forecast->set_id(              doc.valueInt32(  "id"             ) );
      forecast->set_region_id(       doc.valueInt32(  "region_id"      ) );
      forecast->set_region_name(     doc.valueString( "region_name"    ).toStdString() );
      forecast->set_user_id(         doc.valueInt32(  "user_id"        ) );
      forecast->set_dt(              doc.valueDt(     "dt"             ).toString(Qt::ISODate).toStdString() );
      forecast->set_dt_start(        doc.valueDt(     "dt_start"       ).toString(Qt::ISODate).toStdString() );
      forecast->set_dt_end(          doc.valueDt(     "dt_end"         ).toString(Qt::ISODate).toStdString() );
      forecast->set_forecast_hour(   doc.valueInt32(  "forecast_hour"  ) );
      forecast->set_forecast_code(   doc.valueString( "forecast_code"  ).toStdString());
      forecast->set_forecast_text(   doc.valueString( "forecast_text"  ).toStdString());
      forecast->set_forecast_params( doc.valueString( "forecast_params").toStdString());
      if ( doc.hasField("station") ){
        auto s = doc.valueDocument("station");
        if ( s.hasField("station") ){
          forecast->set_station(      s.valueString(  "station"     ).toStdString() );
          forecast->set_station_type(      s.valueInt32(  "type"     ) );
        }
      }
      forecast->set_station_name(    doc.valueString( "station_name"   ).toStdString() );
	  }
  }
  return true;
}




} // namespace forecast
} // namespace meteo



