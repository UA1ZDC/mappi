#include "methodbag.h"
#include "tforecastservice.h"
#include "tgradaciidb.h"

#include <meteo/commons/global/global.h>
#include <meteo/commons/global/dbnames.h>
#include <commons/meteo_data/tmeteodescr.h>

namespace meteo {
namespace forecast {

MethodBag::MethodBag( TForecastService* s )
  : QObject(),
  service_(s)
{
}

MethodBag::~MethodBag()
{
}

/***********************

 *      Методы
 *
 ************************/

/**
 * получаем оправдываемость
 * @param param    [description]
 * @param response [description]
 */
void MethodBag::GetAccuracy( QPair< rpc::ClientHandler*, const AccuracyRequest* > param, AccuracyReply* response )
{

  const AccuracyRequest* request = param.second;
  if ( nullptr == request || nullptr == response ) {
    error_log << QObject::tr("Неправильно заполнен параметр в функции. Невозможно получить оправдываемость прогнозов.");
    response->set_result(false);
    return;
  }
   //
   // формируем запрос
   // в базу
   //
   //
   // по каким пунктам забиираем резульаты прогнозов
   //
   // заполняем запрос

 std::unique_ptr<Dbi> db(meteo::global::dbForecast());
  if ( nullptr == db.get() ) {
    error_log << QObject::tr("Ошибка при подключении к базе данных Forecast");
    return ;
    }
  auto query = db->queryptrByName("get_forecast_opr_forecastprocess");
  if(nullptr == query) {return ;}
    // устанавливаем значения
    if ( request->has_center()  ){
      query->arg("center",request->center());
    }
    if ( request->has_punkt() && request->punkt().has_index() ){
      query->arg("punkt_id",request->punkt().index());
    }
    if ( request->has_descrname() ) {
      query->arg("descrname", request->descrname());
    }
    if ( request->has_level() ){
      query->arg("level",request->level());
    }
    if ( request->has_type_level() ){
      query->arg("type_level",request->type_level());
    }

    if ( request->has_method() ){
      query->arg("ftype_method",request->method());
    }

    // выполняем запрос
  QString error;
  if(false == query->execInit( &error)){
      //error_log << error;
      return;
    }
   int opr = 0;
   int not_opr = 0;
   int total = 0;
   response->set_opr(0);
   response->set_not_opr(0);
   response->set_total(0);
   // проходимся по ответу и считаем сколько там
   // оправдалось, сколько не очень
  while ( true == query->next()) {
      const DbiEntry& adoc = query->entry();
      opr     += adoc.valueInt32("opr");
      not_opr += adoc.valueInt32("not_opr");
      // total   += adoc.valueInt32("count_progn");
      total   += adoc.valueInt32("total");
    }

   response->set_opr( opr );
   response->set_not_opr( not_opr );
   response->set_total( total );
   response->set_method( request->method() );
   response->set_msg("OK");
   response->set_result(true);

 return;
}


/**
 *  запрашиваем данные оправдываемости по всем методам по одной станции
 * @param param [description]
 * @param resp  [description]
 */
void MethodBag::GetManyAccuracy( QPair< rpc::ClientHandler*, const ManyAccuracyRequest* > param, ManyAccuracyReply* response ){

  const ManyAccuracyRequest* request = param.second;
  if ( nullptr == request || nullptr == response ) {
    QString error = QObject::tr("Неправильно заполнен параметр в функции. Невозможно получить оправдываемость прогнозов.");
    error_log << error;
    response->set_comment( error.toStdString() );
    response->set_result(false);
    return;
  }

  if ( !request->has_punkt() ||
      (
       !request->punkt().has_index() &&
       !request->punkt().has_fi() &&
       !request->punkt().has_la()
      )
     ){
    error_log << QObject::tr("В запросе не указаны ни пункт, ни координаты, по которым производятся расчеты");
    return ;
  }
 std::unique_ptr<Dbi> db(meteo::global::dbForecast());
  if ( nullptr == db.get() ) {
    error_log << QObject::tr("Ошибка при подключении к базе данных Forecast");
    return ;
    }
  auto query = db->queryptrByName("get_manyforecast_opr_forecastprocess");
  if(nullptr == query) {return ;}

  // заполняем индекс
  if ( request->punkt().has_index()  ){
    query->arg("punkt_id",request->punkt().index());
  }
  query->arg("ftype_method", request->methods() );
  QString error;
  if(false == query->execInit( &error)){
      //error_log << error;
      return;
    }

  // проходимся по ответу и считаем сколько там
  // оправдалось, сколько не очень

  // заполняем ответ
  while ( true == query->next()) {
    auto acc = response->add_accuracy();
    const DbiEntry& adoc = query->entry();
    acc->set_opr( adoc.valueInt32("opr") );
    acc->set_not_opr( adoc.valueInt32("not_opr") );
    acc->set_total( adoc.valueInt32("total") );
    acc->set_method( adoc.valueString("ftype_method").toStdString() );
  }
  response->set_comment("OK");
  response->set_result(true);
 return;
}

