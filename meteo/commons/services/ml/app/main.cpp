#include <stdlib.h>


#include <meteo/commons/services/ml/methods/ml.h>
#include <meteo/commons/services/ml/analyser/analyser.h>
#include <meteo/commons/services/ml/analyser/analyser.h>
#include <meteo/commons/services/ml/analyser/processes.h>

#include <cross-commons/debug/tlog.h>
#include <meteo/commons/services/obanal/tobanalservice.h>
#include <meteo/commons/services/obanal/tobanaldb.h>
#include <meteo/commons/global/global.h>

#include <qcoreapplication.h>

using commons::ArgParser;
using commons::Arg;

namespace {


const Arg kHelp                           = Arg::make( "-hh",  "--hhelp"  );
const Arg kDebugMode                      = Arg::make( "-dbg",  "--debug"  );
const Arg kForecastOpr                    = Arg::make( "-o",  "--opr"   );
const Arg kForecastCalculate              = Arg::make( "-cl",  "--calc"   );
const Arg kForecastML                     = Arg::make( "-ml",  "--machinelearning"   );
const Arg kForecastMLPredict              = Arg::make( "-pt",  "--predict"   );
const Arg kForecastMLRestore              = Arg::make( "-res",  "--restore"   );
const Arg kForecastCalcStationModelRating = Arg::make( "-r",  "--ratemodel"   );
const Arg kForecastProcessML              = Arg::make( "-M",  "--processml"   );

const Arg kModelParam                     = Arg::make( "-m", "--model", true );
const Arg kCenterParam                    = Arg::make( "-c", "--center", true );
const Arg kNettypeParam                   = Arg::make( "-nt", "--nettype", true );
const Arg kLevelParam                     = Arg::make( "-lvl", "--level", true );
const Arg kLevelTypeParam                 = Arg::make( "-lvt", "--level-type", true );
const Arg kParamParam                     = Arg::make( "-pm", "--param", true );
const Arg kHourParam                      = Arg::make( "-hr", "--hour", true );

/**
 * прогнозируем и восстанавливаем поля не по прогнозам, а по фактическим данным
 * у земли
 */
const Arg kForecastMLS                     = Arg::make( "-ms",  "--machinelearning-surf"   );
const Arg kForecastMLPredictS              = Arg::make( "-ps",  "--predict-surf"   );
const Arg kForecastMLRestoreS              = Arg::make( "-rs",  "--restore-surf"   );


// обучаем на фактической информации
const Arg kForecastMLFact                 = Arg::make( "-mlf",  "--mlfact"   );

// дата которому будем считать
const Arg kDate                           = Arg::make( "-d",  "--date", true   );

// Режим дебага, если задано то выводим всю отладочную информацию
bool debugMode = false;

std::string paramhelp( const Arg& arg, const char* descr )
{
  Q_UNUSED(descr);
  QString str = QObject::tr("    %1")
      .arg( arg.help() ).leftJustified(40);
  return (str+descr).toStdString();
}

void usage()
{
  std::cout
      << '\n'
      << QObject::tr("Использование: ").toStdString()
      << qApp->applicationName().toStdString()
      << QObject::tr(" [параметры]").toStdString()
      << '\n' << '\n';
  std::cout
      << QObject::tr("Параметры:").toStdString()
      << '\n';
  std::cout << paramhelp( kHelp,                      "Показать эту справку и выйти"                ) << '\n';
  std::cout << paramhelp( kDebugMode,                 "Вывести отладочную информацию"                ) << '\n';
  std::cout << paramhelp( kForecastOpr,               "Расчет оправдываемости прогнозов"           ) << '\n';
  std::cout << paramhelp( kForecastCalculate,         "Расчет и заполнение весов с грибами"        ) << '\n';
  std::cout << paramhelp( kForecastML,                "Обучение нейросети, возмжно указать дату в формате -d YYYY-MM-DD"        ) << '\n';
  std::cout << paramhelp( kForecastMLPredict,         "Создаем поле на основе обученной нейросети, возмжно указать дату в формате -d YYYY-MM-DD"        ) << '\n';
  std::cout << paramhelp( kForecastMLRestore,         "Восстанавливаем поле с параметрами за прошедший срок, возмжно указать дату в формате -d YYYY-MM-DD"        ) << '\n';
  std::cout << paramhelp( kForecastCalcStationModelRating, "Пересчитываем рейтинг моделей по станции"        ) << '\n';
  std::cout << paramhelp( kForecastProcessML, "Прогресс выполнения очереди на обучения"        ) << '\n';

  std::cout << paramhelp( kModelParam,    "Номер модели по которой обучаемся"    ) << '\n';
  std::cout << paramhelp( kCenterParam,   "Номер центра который генерирует модель"    ) << '\n';
  std::cout << paramhelp( kNettypeParam,  "Тип сетки (2,4)"    ) << '\n';
  std::cout << paramhelp( kLevelParam,    "Уровень (850, 700, 500)"    ) << '\n';
  std::cout << paramhelp( kLevelTypeParam,"Тип Уровня (0,100)"    ) << '\n';
  std::cout << paramhelp( kParamParam,    "Номер параметра (12101)"    ) << '\n';
  std::cout << paramhelp( kHourParam,     "За какой срок (00, 12)"    ) << '\n';


  /**
   * работаем с фактическими данными и нейросетями на фактической информации
   */
  std::cout << paramhelp( kForecastMLS,                "Обучение нейросети по фактическим данным, возмжно указать дату в формате -d YYYY-MM-DD"        ) << '\n';
  // std::cout << paramhelp( kForecastMLPredictS,         "Создаем поле на основе обученной нейросети по фактическим данным, возмжно указать дату в формате -d YYYY-MM-DD"        ) << '\n';
  std::cout << paramhelp( kForecastMLRestoreS,         "Восстанавливаем поле по нейросети обученной на ФП, возмжно указать дату в формате -d YYYY-MM-DD"        ) << '\n';
}
}


