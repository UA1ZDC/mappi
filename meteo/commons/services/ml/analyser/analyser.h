#ifndef METEO_ML_ANALYSER_ANALYZE_H
#define METEO_ML_ANALYSER_ANALYZE_H

#include <stdlib.h>
#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>
#include <cross-commons/app/options.h>
#include <meteo/commons/settings/settings.h>
#include <meteo/commons/proto/field.pb.h>
#include <meteo/commons/proto/forecast.pb.h>

#include <meteo/commons/rpc/rpc.h>
#include <meteo/commons/services/obanal/tobanalservice.h>
#include <meteo/commons/proto/sprinf.pb.h>
#include <meteo/commons/proto/field.pb.h>

#include <meteo/commons/proto/surface.pb.h>
#include <commons/textproto/pbtools.h>
#include <commons/meteo_data/meteo_data.h>
#include <commons/meteo_data/complexmeteo.h>
#include <commons/meteo_data/tmeteodescr.h>
#include <commons/mathtools/mnmath.h>
#include <meteo/commons/services/obanal/tobanalservice.h>
#include <meteo/commons/services/obanal/tobanaldb.h>
#include <commons/obanal/func_obanal.h>

#include <commons/geobasis/geodata.h>
#include <commons/geobasis/geopoint.h>
#include <commons/geobasis/geovector.h>
//#include <meteo/commons/services/fieldata/fieldservicedb.h>

namespace meteo {
namespace ml {

/**
 * Класс восстановления полей по прогностическим данным
 */
class Analyser
{

public:
  Analyser();
  ~Analyser();

  /**
   * Функция расчета коэффициентов корелляции прогностических и фактических полей
   */

// Анализируем поле по конкретному вырезанному региону
void                      startForecastCustomRegionCalculate();

// Строим прогностическое поле на основе данных, которых не хватает
// (не пришли данные со станций)
void                      startForecastLostDataCalculate();

Analyser&         getFactDataFromSRV();
Analyser&         substractRegionFromData();
Analyser&         getFieldFromSRV( meteo::field::DataRequest request );
/**
 * Подготавливаем поле запроса
 */
meteo::field::DataRequest prepareRequestForField();
// для данных фактических наблюдений
meteo::field::DataRequest prepareRequestForFactField();
/**
 * Получаем 12часовые прогнозы на текущий срок
 */
Analyser&         getGribDataFromSRV( );
// поля по фактической погоде за срок
Analyser&         getFieldDataFromSRV( );
/**
 * Забираем из базы дельту
 */
Analyser&         getDeltaDataFromSRV();
/**
 * Сохраняем рассчитанное поле в базе данных с новым дескриптором
 */
Analyser&         saveFieldToDb( meteo::field::DataDesc  );
/**
 * СОохраняем все данные (вырезанные фактические) в базу
 */
Analyser&         saveFactDataToDb( meteo::field::DataDesc  );


//  * и сохраняем поле в базе
Analyser&         makeObanalAndSaveField();
//  * заполнение вырезанного квадрата данными грибов
Analyser&         fillGribData();
//  * заполнение вырезанного квадрата данными дельт
Analyser&         fillDeltaData();
//  * замешиваем грибы с дельтами
Analyser&         mergeGribWithW();
//  * смешивание в одной prepared_data данных наблюдений и скорректированных прогностических данных
//  с весовыми коэффициентами
Analyser&         mergeFactWithGrib();

// забираем данные по наличию станций
// и заполняем массив точками, в которых данных нет
Analyser&         fillLostDataFromStations();
// из списка со станциями
Analyser&         fillDataFromStationList( QList<QString> stlist );
// из списка с координатами
Analyser&         fillDataFromStations( meteo::GeoData* stations_data, bool without_space=false );
// очищаем данные (подготавливаем)
Analyser&         clearData();
// очищаем подготовленные данные
Analyser&         clearPreparedData();
// очищаем вырезанные данные
Analyser&         clearSubstractedData();

/**
 * Переключения режима отладки
 * @return [description]
 */
// отправляем сообщение в дебаг, если надо
Analyser&         debugMSG( QString txt );
Analyser&         setDebugOn();
Analyser&         setDebugOff();

//
// Режим прогнозирования
// если выставлен, то прогнозы берутся за последнюю дату
// иначе берутся за вчера и сравниваются/обучаются на фактических данных на нули сегодняшщнего дня
//
Analyser&         setPredictModeOn();
Analyser&         setPredictModeOff();

//
// Меняем дату на слуедующую с использованием hour_step
// прибавляем к часу hour_step и если он превышает 24 часа, то меняем следующий день
// nextTime()
//
Analyser&         nextTime();
Analyser&         prevTime();
//
// прибавляем один день
//
Analyser&         nextDay();
Analyser&         prevDay();


//
// подключаемся к сервису
//
// подключаемся к произвольному сервису
Analyser&         connect(meteo::settings::proto::ServiceCode srv);
// подключение к сервису исходников
Analyser&         connectToSrcSRV();
// подключение к сервису полей
Analyser&         connectToFieldSRV();
// подключение к сервису данных
Analyser&         connectToSprinfSRV();
// подключение к сервису прогнозов
Analyser&         connectToForecastSRV();

Analyser&         disconnect();

// Проверяем есть ли ошикбки?
bool                      checkError();
// получаем сообщение об ошибке
QString                   getErrorMsg();

/**
 * Делаем объективный анализ
 */
Analyser&         makeDataObanal( meteo::field::DataDesc &fdescr, double predel_grad );
/**
 * Делаем поле из полученных фаkтических данных
 */
Analyser&         makeFactField();

Analyser&         setDateTime(  QDateTime dt );
Analyser&         setHour(  int req_hour_new );
Analyser&         setForecasthour(  int req_forecasthour_new );
Analyser&         setParam( int req_param_new );
Analyser&         setLevelType(  int req_level_type_new );
Analyser&         setLevel( int req_level_new );
Analyser&         setType(  int req_type_new );
Analyser&         setCenter(  int req_center_new );
Analyser&         setModel( int req_model_new );
Analyser&         setNetType( int net_type );
Analyser&         setFieldDescription( int req_field_description_new );
Analyser&         setHourStep( int req_hour_step );
Analyser&         setSynopStationType(){ this->req_station_type_=meteo::sprinf::kStationSynop; return *this; };
Analyser&         setAeroStationType(){ this->req_station_type_=meteo::sprinf::kStationAero; return *this; };

/**
 * задаем координаты региона в геопоинтах
 */
Analyser&         setRegion( meteo::GeoPoint lefttop, meteo::GeoPoint rightbottom );
/**
 * или в градусах поштучно
 * координаты верхнего левого и правого нижнего углов
 */
Analyser&         setRegion( double lefttop_lat, double lefttop_lon, double rightbottom_lat, double rightbottom_lon );
// устанавливаем центр, от имени которого сохраняем в прогностический
// 253
Analyser&         setPredictedCenter() { this->req_save_center_=253; return *this; }
Analyser&         setRepairedFactCenter() { this->req_save_center_=253; return *this; }
//устанавливаем переменную prepared_data, чтобы потом проводить объективный анализ
Analyser&         setPreparedData( meteo::GeoData prepared_data_new );


//
//  === GETTERS ====
//
int                       getHour(){ return this->req_hour_; }
QDateTime                 getDateTime( );
obanal::TField&           getAllgribField();
meteo::GeoData&           getAllData();

// получаем подготовленные данные
meteo::GeoData&           getPreparedData();

//
// получаем список станций с координатами
// (аэрологические)
//
// все станции
meteo::GeoData            getStationsCoords();
// по списку
meteo::GeoData            getStationsCoords(QList<QString> stlist);
// по запросу
meteo::GeoData            getStationsCoords(meteo::sprinf::MultiStatementRequest &request);

//
// Получаем пункты дислокации (координаты станций, в которых дислоцируется часть)
//
meteo::GeoData            getAllPunktsFromSRV();

// считаем дельту между прогностическим полем и фактическим в
// заданной точке
double                    getDeltaOnPoint(meteo::GeoPoint point);

//  проверяем попадает ли параметр в градацию
bool                      isParamInGradation( QString param, double delta );

//
// получаем заполненный описательный формат
// из переменных, которые заданы внутри класса
//
meteo::field::DataDesc   getFilledDescr();

//
// Удаляем регион из фактических данных
//
Analyser&         substractSquareFromSrcData();

private:

  int req_hour_              = 0,      //срок за который берем данные
      req_forecasthour_      = 12,     //заблаговременность прогнозов, запрашиваемых через грибы
      req_param_             = 12101,  //значение запрашиваемого дескриптора
      req_level_type_        = 100,
      req_level_             = 850,
      req_type_              = meteo::surf::kAeroFix;     //тип данных (аэрология, приземные и т.д.)
  int req_station_type_      = meteo::sprinf::kStationAero;
      // для грибов_
  int req_center_            = 34,
      req_model_             = 4,
      req_net_type_          = 4; // 1 градус по умолчанию

      // центр, в которой сохраняем
  int req_save_center_       = 252;

      // для полей
  int req_field_description_ = 9100000;

  // вырезаемый регион
  double lefttop_lat_      =  70  ,
         lefttop_lon_      =  5   ,
         rightbottom_lat_  =  40  ,
         rightbottom_lon_  =  50  ;

  // шаг в часах для функции nextTime();
  // при вызове функции увеличивается час
  // если час больше 24, то меняется дата
  // нужно для итеративных запросов
  int hour_step_           = 3;

  QDateTime*     req_date_;

  bool isDebug_            = false;

  //
  // Режим прогнозирования
  // если выставлен, то прогнозы берутся за последнюю дату
  // иначе берутся за вчера и сравниваются/обучаются на фактических данных на нули сегодняшщнего дня
  //
  bool predictMode_        = false;

  // сообщение об ошибке
  QString *errormsg_       = nullptr;


  /**
   *
   *  Переменные для хранения полей
   *
   */

  obanal::TField*           result_field_ = nullptr; // итоговое поле
  obanal::TField*              all_field_ = nullptr; // поле со всеми данными
  obanal::TField*         alldelta_field_ = nullptr; // все данные дельт
  obanal::TField*          allgrib_field_ = nullptr; // все грибы

  meteo::GeoData*          prepared_data_ = nullptr; // подготовленные данные
  meteo::GeoData*       substracted_data_ = nullptr; // вырезаднные данные
  meteo::GeoData*               all_data_ = nullptr; // все данные наблюдений
  meteo::GeoData* substracted_delta_data_ = nullptr; // вырезанные данные дельт
  meteo::GeoData*  substracted_grib_data_ = nullptr; // вырезанные данные грибов

  // коннектор к сервисам
  meteo::rpc::Channel* ctrl_       = nullptr;
  // класс работы с базой данных полей
  //meteo::field::TFieldDataDb* db_ = nullptr;
};

}
}

#endif