  bool MethodBag::setOprFlags(const QList<uint64_t>& ids,
			      const QList<uint64_t>& opr_ids,
			      const QList<uint64_t>& not_opr_ids)
  {
    std::unique_ptr<Dbi> db(meteo::global::dbForecast());
    if ( nullptr == db.get() ) {
      error_log << QObject::tr("Ошибка при подключении к базе данных Forecast");
      return false;
    }
        
    auto query = db->queryptrByName("insert_forecast_flag_opravd");
    if(nullptr == query) {
      return false;
    }
    
    query->arg("id", ids);
    query->arg("opr_id", opr_ids);
    query->arg("not_opr_id", not_opr_ids);
    
    //var(query->query());
    if ( false == query->exec() ) {
      error_log << QObject::tr("Ошибка при выполнении запроса к базе данных");
      return false;
    }

    return true;
  }


/**
 *  Считаем оправдавшиеся прогнозы по станции
 * @param param    [description]
 * @param response [description]
 */
void MethodBag::CalcForecastOprStation(QPair< rpc::ClientHandler*, const meteo::forecast::AccuracyRequest* > param,
                                        meteo::field::SimpleDataReply* response )
{
  const meteo::forecast::AccuracyRequest* req_point = param.second;

  if ( nullptr == req_point || nullptr == response ) {
    QString error = QObject::tr("Неправильно заполнен параметр в функции. Невозможно получить оправдываемость прогнозов.");
    error_log << error;
    response->set_comment( error.toStdString() );
    response->set_result(false);
    return;
  }

  //var(req_point->Utf8DebugString());

  //
  // 2. запрашиваем прогнозы по станции
  //

  // сам запрос, котоорый надо заполнить
  // заполняем пока тока станцию
  meteo::forecast::ForecastResultRequest freq;

  //пересчитываем оправдываемость для прогнозов, действие которых закончилось 6 ч назад (на случай ещё приходящих запоздавших данных)
  QDateTime dt_end = QDateTime::currentDateTimeUtc().addSecs(-6*3600);
  
  ::meteo::surf::Point* station_coord = freq.mutable_coord();
  station_coord->set_index(  req_point->punkt().index()  );
  station_coord->set_type(   req_point->punkt().type()  );
  freq.set_time_end(dt_end.toString(Qt::ISODate).toStdString());
  freq.set_without_opravd(true);

  QList<uint64_t> ids;      //id учтенных прогнозов
  QList<uint64_t> opr_ids;   //id оправдавшихся прогнозов
  QList<uint64_t> not_opr_ids;//id не оправдавшихся прогнозов
  //
  // массив методов, для которых будем считать оправдываемость
  //
  QList<UniqOprKey> methods;
  // оправдываемость для каждого метода
  // оправдалось прогнозов
  QList<int> opr_arr;
  QList<int> not_opr_arr;
  QList<int> total_arr;
  // текущий индекс
  int current_index = 0;

  // делаем запрос
  //
  // TForecastDb::getForecastResult

  TForecastDb *fdb_          = new TForecastDb();
  ForecastResultReply* reply = new ForecastResultReply();
  fdb_->getForecastResult(&freq,reply);

  // var(freq.Utf8DebugString());
  // var(reply->Utf8DebugString());

  
  if ( nullptr == reply ) {
    error_log << QObject::tr("Нет ответа от GetForecastResult");
  } else {

    // проходимся по ответу, получаем прогнозы
    for(int i = 0; i < reply->data_size() ; ++i) {
      auto forecast = reply->data(i);     
      if (!forecast.has_id()) {
	continue;
      }
      
      long long id = std::stol(forecast.id());
      if (id == 0) {
	continue;
      }
      
      ids << id;
      
      //var(forecast.Utf8DebugString());
      UniqOprKey key;
      key.method = QString::fromStdString(forecast.method_name());
      key.hour   = forecast.fhour();
      key.center = forecast.center();
      key.level  = forecast.flevel();
      key.type_level = forecast.type_level();
      key.descr  = forecast.val_descr();
      //
      // если такого метода еще нет в списке - добавляем
      //
      current_index = methods.indexOf(key);
      if ( current_index < 0 ){
        methods     <<  key;
        opr_arr     <<  0;
        not_opr_arr <<  0;
        total_arr   <<  0;	
        current_index = methods.indexOf(key);
      }

      if(forecast.has_opr()) { //если проводилась проверка
	if (true == forecast.opr()){
          // попали в градацию, все ок
          opr_arr[current_index]++;
	  opr_ids << id;
        } else {
          not_opr_arr[current_index]++;
	  not_opr_ids << id;
          // error_log<<"Хрен! не попали! теперь чило неоправдавшихся прогнозов: "<< not_opr_arr[current_index];
        }
      }
      total_arr[current_index]++;     
    }

  }

  // отключаемся
  if(nullptr != fdb_){
    delete fdb_; fdb_ = nullptr;
  }
  if(nullptr != reply){
    delete reply; reply = nullptr;
  }

  if (ids.isEmpty()) {
    return;
  }
  
  //
  //  5. записываем в базу результаты по станции
  //

  if (!setOprFlags(ids, opr_ids, not_opr_ids)) {
    return;
  }
  
  std::unique_ptr<Dbi> db(meteo::global::dbForecast());
  if ( nullptr == db.get() ) {
    error_log << QObject::tr("Ошибка при подключении к базе данных Forecast");
    return ;
    }

  //var(methods.size());

  
  // проходимся по методам и сохраняем в базу
  for (int current_index = 0; current_index < methods.size(); ++current_index)
    {
      // заполняем запрос
      auto query = db->queryptrByName("insert_forecast_opravd");
      if(nullptr == query) {return ;}
      // устанавливаем значения
      query->arg("punkt_id",  req_point->punkt().index());
      query->arg("center",    methods[current_index].center);
      query->arg("descrname", TMeteoDescriptor::instance()->name(methods[current_index].descr));
      query->arg("level",     methods[current_index].level);
      query->arg("type_level", methods[current_index].type_level);
      query->arg("hour",       methods[current_index].hour);
      query->arg("ftype_method", methods[current_index].method);
      query->arg("opr",       opr_arr[current_index] );
      query->arg("not_opr",   not_opr_arr[current_index] );
      query->arg("total",     total_arr[current_index] );
      
      //var(query->query());
      
      // выполняем запрос
      if ( false == query->exec() ) {
	error_log << QObject::tr("Ошибка при выполнении запроса к базе данных");
	// return ;
      }
    }
  
}


// Метод сохранения введенных вручную прогнозов погоды по региону или по станции 
void MethodBag::SaveManualForecast( QPair< rpc::ClientHandler*, const meteo::forecast::ForecastManualData* > param, 
                                    meteo::forecast::ForecastManualReply* response )
{
  const meteo::forecast::ForecastManualData* request = param.second;
  if ( nullptr == request || nullptr == response ) {
    QString error = QObject::tr("Неправильно заполнен параметр в функции. Невозможно получить оправдываемость прогнозов.");
    error_log << error;
    response->set_comment( error.toStdString() );
    response->set_result(false);
    return;
  }
  
  TForecastDb *fdb_          = new TForecastDb();
  // последний айдишник или айдишник измененной записи

  if( fdb_->saveManualForecast(request, response) ){
    response->set_result("All ok");
    response->set_comment( QObject::tr("Запрос к сервису saveManualForecast выполнен успешно!").toStdString() );
  }

  // отключаемся
  if(nullptr != fdb_){
    delete fdb_; fdb_ = nullptr;
  }
  return;
}


// Метод получения прогнозов введенных вручную прогнозов погоды по региону или по станции 
void MethodBag::GetManualForecastResult( QPair< rpc::ClientHandler*, const meteo::forecast::ForecastManualRequest* > param, 
                                          meteo::forecast::ForecastManualReply* response )
{
  const meteo::forecast::ForecastManualRequest* request = param.second;
  if ( nullptr == request || nullptr == response ) {
    QString error = QObject::tr("Неправильно заполнен параметр в функции. Невозможно получить оправдываемость прогнозов.");
    error_log << error;
    response->set_comment( error.toStdString() );
    response->set_result(false);
    return;
  }

  TForecastDb *fdb_          = new TForecastDb();

  fdb_->getManualForecastResult(request,response);

  // отключаемся
  if(nullptr != fdb_){
    delete fdb_; fdb_ = nullptr;
  }
  
}


// endnamespace
}
}