/**
 * @brief Выводим доступные поля для восстановления
 * 
 * @param date 
 * @param kNettype 
 * @param kLevel 
 * @param kLevelType 
 * @param kParam 
 * @param kHour 
 */
void printAvailableData(QString model, QString center, QString date, QString kNettype, QString kLevel, QString kLevelType, QString kParam, QString kHour){
  Q_UNUSED(model);
  Q_UNUSED(center);
  Q_UNUSED(date);
  Q_UNUSED(kNettype);
  Q_UNUSED(kLevel);
  Q_UNUSED(kLevelType);
  Q_UNUSED(kParam);
  Q_UNUSED(kHour);
  meteo::field::DataRequest req;
  req.set_date_start(date.toStdString());
  req.set_date_end(date.toStdString());
  // req.set_center( center.toInt() );
  // req.set_model( model.toInt() );
  // req.add_level( kLevel.toInt() );
  // int _h = 0;
  req.add_hour( 0 );
  req.add_meteo_descr( kParam.toInt() );
  auto ctrl = std::unique_ptr<meteo::rpc::Channel>(meteo::global::serviceChannel( meteo::settings::proto::kField ));
  if(nullptr == ctrl) {
    error_log << QObject::tr("Ошибка подключения к сервису доступа к результатам ОА");
    return;
  }

  meteo::field::DataDescResponse* resp = ctrl->remoteCall( &meteo::field::FieldService::GetAvailableData, req, 300000 );
  if (nullptr == resp) {
    error_log << QObject::tr("При попытке выполнить расчет индексов неустойчивости ответ от сервиса полей не получен");
  }
  debug_log<<QString("Доступные поля на дату %1 ").arg(date);
  debug_log<<QString("|%1|%2|%3|%4|%5|%6|%7|%8|%9|%10|").arg( "________date_______", 19).arg( "_descr_", 7).arg( "_dcode_", 7).arg( "_hour__", 7).arg( "_model_", 7).arg( "_center", 7).arg( "__net__", 7).arg( "levtype", 7).arg( "__lvl__", 7).arg( "count_p", 7);
  for (auto one: resp->descr()) {
    debug_log<<QString("|%1|%2|%3|%4|%5|%6|%7|%8|%9|%10|").arg(QString::fromStdString(one.date()),19).arg( QString::fromStdString(one.descr_name()), 7).arg( one.meteodescr(), 7).arg( one.hour(), 7).arg( one.model(), 7).arg( one.center(), 7).arg( one.net_type(), 7).arg( one.level_type(), 7).arg( one.level(), 7).arg( one.count_point(), 7);
  }
}




/**
 * @brief Выводим доступные модели
 * 
 * @param date 
 * @param kNettype 
 * @param kLevel 
 * @param kLevelType 
 * @param kParam 
 * @param kHour 
 */
void printAvailableMLData(QString model, QString center, QString date, QString kNettype, QString kLevel, QString kLevelType, QString kParam, QString kHour){
  Q_UNUSED(model);
  Q_UNUSED(center);
  Q_UNUSED(date);
  Q_UNUSED(kNettype);
  Q_UNUSED(kLevel);
  Q_UNUSED(kLevelType);
  Q_UNUSED(kParam);
  Q_UNUSED(kHour);
  meteo::field::DataRequest req;
  req.set_date_start(date.toStdString());
  req.set_date_end(date.toStdString());
  // req.set_center( center.toInt() );
  // req.set_model( model.toInt() );
  // req.add_level( kLevel.toInt() );
  // int _h = 0;
  req.add_hour( 0 );
  req.add_meteo_descr( kParam.toInt() );
  auto ctrl = std::unique_ptr<meteo::rpc::Channel>(meteo::global::serviceChannel( meteo::settings::proto::kField ));
  if(nullptr == ctrl) {
    error_log << QObject::tr("Ошибка подключения к сервису доступа к результатам ОА");
    return;
  }

  meteo::field::DataDescResponse* resp = ctrl->remoteCall( &meteo::field::FieldService::GetAvailableData, req, 300000 );
  if (nullptr == resp) {
    error_log << QObject::tr("При попытке выполнить расчет индексов неустойчивости ответ от сервиса полей не получен");
  }
  debug_log<<QString("Доступные поля на дату %1 ").arg(date);
  debug_log<<QString("|%1|%2|%3|%4|%5|%6|%7|%8|%9|%10|").arg( "________date_______", 19).arg( "_descr_", 7).arg( "_dcode_", 7).arg( "_hour__", 7).arg( "_model_", 7).arg( "_center", 7).arg( "__net__", 7).arg( "levtype", 7).arg( "__lvl__", 7).arg( "count_p", 7);
  for (auto one: resp->descr()) {
    debug_log<<QString("|%1|%2|%3|%4|%5|%6|%7|%8|%9|%10|").arg(QString::fromStdString(one.date()),19).arg( QString::fromStdString(one.descr_name()), 7).arg( one.meteodescr(), 7).arg( one.hour(), 7).arg( one.model(), 7).arg( one.center(), 7).arg( one.net_type(), 7).arg( one.level_type(), 7).arg( one.level(), 7).arg( one.count_point(), 7);
  }
}



