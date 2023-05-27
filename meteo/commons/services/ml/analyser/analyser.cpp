#include "analyser.h"
#include <meteo/commons/proto/field.pb.h>
#include <meteo/commons/global/global.h>

namespace meteo {
namespace ml {

Analyser::Analyser()
{
  // дату для запроса данных
  req_date_ = new QDateTime(QDateTime::currentDateTimeUtc());

  // класс работы с базой прогностических полей
  //db_ = new meteo::field::TFieldDataDb();

  // инициализируем переменные
  clearData();
}

Analyser::~Analyser()
{
 // delete db_;
  delete req_date_;
  delete result_field_;
  delete all_field_;
  delete prepared_data_;
  delete substracted_data_;
  delete all_data_;
  delete alldelta_field_;
  delete substracted_delta_data_;
  delete substracted_grib_data_;
  delete allgrib_field_;
}



/**
 *
 * Функция расчета коэффициентов корелляции прогностических и фактических полей
 * 1. запрос фактических данных
 * 2. вырезаем из данных регион
 * 3. запрос прогностических данных на 12 часов по неосвещенной территории
 * 4. заполнение вырезанного квадрата данными грибов
 * 5. запрос из БД дельты (TField)
 * 6. заполнение вырезанного квадрата данными дельт
 * 7. преобразование дельты в коэф. корреляции
 * 8. заполнение весовых коэффициентов для прогностических данных
 * 9. смешивание в одной all_data данных наблюдений и скорректированных прогностических данных с весовыми коэффициентами
 * 10. считаем разницу получившегося винегрета с данными фактических измерений
 *
 */
void Analyser::startForecastCustomRegionCalculate(){

  /**
   *  1.запрос фактических данных ======================================================================
   */
  // запрашиваем фактические данные
  getFactDataFromSRV();

  /**
   *  2.вырезаем регион ================================================================================
   */
  // выкалываем данные из массива по региону
  // сохраняем их в отдельной переменной
  // вырезаем регион
  substractRegionFromData();

  /**
   * 3. запрос прогностических данных на 12 часов по неосвещенной территории ===========================
   */
  getGribDataFromSRV();

  /**
   * 4. заполнение вырезанного квадрата данными грибов =================================================
   */
  fillGribData();

  /**
   * 5. запрос из БД дельты (TField) ===================================================================
   */
  // запрашиваем данные анализа из базы
  getDeltaDataFromSRV();


  /**
   * 6. заполнение вырезанного квадрата данными дельт  =================================================
   */
  fillDeltaData();

  /**
   * 7. преобразование дельты в коэф. корреляции ===========================================================
   */
  // размер массивов
  int size;
  size = substracted_data_->size();

  // определяем массивы
  // для которых будем считать коэффициент корреляции
  double X[size], Y[size];
  //
  // заполняем массивы данными из разных полей
  // фактического и прогностического
  //
  for ( int i=0; i<size; i++ ){
    X[i] = substracted_data_->at(i).data;
    Y[i] = substracted_grib_data_->at(i).data;
    // error_log << X[i]  << Y[i];
  }

  double corr;
  // считаем коэффициент корреляции
  corr = MnMath::calculateKorrelation( X, Y, size );

  if ( isDebug_ ){
    // выводим его
    debug_log << corr;
  }

  /**
   *
   * 8. замешиваем грибы с дельтами ===============================================================================
   *
   */
  mergeGribWithW();

  /**
   * 9. смешивание в одной prepared_data данных наблюдений и скорректированных
   * прогностических данных с весовыми коэффициентами ============================================================
   */

  mergeFactWithGrib();

  /**
   * 10. Делаем объективный анализ на получившемся поле ============================================================
   */
  makeObanalAndSaveField();


  return;
}


/**
 *
 * Строим прогностическое поле на основе данных, которых не хватает
 * (не пришли данные со станций)
 */
void Analyser::startForecastLostDataCalculate(){

  /**
   *  1.запрос фактических данных ======================================================================
   */
  // запрашиваем фактические данные
  getFactDataFromSRV();

  /**
   *  2. забираем данные по наличию станций ============================================================
   *  и заполняем массив точками, в которых данных нет
   */

  fillLostDataFromStations();

  /**
   * 3. запрос прогностических данных на 12 часов по неосвещенной территории ===========================
   */
  getGribDataFromSRV();

  /**
   * 4. заполнение вырезанного квадрата данными грибов =================================================
   */
  fillGribData();

  /**
   * 5. запрос из БД дельты (TField) ===================================================================
   */
  // запрашиваем данные анализа из базы
  getDeltaDataFromSRV();


  /**
   * 6. заполнение вырезанного квадрата данными дельт  =================================================
   */
  fillDeltaData();

  /**
   *
   * 8. замешиваем грибы с дельтами ===============================================================================
   *
   */
  mergeGribWithW();

  /**
   * 9. смешивание в одной prepared_data данных наблюдений и скорректированных
   * прогностических данных с весовыми коэффициентами ============================================================
   */

  mergeFactWithGrib();

  /**
   * 10. Делаем объективный анализ на получившемся поле ============================================================
   */
  makeObanalAndSaveField();

  return;
}


/**
 *
 *  ===    HELP FUNCTIONS    ====
 *
 */


/**
 * Очищаем данные, чтобы работать дальше
 * параметры не трогаем
 * @return [description]
 */
Analyser& Analyser::clearData(){

  errormsg_               = new QString();

  result_field_           = new obanal::TField();
  all_field_              = new obanal::TField();
  alldelta_field_         = new obanal::TField();
  allgrib_field_          = new obanal::TField();

  prepared_data_          = new meteo::GeoData();
  substracted_data_       = new meteo::GeoData();
  all_data_               = new meteo::GeoData();
  substracted_delta_data_ = new meteo::GeoData();
  substracted_grib_data_  = new meteo::GeoData();
  return *this;
}

/**
 * Очищаем данные
 * @return [description]
 */
Analyser& Analyser::clearPreparedData(){
  prepared_data_          = new meteo::GeoData();
  return *this;
}


/**
 * Очищаем данные
 * @return [description]
 */
Analyser& Analyser::clearSubstractedData(){
  substracted_data_       = new meteo::GeoData();
  return *this;
}


/**
 * Проверяем есть ли ошибки
 * @return [description]
 */
bool Analyser::checkError(){
  if ( errormsg_->isEmpty() ){
    return false;
  }else{
    return true;
  }
}

/**
 * получаем текст ошибки
 * @return [description]
 */
QString Analyser::getErrorMsg(){
  return *errormsg_;
}

/**
 *  забираем данные по наличию станций
 *  и заполняем массив точками, в которых данных нет
 *
 * по итогу в prepared_data - данные по станциям
 * в substracted_data - те станции, где нет данных
 * @return [description]
 */
Analyser& Analyser::fillLostDataFromStations(){
  // проверяем есть ли ошибки
  if ( checkError() ) {
    return *this;
  }

  meteo::GeoData* stations_data = new meteo::GeoData(getStationsCoords());

  fillDataFromStations(stations_data);

  debugMSG(
      QString("Данных по станциям измерено: %1, Данных по станциям нет: %2")
            .arg(prepared_data_->size())
            .arg(substracted_data_->size())
  );

  delete stations_data;

  return *this;
}

/**
 * Заполняем массивы prepared_data и substracted_data
 * на вход получаем список станций, по которым нужно выбрать данные
 * предполагается, что данные уже лежат в all_data
 *
 * по итогу в prepared_data - данные по станциям
 * в substracted_data - те станции, где нет данных
 *
 * @param  stlist [description]
 * @return        [description]
 */
Analyser& Analyser::fillDataFromStationList( QList<QString> stlist ){
  // проверяем есть ли ошибки
  if ( checkError() ) {
    return *this;
  }

  meteo::GeoData* stations_data = new meteo::GeoData(getStationsCoords(stlist));

  fillDataFromStations(stations_data, true);

  delete stations_data;

  return *this;
}

/**
 * Заполняем массивы prepared_data и substracted_data
 * на вход получаем список станций, по которым нужно выбрать данные
 * предполагается, что данные уже лежат в all_data
 *
 * по итогу в prepared_data - данные по станциям
 * в substracted_data - те станции, где нет данных
 *
 * @param  stations_data [description]
 * @return               [description]
 */
Analyser& Analyser::fillDataFromStations(meteo::GeoData* stations_data, bool without_space){

  //
  // Заполняем массив точками, в которых нет данных
  //
  for ( int i=0; i<stations_data->size(); i++ ){
    // ищем точку  с данными в списке станций
    bool foundIt=false;

    // проходимся по списку всех станций
    // и пытаемся найти по координатам станцию которая пришла
    for ( int j=0; j<all_data_->size(); j++ ){
      // если находим станцию в списке станций - то
      // возвращаем ОК и прерываемся
      if ( MnMath::isEqual( all_data_->at(j).point.lat(), stations_data->at(i).point.lat() )
           &&
           MnMath::isEqual( all_data_->at(j).point.lon(), stations_data->at(i).point.lon() )    ){

        foundIt=true;
        // сохраняем данные вне области
        prepared_data_->append(all_data_->at(j));
        break;
      }
    }
    // смотрим все, что не попало
    // сохраняем
    if ( !foundIt ){
      // сохраняем данные внутри области
      substracted_data_->append(stations_data->at(i));
      //
      // если у нас надо заполнить пропуски - заполняем пустыми данными
      //
      if ( without_space ){
        prepared_data_->append( stations_data->at(i) );
      }
    } //endif

  }//endfor

  if ( isDebug_ ){
    error_log << "Нет данных по станциям: " << substracted_data_->size();
  }

  return *this;
}


/**
 * Получаем координаты
 * @param  stlist [description]
 * @return        [description]
 */
meteo::GeoData Analyser::getStationsCoords(QList<QString> stlist){
  //
  // получаем список станций
  //
  meteo::sprinf::MultiStatementRequest request;
  request.add_type(this->req_station_type_);
  // заполняем реквест
  if ( !stlist.empty() ){
    for (int i = 0; i < stlist.count(); ++i) {
      request.add_index( stlist.at(i).toInt() );
    }
  }

  return getStationsCoords( request );
}

/**
 * Получаем координаты станций по реквесту
 * @param  request [description]
 * @return         [description]
 */
meteo::GeoData Analyser::getStationsCoords(meteo::sprinf::MultiStatementRequest &request){

  connectToSprinfSRV();
  meteo::sprinf::Stations* reply = nullptr;
  // выполняем запрос meteo::surf::SurfaceService::GetMeteoData
  if ( ctrl_ != nullptr ){
    reply = ctrl_->remoteCall( &meteo::sprinf::SprinfService::GetStations, request, 120000, true );
    disconnect();
  }

  // данные по станциям
  meteo::GeoData stations_data;

  for ( auto item : reply->station() ) {
    // error_log << item.position().lat_radian() << item.position().lon_radian();
    stations_data.append(
            meteo::MData(
                item.position().lat_radian(),
                item.position().lon_radian(),
                0
            )
    );
  }

  //
  // Error MSG
  //
  if ( stations_data.size()==0 ){
    errormsg_->append("\nНет списка станций");
  }

  if ( isDebug_ ){
    debug_log << "Получен список всех станций: " << stations_data.size();
  }

  return stations_data;
}

/**
 * получаем координаты всех станций (аэрологических пока)
 * @return [description]
 */
meteo::GeoData Analyser::getStationsCoords(){

  //
  // получаем список станций
  //
  meteo::sprinf::MultiStatementRequest request;
  request.add_type(this->req_station_type_);

  return getStationsCoords( request );
}



/**
 * Делаем объективный анализ на получившемся поле
 * и сохраняем поле в базе
 * @return [description]
 */
Analyser& Analyser::makeObanalAndSaveField(){
  // проверяем есть ли ошибки
  if ( checkError() ) {
    debugMSG( *errormsg_ );
    return *this;
  }

  // результирующее поле
  double predel_grad  = 2.0;

  meteo::field::DataDesc fdescr;
  //
  // вызываем функцию объективного анализа
  //
  makeDataObanal( fdescr, predel_grad  );

  //
  // сохраняем поле в базе
  //
  saveFieldToDb( fdescr );

  return *this;
}

/**
 * заполнение вырезанного квадрата данными грибов
 * @return [description]
 */
Analyser& Analyser::fillGribData(){
  // проверяем есть ли ошибки
  if ( checkError() ) {
    debugMSG( *errormsg_ );
    return *this;
  }

  for ( auto item : *substracted_data_ ) {
    // error_log << item.data - allgrib_field.pointValue( item.point );
    substracted_grib_data_->append(
        meteo::MData(
            item.point.lat(),
            item.point.lon(),
            allgrib_field_->pointValue( item.point ),
            TRUE_DATA
        )
    );
  }

  if ( isDebug_ ){
    debug_log<< "Грибы: " << substracted_grib_data_->size();
  }

  return *this;
}

/**
 * заполнение вырезанного квадрата данными дельт
 * @return [description]
 */
Analyser& Analyser::fillDeltaData(){
  // проверяем есть ли ошибки
  if ( checkError() ) {
    debugMSG( *errormsg_ );
    return *this;
  }

  for ( auto item : *substracted_data_ ) {
    // error_log << item.data - allgrib_field.pointValue( item.point );
    substracted_delta_data_->append(
        meteo::MData(
            item.point.lat(),
            item.point.lon(),
            alldelta_field_->pointValue( item.point ),
            TRUE_DATA
        )
    );
  }

  if ( isDebug_ ){
    // error_log<< substracted_delta_data;
    debug_log<< "Дельты: " << substracted_delta_data_->size();
  }

  return *this;
}

/**
 * замешиваем грибы с дельтами
 * @return [description]
 */
Analyser& Analyser::mergeGribWithW(){
  // проверяем есть ли ошибки
  if ( checkError() ) {
    debugMSG( *errormsg_ );
    return *this;
  }

  // размер массива
  int size;
  size = substracted_grib_data_->size();
  for ( int i=0; i<size; i++ ){
    // ставим веса на 1
    (*substracted_grib_data_)[i].w = 1;
    // заполняем веса
    // substracted_grib_data_[i].w = substracted_delta_data->at(i).data;
  }

  if ( isDebug_ ){
    debug_log<< "Замешали грибы с дельтами: " << substracted_grib_data_->size();
  }

  return *this;
}

/**
 * смешивание в одной prepared_data данных наблюдений и скорректированных прогностических данных с весовыми коэффициентами
 * @return [description]
 */
Analyser& Analyser::mergeFactWithGrib(){
  // проверяем есть ли ошибки
  if ( checkError() ) {
    debugMSG( *errormsg_ );
    return *this;
  }

  int size;
  size = substracted_grib_data_->size();
  for ( int i=0; i<size; i++ ){
    prepared_data_->append( substracted_grib_data_->at(i) );
    // error_log<< substracted_grib_data.at(i).data ;
  }

  if ( isDebug_ ){
    debug_log<< "Подготовили итоговый массив с данными: " << prepared_data_->size();
  }

  return *this;
}

/**
 * устанавливаем шаг часа для nextTime()
 * @param  req_hour_step [description]
 * @return               [description]
 */
Analyser&        Analyser::setHourStep( int req_hour_step ){
  hour_step_ = req_hour_step;
  return *this;
}


/**
 * Меняем дату на слуедующую с использованием hour_step
 * прибавляем к часу hour_step и если он превышает 24 часа, то меняем следующий день
 * @return [description]
 */
Analyser&        Analyser::nextTime(){
  // прибавляем час
  req_hour_+=hour_step_;
  // если сутки закончились
  // то вычитаем 2 часа и меняем счетчик дней
  if ( req_hour_>=24 ){
    req_hour_-=24;
    nextDay();
  }
  setHour(req_hour_);
  debugMSG( QString("Установлена дата %1 (шаг %2) ").arg(req_date_->toString(Qt::ISODate)).arg(hour_step_) );
  return *this;
}

/**
 * Меняем дату на предыдущую с использованием hour_step
 * прибавляем к часу hour_step и если он превышает 24 часа, то меняем следующий день
 * @return [description]
 */
Analyser&        Analyser::prevTime(){
  // прибавляем час
  req_hour_-=hour_step_;
  // если сутки закончились
  // то вычитаем 2 часа и меняем счетчик дней
  if ( req_hour_<0 ){
    req_hour_+=24;
    prevDay();
  }
  setHour(req_hour_);
  debugMSG( QString("Установлена дата %1 (шаг %2) ").arg(req_date_->toString(Qt::ISODate)).arg(hour_step_) );
  return *this;
}

/**
 * прибавляем один день
 * @return [description]
 */
Analyser&        Analyser::nextDay(){
  // добавляем секунд в сутках
  // 24*3600
  req_date_->setDate( req_date_->addDays( 1 ).date() );
  return *this;
}

/**
 * убавляем один день
 * @return [description]
 */
Analyser&        Analyser::prevDay(){
  // добавляем секунд в сутках
  // 24*3600
  req_date_->setDate( req_date_->addDays( -1 ).date() );
  return *this;
}

/**
 * подключаемся к сервису
 */
Analyser& Analyser::connect( meteo::settings::proto::ServiceCode srv ){
  // создаем коннектор
  ctrl_ = meteo::global::serviceChannel( srv );
  if(0 == ctrl_) {
    error_log << "Не удалось подключиться к сервису";
  }
  return *this;
}


/**
 * подключаемся к сервису сухопутному
 * @return [description]
 */
Analyser& Analyser::connectToSrcSRV() {
  connect(meteo::settings::proto::kSrcData);
  return *this;
}

/**
 * подключаемся к сервису полей
 * @return [description]
 */
Analyser& Analyser::connectToFieldSRV() {
  connect(meteo::settings::proto::kField);
  return *this;
}

/**
 * подключаемся к сервису полей
 * @return [description]
 */
Analyser& Analyser::connectToSprinfSRV() {
  connect(meteo::settings::proto::kSprinf);
  return *this;
}


/**
 * подключаемся к сервису полей
 * @return [description]
 */
Analyser& Analyser::connectToForecastSRV() {
  connect(meteo::settings::proto::kForecastData);
  return *this;
}


/**
 * отключаемся от сервисов
 * @return [description]
 */
Analyser& Analyser::disconnect() {
  delete ctrl_;
  return *this;
}

/**
 * Получаем дату
 * @return [description]
 */
QDateTime Analyser::getDateTime() {
  return *req_date_;
}

/**
 * получаем фактические данные от сервиса
 */
Analyser& Analyser::getFactDataFromSRV( ){

  if ( isDebug_ ){
    debug_log << QObject::tr("Запрос данных фактической погоды");
  }

  // проверяем есть ли ошибки
  if ( checkError() ) {
    debugMSG( *errormsg_ );
    return *this;
  }

  // создаем запрос
  meteo::surf::DataRequest request;

  // заполняем запрос
  // request.add_type(req_type_);
  request.set_as_proto(true);
  request.set_date_start(req_date_->toString(Qt::ISODate).toStdString());
  //
  // что запрашиваем
  //

  // создаем синглтон для работы с дескрипторами
  // надо для того, чтобы из числа получить букву
  meteodescr::TMeteoDescriptor* md = TMeteoDescriptor::instance();
  // запрагиваем дескриптор по имени
  request.add_meteo_descrname( md->name(req_param_).toStdString() );

  // обязательные параметры
  request.set_level_p(req_level_);
  request.set_type_level(req_level_type_);

  // устанавливаем тип запроса
  // request.set_query_type(meteo::surf::kTermValue);

  if ( isDebug_ ){
    debug_log<< request.DebugString();
  }

  meteo::surf::DataReply* reply = nullptr;
  connectToSrcSRV();
  // выполняем запрос meteo::surf::SurfaceService::GetMeteoData
  if ( ctrl_ != nullptr ){
    reply = ctrl_->remoteCall( &meteo::surf::SurfaceService::GetMeteoData, request, 1200000, true );
    disconnect();
  }

  // если ответ не пришел
  if ( 0 == reply ){
    debugMSG( "Не пришел ответ от сервиса" );
    delete reply;
    return *this;
  }

  //
  // заполняем структуру данными
  //
  for ( auto item : reply->meteodata_proto() ) {
    // error_log << item.gp().la() << item.param().size() ;
    if ( item.param().size() >0 ){
        // error_log<< "quality: "<< item.param(0).quality() << "value:" << item.param(0).value() <<  "size:" <<item.param().size();
        all_data_->append(
                meteo::MData(
                    item.gp().la(),
                    item.gp().lo(),
                    item.param(0).value(),
                    TRUE_DATA
                )
        );
    }//endif
  }

  delete reply;

  //
  // Error MSG
  //
  if ( all_data_->count()==0 ){
    errormsg_->append("\nНет данных фактических наблюдений");
    if ( isDebug_ ){
      error_log<< "Нет данных фактических наблюдений";
    }
    return *this;
  }

  if ( isDebug_ ){
    debug_log<< QObject::tr("Данные фактической погоды получены, усвоены. Всего: ") << all_data_->count();
  }


  return *this;
}


/**
 * Функция выкалывания обозначенного региона из
 * массива данных
 */
Analyser& Analyser::substractRegionFromData(){

  // проверяем есть ли ошибки
  if ( checkError() ) {
    debugMSG( *errormsg_ );
    return *this;
  }

  // координаты региона
  // верхнего левого угла и нижнего правого угла
  // преобразуем в радианы
  // проходимся в цикле и выкалываем точки
  for ( int i=0; i<all_data_->size();i++ ){
    // смотрим все, что не попало в квадрат
    // сохраняем
    if (
        ( lefttop_lon_     *meteo::DEG2RAD  >= all_data_->at(i).point.lon()
      ||  rightbottom_lon_ *meteo::DEG2RAD  <= all_data_->at(i).point.lon() )
      &&( lefttop_lat_     *meteo::DEG2RAD  >= all_data_->at(i).point.lat()
      ||  rightbottom_lat_ *meteo::DEG2RAD  <= all_data_->at(i).point.lat() )
       ){
      // сохраняем данные вне области
      prepared_data_->append(all_data_->at(i));
    }else{
      // сохраняем данные внутри области
      substracted_data_->append(all_data_->at(i));
    } //endif
  }//endfor



  if ( isDebug_ ){
    debug_log<< "Подготовлено: " << prepared_data_->size();
    debug_log<< "Вырезано:     " << substracted_data_->size();
  }

  return *this;
}


/**
 * Делаем запрос к сервису полей и получаем в ответ
 * поле с данными
 * @param  request [description]
 * @return         [description]
 */
Analyser& Analyser::getFieldFromSRV( meteo::field::DataRequest request ){

  // проверяем есть ли ошибки
  if ( checkError() ) {
    debugMSG( *errormsg_ );
    return *this;
  }

  meteo::field::DataReply* reply = nullptr;
  connectToFieldSRV();

  if ( isDebug_ ){
    debug_log<< request.DebugString();
  }

  if ( ctrl_ != nullptr ){
    reply = ctrl_->remoteCall( &meteo::field::FieldService::GetFieldData, request, 50000 );
    disconnect();
  }

  // если ответ не пришел
  if ( 0 == reply ){
    debugMSG( "Не пришел ответ от сервиса" );
    delete reply;
    return *this;
  }

  QByteArray arr( reply->fielddata().data(), reply->fielddata().size() );
  delete reply;

  // загружаем данные
  allgrib_field_->fromBuffer(&arr);

  if (allgrib_field_->kolData()==0){
    error_log<<"Данные не получены. Расчет невозможен";
    error_log<< request.DebugString();
    errormsg_->append("\nНулевое поле. Данных нет.");
  }

  debugMSG( QString("Данные приняты, всего %1").arg(allgrib_field_->kolData()) );

  return *this;
}



meteo::field::DataRequest Analyser::prepareRequestForField( ){

  // создаем запрос
  meteo::field::DataRequest request;
  // и отматываем назад на время заблагговременности
  // т.к. нам нужны свежие прогнозы именно с этой заблаговременностью

  // если включен режим прогнозирования, то берем свежие грибы
  if ( predictMode_==true ){
    request.set_date_start( req_date_->addSecs( req_forecasthour_ * 3600 ).toString(Qt::ISODate).toStdString() );
  }else{
    request.set_date_start( req_date_->addSecs( req_hour_ -  req_forecasthour_ * 3600 ).toString(Qt::ISODate).toStdString() );
  }

  request.add_hour(       req_forecasthour_*3600 );
  request.add_level(      req_level_     );
  request.add_type_level( req_level_type_);
  request.add_meteo_descr(req_param_     );
  request.add_center(     req_center_    );
  request.set_model(      req_model_     );
  request.set_need_field_descr( true );

  // STEP_500x500 = 1,  //!< через 5 градусов по широте и долготе
  // STEP_250x250 = 2,  //!< через 2.5 градуса по широте и долготе
  // STEP_125x125 = 3,  //!< через 1.25 градуса по широте и долготе
  // STEP_100x100 = 4,  //!< через 1 градус по широте и долготе
  // STEP_0625x0625 = 5,//!< через 0.625 градуса по широте и долготе
  // STEP_2983x100 = 6, //!< через 2.983 градуса по широте и долготе
  // STEP_0500x0500 = 7, //!< через .5 градуса по широте и долготе
  // STEP_0100x0100 = 8, //!< через .1 градуса по широте и долготе
  request.set_net_type(   req_net_type_  );
  // request.set_is_df(      false     );

  if ( isDebug_ ){
    debug_log<< request.DebugString();
  }
  return request;
}




meteo::field::DataRequest Analyser::prepareRequestForFactField( ){

  // создаем запрос
  meteo::field::DataRequest request;
  
  request.set_date_start( req_date_->toString(Qt::ISODate).toStdString() );
  request.add_hour(       0              );
  request.add_level(      req_level_     );
  request.add_type_level( req_level_type_);
  request.add_meteo_descr(req_param_     );
  request.add_center(     req_center_    ); // 250
  request.set_model(      req_model_     ); // 250
  request.set_need_field_descr( true );

  // STEP_500x500 = 1,  //!< через 5 градусов по широте и долготе
  // STEP_250x250 = 2,  //!< через 2.5 градуса по широте и долготе
  // STEP_125x125 = 3,  //!< через 1.25 градуса по широте и долготе
  // STEP_100x100 = 4,  //!< через 1 градус по широте и долготе
  // STEP_0625x0625 = 5,//!< через 0.625 градуса по широте и долготе
  // STEP_2983x100 = 6, //!< через 2.983 градуса по широте и долготе
  // STEP_0500x0500 = 7, //!< через .5 градуса по широте и долготе
  // STEP_0100x0100 = 8, //!< через .1 градуса по широте и долготе
  request.set_net_type(   req_net_type_  );
  // request.set_is_df(      false     );
  return request;
}

/**
 * Получаем поля объективного анализа фактической информации
 * @return                        [description]
 */
Analyser& Analyser::getFieldDataFromSRV( ){

  debugMSG("Запрос данных фактических наблюдений");

  getFieldFromSRV( prepareRequestForFactField() );

  return *this;
}

/**
 * Получаем 12часовые прогнозы на текущий срок
 * @return                        [description]
 */
Analyser& Analyser::getGribDataFromSRV( ){

  debugMSG("Запрос данных грибов");

  getFieldFromSRV( prepareRequestForField() );

  return *this;
}




/**
 * Забираем из базы
 * дельту
 * @return [description]
 */
Analyser& Analyser::getDeltaDataFromSRV(){
  // проверяем есть ли ошибки
  if ( checkError() ) {
    debugMSG( *errormsg_ );
    return *this;
  }

  //                        создаем запрос
  meteo::field::DataRequest request;
  request.add_center(       req_center_        );
  request.add_hour(         req_forecasthour_ *3600);
  request.add_level(        req_level_         );
  request.set_model(        req_model_         );
  request.add_type_level(   req_level_type_    );
  //                        тип данных содержит в начале 90
  request.add_meteo_descr(9000000 + req_param_);
  // дату не ставим пока
  // request.set_date_start(req_date_->toString(Qt::ISODate).toStdString());
  // request.set_date_end(analyse_dt_s);
  //
  //куда будем получать поля
  meteo::field::DataReply reply;

  if ( isDebug_ ){
    debug_log<< request.DebugString();
  }

  // загружаем поля
  //FIXME! db_->loadField(&request, &reply);//Сервис использовать нужно

  // если ответ не пришел
  // if ( 0 == reply ){
  //   debugMSG( "Не пришел ответ от сервиса" );
  //   return *this;
  // }

  // вытаскиваем данные из TFIELD
  QByteArray arr( reply.fielddata().data(), reply.fielddata().size() );

  // загружаем данные
  alldelta_field_->fromBuffer(&arr);



  return *this;
}



/**
 * Сохраняем рассчитанное поле в базе данных
 * с новым дескриптором
 */
Analyser& Analyser::saveFieldToDb( meteo::field::DataDesc fdescr ){
  // проверяем есть ли ошибки
  if ( checkError() ) {
    debugMSG( *errormsg_ );
    return *this;
  }

  if ( result_field_->kolData() == 0 ){
    errormsg_->append("\n Данные не записаны, т.к. их ноль!");
    if ( isDebug_ ){
      debug_log << "Ничего не сохранили в базу, т.к. грибов нет!";
    }
    return *this;
  }

  TObanalDb* obanal_db;
  obanal_db = new TObanalDb();

  // сохраняем поле
  obanal_db->saveField(*result_field_, fdescr );

  if ( isDebug_ ){
    debug_log << fdescr.DebugString();
    debug_log << "Информация Сохранена в базу";
  }

  delete obanal_db;

  return *this;
}



/**
 * Сохраняем в базу вырезанное поле
 */
Analyser& Analyser::saveFactDataToDb( meteo::field::DataDesc fdescr ){
  // проверяем есть ли ошибки
  if ( checkError() ) {
    debugMSG( *errormsg_ );
    return *this;
  }

  if ( prepared_data_!=nullptr && prepared_data_->size()==0 ){
    errormsg_->append("\n Данные не записаны, т.к. их ноль!");
    if ( isDebug_ ){
      debug_log << "Ничего не сохранили в базу, т.к. данных нет!";
    }
    return *this;
  }

  TObanalDb* obanal_db;
  obanal_db = new TObanalDb();

  // сохраняем поле
  // obanal_db->saveField(*result_field_, fdescr );

  if ( isDebug_ ){
    debug_log << fdescr.DebugString();
    debug_log << "Информация (фактические данные) Сохранена в базу";
  }

  delete obanal_db;

  return *this;
}


/**
 * Делаем объективный анализ по получившемуся полю
 * @param  fdescr        [description]
 * @param  prepared_data [description]
 * @param  predel_grad   [description]
 * @return               [description]
 */
Analyser& Analyser::makeDataObanal( meteo::field::DataDesc &fdescr,  double predel_grad ){

  // проверяем есть ли ошибки
  if ( checkError() ) {
    debugMSG( *errormsg_ );
    return *this;
  }

  // если поле с данными пустое - ничего не делаем
  if ( prepared_data_!=nullptr && prepared_data_->size()==0 ){
    errormsg_->append("\nНет подготовленных для интерполяции данных");
    return *this;
  }

  QStringList         badData;

  // если включен режим прогнозирования, то
  // сохраняем в будущее
  if ( predictMode_==true ){
    fdescr.set_date(     req_date_->addSecs( req_forecasthour_ * 2 * 3600 ).toString(Qt::ISODate).toStdString()   );
  }else{
    // или ставим сегодняшний день
    fdescr.set_date(     req_date_->toString(Qt::ISODate).toStdString()   );
  }
  fdescr.set_hour(       req_forecasthour_*3600  ); //т.к. это не прогноз, то ставим нули (текущий срок, на который прогнозируем)
  fdescr.set_meteodescr( req_field_description_ + req_param_  );
  fdescr.set_level(      req_level_              );
  fdescr.set_net_type(   req_net_type_           );
  fdescr.set_center(     req_save_center_        ); //центр, в котором будем сохранять
  fdescr.set_model(      req_model_              );
  fdescr.set_level_type( req_level_type_         );

  result_field_->setNet      (result_field_->getRegPar(), static_cast<NetType>(fdescr.net_type()));
  result_field_->setNetMask  (result_field_->getRegPar());
  result_field_->setValueType(fdescr.meteodescr(),  fdescr.level(), fdescr.level_type());
  result_field_->setHour     (fdescr.hour());
  result_field_->setModel    (fdescr.model());
  result_field_->setCenter   (fdescr.center());
  result_field_->setDate     (QDateTime::fromString(QString::fromStdString(fdescr.date()), Qt::ISODate));
  if(fdescr.has_center_name())     { result_field_->setCenterName(QString::fromStdString(fdescr.center_name())); }
  if(fdescr.has_level_type_name()) { result_field_->setLevelTypeName(QString::fromStdString(fdescr.level_type_name())); }

  // интерполируем поле
  MnObanal::prepInterpolHaos( prepared_data_, result_field_, predel_grad, &badData);

  if ( isDebug_ ){
    debug_log<< "***********************************************************";
    debug_log << "badsize: " << badData.size();
    debug_log << "koldata: "<< result_field_->kolData();
  }

  int count=0;
  const bool* msk=nullptr;
  msk = result_field_->mask();
  for( unsigned int a = 0; a < sizeof(msk); a = a + 1 ){
    if (msk[a] == true){
      count++;
    }
  }

  if ( isDebug_ ){
    debug_log << "count mask: "<< count;
  }

  count=0;
  msk = result_field_->netMask();
  for( unsigned int a = 0; a < sizeof(msk); a = a + 1 ){
    if (msk[a] == true){
      count++;
    }
  }

  if ( isDebug_ ){
    debug_log << "count netMask: "<< count;
    debug_log<< "********************   END   ***********************";
  }

  fdescr.set_count_point(  result_field_->kolData()   );

  if ( isDebug_ ){
    debug_log<< "Проинтерполировали поле: " << result_field_->kolData();
  }

  return *this;
}




/**
 * получаем заполненный описательный формат
 * из переменных, которые заданы внутри класса
 * @return [description]
 */
meteo::field::DataDesc Analyser::getFilledDescr(){

  meteo::field::DataDesc fdescr;

  fdescr.set_date(       req_date_->toString(Qt::ISODate).toStdString()   );
  fdescr.set_hour(       req_forecasthour_*3600  );
  fdescr.set_meteodescr( req_field_description_ + req_param_  );
  fdescr.set_level(      req_level_              );
  fdescr.set_net_type(   req_net_type_           );
  // fdescr.set_center(     req_save_center        ); //центр, в котором будем сохранять
  fdescr.set_model(      req_model_              );
  fdescr.set_center(     req_center_             );
  fdescr.set_level_type( req_level_type_         );

  return fdescr;
}



/**
 * Получаем все станции, по которым будем считать оправдываемость прогнозов
 * и потом ранжировать прогностические модели
 * @return [description]
 */
meteo::GeoData Analyser::getAllPunktsFromSRV(){

  // подключаемся
  connectToForecastSRV();
  meteo::forecast::PunktRequest req;
  req.set_requestcode(meteo::forecast::kPunktGetRequest);

  meteo::forecast::PunktResponce* resp = nullptr;
  // запрашиваем пункты
  if ( ctrl_ != nullptr ){
    resp = ctrl_->remoteCall(&meteo::forecast::ForecastData::GetForecastPunkts, req, 10000);
    // отключаемся
    disconnect();
  }

  // данные по станциям
  meteo::GeoData stations_data;

  if (nullptr == resp){
    errormsg_->append("\nНет ответа от сервиса");
    debugMSG( "Нет ответа от сервиса" );
    return stations_data;
  }

  if ( resp->punkts_size()==0 ){
    errormsg_->append("\nНет пунктов, по которым считать. Задайте пункт дислокации");
    debugMSG( "Нет пунктов, по которым считать. Задайте пункт дислокации" );
    return stations_data;
  }

  for (int i = 0; i < resp->punkts_size(); ++i) {
    const meteo::forecast::PunktValue &value = resp->punkts(i);

    stations_data.append(
            meteo::MData(
                MnMath::deg2rad( value.fi() ),
                MnMath::deg2rad( value.la() ),
                0
            )
    );
  }

  return stations_data;
}

/**
 * Делаем поле из полученных фатических данных
 *
 * @return [description]
 */
Analyser& Analyser::makeFactField(){
  // проверяем есть ли ошибки
  if ( checkError() ) {
    debugMSG( *errormsg_ );
    return *this;
  }
  delete prepared_data_;
  // говорим, что все данные, которые мы получили - являются итоговыми для объективного анализа
  prepared_data_ = all_data_;

  // результирующее поле
  double predel_grad  = 2.0;

  meteo::field::DataDesc fdescr;
  makeDataObanal(fdescr,predel_grad);
  return *this;
}


/**
 * Рассчитываем разницу между полем грибов и полем ФП
 * в заданной точке
 * @param  point [description]
 * @return       [description]
 */
double Analyser::getDeltaOnPoint( meteo::GeoPoint point ){
  // проверяем есть ли ошибки
  if ( checkError() ) {
    return 0;
  }

  double delta=0;

  if ( result_field_->kolData() >0
   &&  allgrib_field_->kolData() >0 ){

    debugMSG( QString("Считаем дельту по точке %1 %2: %3 - %4")
                    .arg(point.lat())
                    .arg(point.lon())
                    .arg(result_field_->pointValue( point ))
                    .arg(allgrib_field_->pointValue( point )) );

    delta = result_field_->pointValue( point ) - allgrib_field_->pointValue( point );
  }else{
    errormsg_->append("\nНет данных в полях для расчета");
    debugMSG("Нет данных в полях для расчета");
  }

  return delta;
}

/**
 * Вычитаем регион из данных наблюдений
 * удаляем из базы данные
 * @return [description]
 */
Analyser&         Analyser::substractSquareFromSrcData(){
  meteo::GeoVector gv;
  meteo::GeoPoint p;

  p.setLatDeg( lefttop_lat_ );
  p.setLonDeg( lefttop_lon_ );
  gv.append(   p ); // добавляем верхний левый угол

  p.setLatDeg( lefttop_lat_ );
  p.setLonDeg( rightbottom_lon_ );
  gv.append(   p ); // добавляем верхний правый угол

  p.setLatDeg( rightbottom_lat_ );
  p.setLonDeg( rightbottom_lon_ );
  gv.append(   p ); // добавляем нижний правый угол

  p.setLatDeg( rightbottom_lat_ );
  p.setLonDeg( lefttop_lon_ );
  gv.append(   p ); // добавляем нижний левый угол

  p.setLatDeg( lefttop_lat_ );
  p.setLonDeg( lefttop_lon_ );
  gv.append(   p ); // добавляем верхний левый угол

  // заполняем параметры запроса
  meteo::field::DataDesc fdescr;

  fdescr.set_date(       req_date_->toString(Qt::ISODate).toStdString()   );
  fdescr.set_hour(       req_hour_  );

  // fdescr.set_level(      req_level_              );
  // fdescr.set_center(     req_save_center        );
  // fdescr.set_level_type( req_level_type_         );

  TObanalDb* obanal_db;
  obanal_db = new TObanalDb();

  // сохраняем поле
  obanal_db->substractRegionFromSrc( gv, fdescr );

  delete obanal_db;

  return *this;
}


/**
 * смотрим попадает ли разница в градацию
 *
 * @param  param [description]
 * @param  delta - разница между прогноом и фактикой
 * @return       [description]
 */
bool Analyser::isParamInGradation( QString param, double delta ){

  QMap<QString, double> gradation;
  gradation.insert( QString("T"), 2.0 );
  gradation.insert( QString("V"), 2000.0 );
  gradation.insert( QString("Td"), 2.0 );

  if ( gradation.value( param, 0 ) > fabs(delta) ){
    return true;
  }

  return false;
}


/**
 * Отправляем сообщение в дебаг, если надо
 * @param  txt [description]
 * @return     [description]
 */
Analyser&  Analyser::debugMSG( QString txt ){
  if ( isDebug_ ){
    debug_log << txt;
  }
  return *this;
}

/**
 * Устанавливаем переменную подготовленных данных
 * для дальнейшего использования в объективном анализе
 * @param  prepared_data_new [description]
 * @return                   [description]
 */
Analyser& Analyser::setPreparedData( meteo::GeoData prepared_data_new ){
  prepared_data_ = new meteo::GeoData(prepared_data_new);
  return *this;
}


/**
 * Установка значения переменной
 * @param  date [description]
 * @return          [description]
 */
Analyser& Analyser::setDateTime(  QDateTime dt ){
  req_date_ = new QDateTime( dt );
  return *this;
}



/**
 * Установка значения переменной
 * @param  req_hour [description]
 * @return          [description]
 */
Analyser& Analyser::setHour(  int req_hour_new ){
  req_hour_ = req_hour_new;
  // устанавливаем время на нужный срок
  req_date_->setTime( QTime(req_hour_,0,0) );
  return *this;
}

/**
 * Установка значения переменной
 * @param  req_hour [description]
 * @return          [description]
 */
Analyser& Analyser::setForecasthour(  int req_forecasthour_new ){
  req_forecasthour_ = req_forecasthour_new;
  return *this;
}

/**
 * Установка значения переменной
 * @param  req_hour [description]
 * @return          [description]
 */
Analyser& Analyser::setParam( int req_param_new ){
  req_param_ = req_param_new;
  return *this;
}

/**
 * Установка значения переменной
 * @param  req_hour [description]
 * @return          [description]
 */
Analyser& Analyser::setLevelType(  int req_level_type_new ){
  req_level_type_ = req_level_type_new;
  return *this;
}

/**
 * Установка значения переменной
 * @param  req_hour [description]
 * @return          [description]
 */
Analyser& Analyser::setLevel( int req_level_new ){
  req_level_ = req_level_new;
  return *this;
}

/**
 * Установка значения переменной
 * @param  req_hour [description]
 * @return          [description]
 */
Analyser& Analyser::setType(  int req_type_new ){
  req_type_ = req_type_new;
  return *this;
}

/**
 * Установка значения переменной
 * @param  req_hour [description]
 * @return          [description]
 */
Analyser& Analyser::setCenter(  int req_center_new ){
  req_center_ = req_center_new;
  return *this;
}

/**
 * Установка значения переменной
 * @param  req_hour [description]
 * @return          [description]
 */
Analyser& Analyser::setModel( int req_model_new ){
  req_model_ = req_model_new;
  return *this;
}

/**
 * Задаем тип сетки
 * @param  req_net_type [description]
 * @return          [description]
 */
Analyser& Analyser::setNetType( int req_net_type ){
  req_net_type_ = req_net_type;
  return *this;
}

/**
 * Установка значения переменной
 * @param  req_hour [description]
 * @return          [description]
 */
Analyser& Analyser::setFieldDescription( int req_field_description_new ){
  req_field_description_ = req_field_description_new;
  return *this;
}



/**
 * Включение/выключение режима отладки
 * @return [description]
 */
Analyser&  Analyser::setDebugOn(){
  isDebug_ = true;
  return *this;
}

/**
 * Включение/выключение режима отладки
 * @return [description]
 */
Analyser&  Analyser::setDebugOff(){
  isDebug_ = false;
  return *this;
}


/**
 * Задаем регион для исключения
 * @param  lefttop     [description]
 * @param  rightbottom [description]
 * @return             [description]
 */
Analyser&  Analyser::setRegion( meteo::GeoPoint lefttop, meteo::GeoPoint rightbottom ){
  lefttop_lat_     = lefttop.lat();
  lefttop_lon_     = lefttop.lon();
  rightbottom_lat_ = rightbottom.lat();
  rightbottom_lon_ = rightbottom.lon();
  return *this;
}


/**
 * Задаем регион для исключения
 * @param  lefttop     [description]
 * @param  rightbottom [description]
 * @return             [description]
 */
Analyser&  Analyser::setRegion( double lefttop_lat, double lefttop_lon, double rightbottom_lat, double rightbottom_lon ){
  lefttop_lat_     = lefttop_lat;
  lefttop_lon_     = lefttop_lon;
  rightbottom_lat_ = rightbottom_lat;
  rightbottom_lon_ = rightbottom_lon;
  return *this;
}



/**
 * Получаем данные, записанные в переменную
 * @return [description]
 */
obanal::TField&   Analyser::getAllgribField(){
  return *allgrib_field_;
}


/**
 * Получаем данные, записанные в переменную
 * @return [description]
 */
meteo::GeoData&        Analyser::getAllData(){
  return *all_data_;
}

/**
 * Получаем данные, записанные в переменную
 * @return [description]
 */
meteo::GeoData&        Analyser::getPreparedData(){
  return *prepared_data_;
}


/**
 * Включаем/выключаем режим прогнозирования или обучения
 * @return [description]
 */
Analyser&         Analyser::setPredictModeOn(){

  predictMode_=true;

  return *this;
}

/**
 * Включаем/выключаем режим прогнозирования или обучения
 * @return [description]
 */
Analyser&         Analyser::setPredictModeOff(){
  predictMode_=false;
  return *this;
}

} // namespace ml
} //namespace meteo