void startAnalyse(){

  meteo::field::DataAnalyseRequest request;

  request.add_centers(4);
  request.add_centers(34);
  request.add_centers(74);
  request.set_days_interval(-10);
  request.set_step(-3600*6);

  meteo::rpc::Channel* field_ch = meteo::global::serviceChannel( meteo::settings::proto::kField );
  if ( 0 == field_ch ) {
      debug_log << QObject::tr("Не удалось подключиться к сервису объективного анализа данных.");
      return;
    }

  meteo::field::SimpleDataReply * reply =
      field_ch->remoteCall( &meteo::field::FieldService::CalcForecastOpr, request,  300000, true);
  if ( nullptr == reply ) {
      debug_log << QObject::tr("При попытке получить анализ данных ответ от сервиса данных не получен");
      delete field_ch;
      return;
    }

  // debug_log << reply->Utf8DebugString();
  delete field_ch;
  delete reply;
}


/**
 * Строим прогностическое поле на основе данных, которых не хватает
 * (не пришли данные со станций)
 */
void startForecastCalculate(QString kModel,QString kCenter,QString kNettype,QString kLevel,QString kLevelType,QString kParam,QString kHour){

  meteo::ml::Analyser* analyser = new meteo::ml::Analyser();

  if ( debugMode ){
    analyser->setDebugOn();
  }

  QList<int> models = QList<int>() << kModel.toInt();
  //
  // Все прогностические центры,по которым будем считать
  //
  QList<int> centers = QList<int>() << kCenter.toInt();
  //
  // Типы сеток для запроса прогнозов
  //
  QList<int> nettypes = QList<int>() << kNettype.toInt();

  //
  // Стандартные изобарические уровни, для которых рассчитываем поля
  //
  QList<int>  levels = QList<int>() << kLevel.toInt();
  //
  // Метеопараметры, по которым анализируем поля
  //
  QList<int>  params = QList<int>() << kParam.toInt();
  //
  // Сроки за которые анализируем
  //
  QList<int> hours = QList<int>() << kHour.toInt();
  int level_type = kLevelType.toInt();
  // устанаваливаем отладку
  // analyser->setDebugOn()
          // устанавливаем множитель десктриптора под которым будем сохранять
          // в 0 (т.е. не используем)
  analyser->setFieldDescription(0);

  int mi=0;
  for (int m : models) {
    // модель
    analyser->setModel(m);

    // for (int c : centers ) { // центра совпадают с моделями
      // центр
      // analyser->setCenter(c);
       analyser->setCenter(centers.at(mi)); // сделаем так, чтобы один центр - одна модель, теперь они совпадают
       analyser->setNetType(nettypes.at(mi));
       analyser->setLevelType(level_type);
      
      for (int l : levels ){
        // уровень
        analyser->setLevel(l);

        for (int p : params ){
          // метеопараметр
          analyser->setParam(p);

          for (int h : hours ){
              if(!analyser->getErrorMsg().isEmpty()){
                warning_log << analyser->getErrorMsg();
                }
            // час и запускаем
            analyser->setHour(h)
                    .clearData()
                    .startForecastLostDataCalculate();
            debug_log<<"============================= END CALCULATE ===============================================";
          }
        }
      }
    // }
    mi++;
  }
  return;
}






/**
 * обучаем модель нейросети для каждого параметра
 *
 */
void startMLCalculate(QString dateto,QString kModel,QString kCenter,QString kNettype,QString kLevel,QString kLevelType,QString kParam,QString kHour){
  meteo::ml::MethodML* forecastml = new meteo::ml::MethodML();
  // forecastml->setDebugOn()
  forecastml->setDateTime( QDateTime::fromString(dateto,Qt::ISODate) );

  //
  // Все прогностические модели,по которым будем считать
  //
  QList<int> models = QList<int>() << kModel.toInt();
  // << 15
                                   // << 96
                                   // << 4
                                   // << 149
                                   // << 45 ;
  //
  // Все прогностические центры,по которым будем считать
  //
  QList<int> centers = QList<int>() << kCenter.toInt();
  // << 4  // 15 model
                                    // << 7  // 96
                                    // << 34 // 4
                                    // << 98 // 149
                                    // << 74; // 1

  //
  // Типы сеток для запроса прогнозов
  //
  QList<int> nettypes = QList<int>() << kNettype.toInt();
  // << 2
                                     // << 4 ;
  //
  // Стандартные изобарические уровни, для которых рассчитываем поля
  //
  QList<int>  levels = QList<int>() << kLevel.toInt();
  // << 850 ;
                                    // << 700
                                    // << 500
                                    // << 300
                                    // << 200
                                    // << 100 ;

  //
  // Метеопараметры, по которым анализируем поля
  //
  QList<int>  params = QList<int>() << kParam.toInt();
  // << 12101;
                                    // << 10051
                                    // << 10004
                                    // << 12108
                                    // << 12103
                                    // << 11003
                                    // << 11004;

  //
  // Сроки за которые анализируем
  //
  QList<int> hours = QList<int>() << kHour.toInt();
  // << 00;
                                  // << 12;
  int level_type = kLevelType.toInt();  
  meteo::ml::Analyser* analyser = new meteo::ml::Analyser();

  // устанаваливаем отладку
  if ( debugMode ){
    analyser->setDebugOn();
  }
  
        // устанавливаем множитель десктриптора под которым будем сохранять
        // в 0 (т.е. не используем)
  analyser->setDateTime(QDateTime::fromString(dateto,Qt::ISODate))
        .setFieldDescription(0);


  int mi=0;
  for (int m : models) {


    // модель
    analyser->setModel(m);

    // for (int c : centers ) { // центра совпадают с моделями
      // центр
      // analyser->setCenter(c);
      analyser->setCenter( centers.at(mi) ); // сделаем так, чтобы один центр - одна модель, теперь они совпадают
      analyser->setNetType(nettypes.at(mi));
      analyser->setLevelType(level_type);
      for (int l : levels ){
        // уровень
        analyser->setLevel(l);
        

        for (int p : params ){
          // метеопараметр
          analyser->setParam(p);

          for (int h : hours ){
              if(!analyser->getErrorMsg().isEmpty()){
                  error_log << analyser->getErrorMsg();
                }
            //
            // выставляем параметры
            //
            analyser->setHour(h);
            analyser->clearData();
            //
            // запускаем расчеты нейросетей
            //
            forecastml->setAnalyzer(analyser)
                      .trainKRLSTodayModel();
            debug_log<<"============================= END Machine learning "<<m<<" " << centers.at(mi) <<" ==========================================";


          }
        }
      }
    // }
    mi++;
  }


  delete forecastml;
  return;
}



/**
 * Начинаем прогнозировать поля, в соответствии с обученными моделями
 */
void startMLPredict(QString dateto,QString kModel,QString kCenter,QString kNettype,QString kLevel,QString kLevelType,QString kParam,QString kHour){

  meteo::ml::Analyser* analyser = new meteo::ml::Analyser();
  // устанаваливаем отладку
  if ( debugMode ){
    analyser->setDebugOn();
  }
  // analyser->setDebugOn()
  analyser->setDateTime( QDateTime::fromString(dateto,Qt::ISODate) )
          .setFieldDescription(0);

  meteo::ml::MethodML* forecastml = new meteo::ml::MethodML();
  // forecastml->setDebugOn();

  //
  // Все прогностические модели,по которым будем считать
  //
  QList<int> models = QList<int>() << kModel.toInt();
  // << 15
                                   // << 96
                                   // << 4
                                   // << 149
                                   // << 45 ;

  //
  // Все прогностические центры,по которым будем считать
  //
  QList<int> centers = QList<int>() << kCenter.toInt();
  // << 4  // 15 model
                                    // << 7  // 96
                                    // << 34 // 4
                                    // << 98 // 149
                                    // << 74; // 1

  //
  // Типы сеток для запроса прогнозов
  //
  QList<int> nettypes = QList<int>() << kNettype.toInt();
  // << 2
                                     // << 4 ;

  //
  // Стандартные изобарические уровни, для которых рассчитываем поля
  //
  QList<int>  levels = QList<int>() << kLevel.toInt();
  // << 850;
                                    // << 700
                                    // << 500
                                    // << 300 ;

  //
  // Метеопараметры, по которым анализируем поля
  //
  QList<int>  params = QList<int>() //<< 10051
                                    << kParam.toInt();
                                    // << 12101;
                                    // << 10004
                                    // << 12108
                                    // << 12103
                                    // << 11003
                                    // << 11004;

  //
  // Сроки за которые анализируем
  //
  QList<int> hours = QList<int>() << kHour.toInt();
  // << 00;
                                  // << 12;

  // устанавливаем загрузчик полей
  forecastml->setAnalyzer(analyser);
  int level_type = kLevelType.toInt();  
  int mi=0;
  for (int m : models) {


    // модель
    analyser->setModel(m);

    // for (int c : centers ) { // центра совпадают с моделями
      // центр
      // analyser->setCenter(c);
      analyser->setCenter( centers.at(mi) ); // сделаем так, чтобы один центр - одна модель, теперь они совпадают
      analyser->setNetType( nettypes.at(mi) ); //устанавливаем тип сетки
      analyser->setLevelType(level_type);
      for (int l : levels ){
        // уровень
        analyser->setLevel(l);
        for (int p : params ){
          // метеопараметр
          analyser->setParam(p);

          for (int h : hours ){
            error_log << analyser->getErrorMsg();
            //
            // выставляем параметры
            //
            analyser->setHour(h)
                    .clearData()
                    .setPredictModeOn()   //включаем режим прогнозирования. Нужно для времени. берем поля за 12 часов сего дня
                    .setPredictedCenter() //устанавливаем центр проностический типа
                    .getGribDataFromSRV();
            //
            // запускаем прогнозы
            //
            forecastml->predictOnStations();
            analyser->makeObanalAndSaveField();
            debug_log<<"============================= END Predicting =====================================";


          }
        }
      }
    // }
    mi++;
  }
  //
  // записываем данные в поле
  //
  debug_log<<"Закончили прогнозировать";

  return;
}


/**
 * Начинаем восстанавливаеть поля, в соответствии с обученными моделями
 * за сегодняшнюю дату
 */
void startMLRestore(QString dateto,QString kModel,QString kCenter,QString kNettype,QString kLevel,QString kLevelType,QString kParam,QString kHour){

  meteo::ml::Analyser* analyser = new meteo::ml::Analyser();
  // устанаваливаем отладку
  if ( debugMode ){
    analyser->setDebugOn();
  }
  // analyser->setDebugOn()
  analyser->setDateTime( QDateTime::fromString(dateto,Qt::ISODate) )
          .setFieldDescription(0);

  analyser->setHour(kHour.toInt());
  printAvailableData( kModel,  kCenter, analyser->getDateTime().toString(Qt::ISODate),  kNettype,  kLevel,  kLevelType,  kParam,  kHour);


  meteo::ml::MethodML* forecastml = new meteo::ml::MethodML();
  // forecastml->setDebugOn();

  //
  // Все прогностические модели,по которым будем считать
  //
  QList<int> models = QList<int>() << kModel.toInt();
                                  // << 15
                                   // << 96
                                   // << 4
                                   // << 149
                                   // << 45 ;

  //
  // Все прогностические центры,по которым будем считать
  //
  QList<int> centers = QList<int>() << kCenter.toInt();
  // QList<int> centers = QList<int>() << 4  // 15 model
                                    // << 7  // 96
                                    // << 34 // 4
                                    // << 98 // 149
                                    // << 74; // 1

  //
  // Типы сеток для запроса прогнозов
  //
  QList<int> nettypes = QList<int>() << kNettype.toInt();
  // QList<int> nettypes = QList<int>() << 2
  //                                    << 4 ;

  //
  // Стандартные изобарические уровни, для которых рассчитываем поля
  //
  QList<int>  levels = QList<int>() << kLevel.toInt();
  // QList<int>  levels = QList<int>() << 850;
                                    // << 700
                                    // << 500
                                    // << 300 ;
  int level_type = kLevelType.toInt();
  //
  // Метеопараметры, по которым анализируем поля
  //
  QList<int>  params = QList<int>() << kParam.toInt();
                                    //<< 10051
                                    // << 12101;
                                    // << 10004
                                    // << 12108
                                    // << 12103
                                    // << 11003
                                    // << 11004;
  //
  // Сроки за которые анализируем
  //
  QList<int> hours = QList<int>() << kHour.toInt();
                                  // << 00;
                                  // << 12;

  // устанавливаем загрузчик полей
  forecastml->setAnalyzer(analyser);

  int mi=0;
  for (int m : models) {
    // модель
    analyser->setModel(m);

    // for (int c : centers ) { // центра совпадают с моделями
      // центр
      // analyser->setCenter(c);
      analyser->setCenter( centers.at(mi) ); // сделаем так, чтобы один центр - одна модель, теперь они совпадают
      analyser->setNetType(nettypes.at(mi));
      analyser->setLevelType(level_type);
      for (int l : levels ){
        // уровень
        analyser->setLevel(l);       

        for (int p : params ){
          // метеопараметр
          analyser->setParam(p);

          for (int h : hours ){
            error_log << analyser->getErrorMsg();
            //
            // выставляем параметры
            //
            analyser->setHour(h)
                    .clearData()
                    .setPredictedCenter() //устанавливаем центр проностический типа
                    .getGribDataFromSRV();
            //
            // запускаем прогнозы
            //
            forecastml->setAnalyzer(analyser);
            // forecastml->predictOnStations();
            // получаем значения на всем натренированном интервале
            forecastml->predictOnAllModels();
            analyser->makeObanalAndSaveField();
            debug_log<<"============================= END RESTORING  =====================================";
          }
        }
      }
    // }
    mi++;
  }
  //
  // записываем данные в поле
  //
  debug_log<<"Закончили восстанавливать";

  return;
}




/**
 *
 *  Пересчитываем рейтинг моделей по станции
 *
 */
void startCalcStationModelRating(){
  meteo::ml::Analyser* analyser = new meteo::ml::Analyser();
  // analyser->setDebugOn()
  analyser->setFieldDescription(0);
  // устанаваливаем отладку
  if ( debugMode ){
    analyser->setDebugOn();
  }
  // получаем координаты станции, по которой рассчитываем оправдываемость
  meteo::GeoData stations_data;
  stations_data = analyser->getAllPunktsFromSRV();

  //
  // Все прогностические модели,по которым будем считать
  //
  QList<int> models = QList<int>() << 15
                                   << 96
                                   << 4
                                   // << 149
                                   << 1 ;

  //
  // Все прогностические центры,по которым будем считать
  //
  QList<int> centers = QList<int>() << 4  // 15 model
                                    << 7  // 96
                                    << 34 // 4
                                    // << 98 // 149
                                    << 74; // 1

  //
  // Типы сеток для запроса прогнозов
  //
  QList<int> nettypes = QList<int>() << 2
                                     << 7
                                     << 3
                                     << 7 ;

  //
  // Стандартные изобарические уровни, для которых рассчитываем поля
  //
  QList<int>  levels = QList<int>() << 850
                                    << 700
                                    << 500 ;

  //
  // Метеопараметры, по которым анализируем поля
  //
  QList<int>  params = QList<int>() <<  12101 ;
                                    // << 12108

  //
  // Сроки за которые анализируем
  //
  QList<int> hours = QList<int>() << 00;
                                  // << 12;

  // рассчитанные приоритеты центров
  QMap<int, double> centers_priority;
  QMap<int, int>    centers_true_forecast;
  QMap<int, int>    centers_forecast_count;
  // отсортированные центра
  //
  // порядковый номер - рейтинг, а значение -номер центра
  //
  QList<int>     center_rating;
  QList<double>  center_rating_value; //сами значения

  // создаем синглтон для работы с дескрипторами
  // надо для того, чтобы из числа получить букву
  meteodescr::TMeteoDescriptor* md = TMeteoDescriptor::instance();

  int mi=0;
  for (int m : models) {
    // модель
    analyser->setModel( m );
    analyser->setCenter( centers.at(mi) ); // сделаем так, чтобы один центр - одна модель, теперь они совпадают
    analyser->setNetType(nettypes.at(mi));
    int count_of_field = 0;

      for (int l : levels ){
        // уровень
        analyser->setLevel(l);
        for (int p : params ){


          // метеопараметр
          analyser->setParam(p);
          for (int h : hours ){
            // получаем грибы с фактикой и обаналим
            analyser->setHour(h)
                     .clearData()
                    //  .setDebugOn()
                     .getGribDataFromSRV()
                     .getFactDataFromSRV()
                     .makeFactField();

            count_of_field++;

            // итерируемся по станциям
            for ( int st=0; st< stations_data.size(); st++ ){

              double delta;
              delta = analyser->getDeltaOnPoint( stations_data.at(st).point );
              // debug_log<<"center: "<<centers.at(mi) <<" paraM:"<<p<<"model: "<<m;

              // проверяем оправдался ли прогноз
              if ( analyser->isParamInGradation( md->name(p), delta ) ){
                // увеличиваем счетчик оправдавшихся прогнорзов
                centers_true_forecast.insert( centers.at(mi), centers_true_forecast.value(centers.at(mi),0)+1 );
              }
              // увеличиваем количество пройденных прогнозов
              centers_forecast_count.insert( centers.at(mi), centers_forecast_count.value(centers.at(mi),0)+1 );
            }
          }
        }
      }
    mi++;
  }

  // создаем объект работы с базой
  TObanalDb* obanal_db;
  obanal_db = new TObanalDb();

  // centers_priority
  // итератор
  QMapIterator<int, int> it(centers_forecast_count);
  int priority,
      forecast_count,
      forecast_true_count;
  double priority_val;

  while ( it.hasNext() ) {
    it.next();

    //
    // загружаем приоритеты из базы
    //
    obanal_db->loadCenterPriority( it.key(), priority, priority_val, forecast_count, forecast_true_count );
    // суммируем
    forecast_true_count += centers_true_forecast.value(it.key());
    forecast_count      += centers_forecast_count.value(it.key());

    // debug_log << "forecast_true_count" << forecast_true_count;
    // debug_log << "forecast_count" << forecast_count;

    // чтобы не было деления на ноль
    if ( forecast_count==0 ){
      priority_val = 0.0;
    }else{
      priority_val = double(forecast_true_count)/double(forecast_count);
    }

    // записываем для анализа
    centers_priority.insert( it.key(), priority_val );
    center_rating_value  <<  priority_val;
    debug_log << "priority_val" << priority_val;
    // и сохраняем для базы
    centers_true_forecast.insert( it.key()  , forecast_true_count );
    centers_forecast_count.insert( it.key() , forecast_count );
  }


  // сортируем по значению
  qSort(center_rating_value);

  QMapIterator<int, double> itp(centers_priority);
  //
  // пока размеры массивов не выровняются
  // сортируем
  //
  for ( int k=0; k< centers_priority.size(); k++ ){
    // debug_log<<"while cycle";

    for ( double val: center_rating_value ){
      // возвращаем взад
      itp.toFront();

      // debug_log<< "center_rating_value: "<< val;

      while ( itp.hasNext() ) {
        itp.next();
        // если находим то, что надо
        // идентичное по значению и не встречавшееся ранее
        // debug_log << val << "=" << itp.value() <<"; key"<< itp.key() <<center_rating.contains( itp.key() );

        if ( MnMath::isEqual( val, itp.value() ) && !center_rating.contains( itp.key() ) ){
          center_rating<<itp.key();
          itp.toBack();
        }

      }

    }

  }

  //
  // Обнуляем приоритеты
  //
  for (int i = 0; i < 290; ++i)
  {
    if ( false==obanal_db->saveCentersPriority( i, 0, 0, -1, -1 ) ){
      debug_log<<"Невозможно записать в базу данные по сортировке центра";
    }
  }

  //
  // записываем в базу рейтинги
  //
  // debug_log<<" center_rating: ";
  int i=0;

  for ( int val: center_rating ) {

    if ( false==obanal_db->saveCentersPriority( val, i+1, center_rating_value.at(i),
                                                centers_forecast_count.value(val),
                                                centers_true_forecast.value(val) ) ){
      debug_log << "Невозможно записать в базу данные по сортировке центра";
    }
    i++;
  }//endfor

  delete md;

  return;
}


/**
 *
 * Запускаем процессы дообучения
 *
 *
 * разбиваем процесс на чанки, запрашиваем информацию дозированно
 * обучаем тоже дозированно - загружаем предобученную модель, дообучаем, сохраняем обратно
 *
 */

void processMachineLearning(){

  // класс работы с полями и базой
  meteo::ml::Analyser* analyser = new meteo::ml::Analyser();
  // устанаваливаем отладку
  if ( debugMode ){
    analyser->setDebugOn();
  }
  analyser->setHourStep(12) //устанавливаем 12-часовой интервал
          // .setDateTime( QDateTime::currentDateTimeUtc().addDays(-90) )
          .setFieldDescription(0);

  // класс работы с машинным обучением
  meteo::ml::MethodML* forecastml = new meteo::ml::MethodML();
  // forecastml->setDebugOn()
            // устанавливаем загрузчик полей
  forecastml->setAnalyzer(analyser);

  //
  // 1 получаем из базы список незавершенных процессов обучения
  //
  meteo::ml::Processes* process = new meteo::ml::Processes();

  // содержимое для файла модели
  QByteArray ba;
  ba.clear();


  // process->setDebugOn()
  process->setProcessType("learnregion")
         .loadProcess()
  // рассчитываем шаги, если не загрузились еще
  //
         .calcStepFromDate( 90 )
  // 2 загружаем из базы модель выбранного процесса
  //
         .getFile(ba);


  analyser->setDateTime( QDateTime::fromString(process->getLastVal(), Qt::ISODate) );


  // если станций нет - заполняем своими
  if ( process->getStations().count()==0 ){
    process->addStation("26075")
           .addStationPdc("26038")
           .addStationPdc("26298")
           .addStationPdc("22820")
           .addStationPdc("26477");
  }

  // если станций нет - заполняем своими
  if ( process->getParams().count()==0 ){
    process->addParams("12101")
           .addParamsPdc("12101")
           .addParamsPdc("12108")
           .addParamsPdc("11003")
           .addParamsPdc("11004");
  }

  // 3 делаем запрос на получение данных для дообучения модели
  //   проверяем, есть ли вообще данные,
  //
  analyser->setModel(251)
          .setCenter(251)
          .setLevel(850)
          .setHour(0)
          .clearData();

  error_log << analyser->getErrorMsg();

  // 4 скармливаем данные модели, дообучаем
  // устанавливаем параметры, по которым будем прогнозировать

  if (ba.size()>0){
    // устанавливаем загруженную модель
    forecastml->setModel(ba);
  }

  // список станций, по которым нужны прогнозы и которые используются для обучения
  forecastml->setForPredictParamList( process->getParamsPdc() )
            .setPredictParamList( process->getParams() )
            .fillMatrixForTrain( process->getStations(), process->getStationsPdc() )
            .dnnTrain();
  // 5 сохраняем модель
  //
  ba = forecastml->getModel();

  process->saveFile(ba)
  // 6 сохраняем текущее состояние процесса обучения
  //
         .process()
         .setLastVal( analyser->getDateTime().toString(Qt::ISODate) )
         .saveProcess();
  return;
}

/**
 *
 * =================== Прогнозируем по фактическим данным =====================
 *
 */


/**
 * @brief Обучаем модель по данным фактических наблюдений (полям)
 * 
 * @param date 
 * @param kModel 
 * @param kCenter 
 * @param kNettype 
 * @param kLevel 
 * @param kParam 
 * @param kHour 
 */
void startMLSCalculate(QString date,QString kNettype,QString kLevel,QString kLevelType,QString kParam,QString kHour){
  meteo::ml::MethodML* forecastml = new meteo::ml::MethodML();

  // Все модели фактики (250)
  QList<int> models = QList<int>() << 250;
  // Все  центры (250)
  QList<int> centers = QList<int>() << 250;
  // Типы сеток для запроса прогнозов
  QList<int> nettypes = QList<int>() << kNettype.toInt();
  // Стандартные изобарические уровни, для которых рассчитываем поля
  QList<int>  levels = QList<int>() << kLevel.toInt();
  // Метеопараметры, по которым анализируем поля
  QList<int>  params = QList<int>() << kParam.toInt();
  // Сроки за которые анализируем
  QList<int> hours = QList<int>() << kHour.toInt();
  meteo::ml::Analyser* analyser = new meteo::ml::Analyser();
  // устанаваливаем отладку
  if ( debugMode ){
    analyser->setDebugOn();
  }

  analyser->setDateTime(QDateTime::fromString(date,Qt::ISODate))
        .setFieldDescription(0);
  int level_type = kLevelType.toInt();  
  int mi=0;
  for (int m : models) {
    // модель
    analyser->setModel(m);
    analyser->setCenter( centers.at(mi) ); // сделаем так, чтобы один центр - одна модель, теперь они совпадают
    analyser->setNetType(nettypes.at(mi));
    analyser->setLevelType(level_type);
    for (int l : levels ){
      // уровень
      analyser->setLevel(l);
      
      for (int p : params ){
        // метеопараметр
        analyser->setParam(p);

        for (int h : hours ){
            if(!analyser->getErrorMsg().isEmpty()){
                error_log << analyser->getErrorMsg();
              }
          //
          // выставляем параметры
          //
          analyser->setHour(h);
          analyser->clearData();
          //
          // запускаем расчеты нейросетей
          //
          forecastml->setAnalyzer(analyser)
                    .trainKRLSTodayFactModel();
          debug_log<<"============================= END Machine learning "<<m<<" " << centers.at(mi) <<" ==========================================";
        }
      }
    }
    // }
    mi++;
  }


  delete forecastml;
  return;
}



/**
 * @brief Восстановление полей по станциям на основе моделей обученных на фактических данных
 * 
 * @param date дата, за которую необходимо восстановить данные
 * @param kNettype 
 * @param kLevel 
 * @param kLevelType 
 * @param kParam 
 * @param kHour  срок, за который пытаемся восстановить данные
 */
void startMLSRestore(QString date,QString kNettype,QString kLevel,QString kLevelType,QString kParam,QString kHour){
  
  meteo::ml::Analyser* analyser = new meteo::ml::Analyser();
  
  analyser->setDateTime( QDateTime::fromString(date,Qt::ISODate) )
          .setFieldDescription(0);

  analyser->setHour(kHour.toInt());
  printAvailableData( "250",  "250", analyser->getDateTime().toString(Qt::ISODate),  kNettype,  kLevel,  kLevelType,  kParam,  kHour);

  meteo::ml::MethodML* forecastml = new meteo::ml::MethodML();
  // 

  // устанаваливаем отладку
  if ( debugMode ){
    analyser->setDebugOn();
    forecastml->setDebugOn();
  }

  // Все прогностические модели,по которым будем считать
  QList<int> models = QList<int>() << 250;
  // Все прогностические центры,по которым будем считать
  QList<int> centers = QList<int>() << 250;
  // Типы сеток для запроса прогнозов
  QList<int> nettypes = QList<int>() << kNettype.toInt();
  // Стандартные изобарические уровни, для которых рассчитываем поля
  QList<int>  levels = QList<int>() << kLevel.toInt();
  // Метеопараметры, по которым анализируем поля
  QList<int>  params = QList<int>() << kParam.toInt();
  // Сроки за которые анализируем
  QList<int> hours = QList<int>() << kHour.toInt();
  // устанавливаем загрузчик полей
  forecastml->setAnalyzer(analyser);
  int level_type = kLevelType.toInt();  
  int mi=0;
  bool has_errors = false;
  for (int m : models) {
    // модель
    analyser->setModel(m);
    analyser->setCenter( centers.at(mi) ); // сделаем так, чтобы один центр - одна модель, теперь они совпадают
    analyser->setNetType( nettypes.at(mi) ); //устанавливаем тип сетки
    analyser->setLevelType(level_type);
    for (int l : levels ){
      // уровень
      analyser->setLevel(l);
      for ( int p : params ){
        // метеопараметр
        analyser->setParam(p);

        for (int h : hours ){
          error_log << analyser->getErrorMsg();
          //
          // выставляем параметры
          //
          analyser->setHour(h)
                  .clearData()
                  .setForecasthour(0)
                  // синоптические станции
                  .setSynopStationType()
                  .setRepairedFactCenter() // устанавливаем центр наблюдения за погодой типа
                  // забираем фактику за предшествующий срок
                  .setHourStep(3)
                  .prevTime()
                  .getFieldDataFromSRV()
                  .nextTime();
          //
          // запускаем прогнозы
          //
          forecastml->predictOnStations();
          analyser->makeObanalAndSaveField();
          // если есть ошибки в данных и при обработке, то надо выкатить доступные для анализа поляи данные
          if ( !has_errors && analyser->checkError() ){
            has_errors = true;
          }
          debug_log<<"============================= END Predicting =====================================";
        }
      }
    }
    mi++;
  }
  //
  // записываем данные в поле
  //
  debug_log<<"Закончили прогнозировать";

  // if (has_errors){
  //   printAvailableData( "250",  "250", analyser->getDateTime().toString(Qt::ISODate),  kNettype,  kLevel,  kLevelType,  kParam,  kHour);
  // }

  return;
}

/**
 * Главная функция
 * @param  argc [description]
 * @param  argv [description]
 * @return      [description]
 */
int main( int argc, char* argv[] )
{
  TAPPLICATION_NAME("meteo");
  QCoreApplication* app = new QCoreApplication( argc, argv, false );
  Q_UNUSED( app );
  ::setlocale(LC_NUMERIC, "C"); // локаль для записи координат в mongo (libbson)

  ::meteo::gSettings(::meteo::global::Settings::instance());
  if( !::meteo::global::Settings::instance()->load() ){
    return EXIT_FAILURE;
  }
  meteo::global::setLogHandler();
  if ( 1 == argc ) {
    usage();
    return EXIT_FAILURE;
  }
  ArgParser* options = ArgParser::instance();
  if ( false == options->parse( argc, argv ) ) {
    usage();
    return EXIT_FAILURE;
  }
  QDateTime dt = QDateTime::currentDateTimeUtc();
  int hour = dt.time().hour();
  dt.setTime( QTime( (hour-hour%3),0,0,0) );
  QString date= dt.toString(Qt::ISODate);
  if ( options->at(kDate).value() != QString() ) {
    date = options->at(kDate).value();
  }

  QString model = "45";
  if (options->at(kModelParam).value() !=QString()){
    model = options->at(kModelParam).value();
  }

  QString center = "74";
  if (options->at(kCenterParam).value() !=QString()){
    center = options->at(kCenterParam).value();
  }

  QString kNettype = "4";
  if (options->at(kNettypeParam).value() !=QString()){
    kNettype = options->at(kNettypeParam).value();
  }

  QString kLevel = "850";
  if (options->at(kLevelParam).value() !=QString()){
    kLevel = options->at(kLevelParam).value();
  }

  QString kLevelType = "100";
  if (options->at(kLevelTypeParam).value() !=QString()){
    kLevelType = options->at(kLevelTypeParam).value();
  }

  QString kParam = "12101";
  if (options->at(kParamParam).value() !=QString()){
    kParam = options->at(kParamParam).value();
  }

  QString kHour = "00";
  if (options->at(kHourParam).value() !=QString()){
    kHour = options->at(kHourParam).value();
  }

  if ( true == options->installed(kDebugMode)) {
    debugMode = true;
  }

  if ( true == options->installed(kHelp)) {
    usage();
    return EXIT_SUCCESS;
  }
  if (true == options->installed(kForecastOpr)) {
    startAnalyse();
    return EXIT_SUCCESS;
  }
  if (true == options->installed(kForecastCalculate)) {
    startForecastCalculate(model,center,kNettype,kLevel,kLevelType,kParam,kHour);
    return EXIT_SUCCESS;
  }
  if (true == options->installed(kForecastML)) {
    startMLCalculate(date,model,center,kNettype,kLevel,kLevelType,kParam,kHour);
    return EXIT_SUCCESS;
  }
  if (true == options->installed(kForecastMLS)) {
    if ( options->installed(kLevel)     ==false &&
         options->installed(kLevelType) ==false &&
         options->installed(kParam)     ==false &&
         options->installed(kHour)      ==false ){
      // если не установлен ни один из параметров, то делаем все автоматически
      kHour = QString("%1").arg((hour-hour%3));
      kLevel = "0";
      kLevelType = "1";
      QStringList  params = { "12101", "10051", "12103" };
      for (int i = 0; i < params.size(); ++i){
        // debug_log<< date << kNettype << kLevel << kLevelType << params.at(i) << kHour;
        startMLSCalculate(date,kNettype,kLevel,kLevelType,params.at(i),kHour); 
      }
    }else{
      startMLSCalculate(date,kNettype,kLevel,kLevelType,kParam,kHour);
    }
    return EXIT_SUCCESS;
  }
  if (true == options->installed(kForecastMLPredict)) {
    startMLPredict(date,model,center,kNettype,kLevel,kLevelType,kParam,kHour);
    return EXIT_SUCCESS;
  }

  if (true == options->installed(kForecastMLRestore)) {
    startMLRestore(date,model,center,kNettype,kLevel,kLevelType,kParam,kHour);
    return EXIT_SUCCESS;
  }
  if (true == options->installed(kForecastCalcStationModelRating)) {
    startCalcStationModelRating();
    return EXIT_SUCCESS;
  }

  if (true == options->installed(kForecastProcessML)) {
    processMachineLearning();
    return EXIT_SUCCESS;
  }


  /**
   * нейросети по фактическим данным
   */

  if ( true == options->installed(kForecastMLRestoreS) ) {
    startMLSRestore(date,kNettype,kLevel,kLevelType,kParam,kHour);
    return EXIT_SUCCESS;
  }

  if ( true == options->installed(kForecastMLRestoreS) ) {
    // startForecastMLRestoreS(date);
    return EXIT_SUCCESS;
  }


  usage();

}


