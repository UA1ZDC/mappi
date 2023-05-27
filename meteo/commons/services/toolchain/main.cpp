#include <stdlib.h>

#include <cross-commons/debug/tlog.h>
#include <qcoreapplication.h>
#include <cross-commons/app/options.h>
#include <meteo/commons/settings/settings.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/global/weatherloader.h>
#include <commons/meteo_data/meteo_data.h>

#include <meteo/commons/ui/map/puanson.h>
#include <meteo/commons/punchrules/punchrules.h>

#include <google/protobuf/util/json_util.h>
#include <commons/textproto/pbtools.h>
#include <meteo/commons/proto/sprinf.pb.h>
#include <meteo/commons/proto/field.pb.h>
#include <meteo/commons/proto/surface_service.pb.h>

using commons::ArgParser;
using commons::Arg;

const Arg kHelp                       = Arg::make( "-h",    "--help"  );
const Arg kFactweather                = Arg::make( "-fw",   "--fact-weather" );
const Arg kForecastModel              = Arg::make( "-fm",   "--forecast-model" );
const Arg kForecastStart              = Arg::make( "-df",   "--date-forecast", true );
const Arg kCounrtyList                = Arg::make( "-cl",   "--country-list" );
const Arg kDate                       = Arg::make( "-d",    "--date", true );
const Arg kHourPeriod                 = Arg::make( "-dp",   "--date-period", true );
const Arg kHourParam                  = Arg::make( "-hr",   "--hour", true );
const Arg kCountry                    = Arg::make( "-cn",   "--country", true );
const Arg kStantion                   = Arg::make( "-st",   "--stantion", true );
const Arg kLatLon                     = Arg::make( "-ll",   "--latlon", true );
const Arg kStantionType               = Arg::make( "-tp",   "--stantion-type", true );
const Arg kFormat                     = Arg::make( "-f",    "--format", true );
const Arg kFile                       = Arg::make( "-fn",   "--filename", true );


std::string paramhelp( const Arg& arg, const char* descr )
{
  Q_UNUSED(descr);
  QString str = QObject::tr("    %1")
      .arg( arg.help() ).leftJustified(40);
  return (str+descr).toStdString();
}

void usage() {
  std::cout
      << '\n'
      << QObject::tr("Использование: ").toStdString()
      << qApp->applicationName().toStdString()
      << QObject::tr(" [параметры]").toStdString()
      << '\n' << '\n';
  std::cout
      << QObject::tr("Параметры:").toStdString()
      << '\n';
  std::cout << paramhelp( kHelp,                      "Показать эту справку и выйти" ) << '\n';
  std::cout << paramhelp( kCounrtyList,               "Получить перечень стран" ) << '\n'<< '\n';
  std::cout << paramhelp( kFactweather,               "Получить фактическую погоду (возможно задать перечень стран, станций, типов станций) (выбрать формат json, csv)" ) << '\n';
  std::cout << paramhelp( kCountry,                   "             Код страны, по которой необходимы данные (164, ...)" ) << '\n';
  std::cout << paramhelp( kStantion,                  "             Код станции (27063, ULLI, ...)" ) << '\n';
  std::cout << paramhelp( kStantionType,              "             Тип станции (kSynopFix - синоптические, kAirportRegType - аэропорты, kAerodrome - аэродромы ГМЦ, kSynopMob - мобильные)" ) << '\n';
  std::cout << paramhelp( kDate,                      "             Задать дату, за которую необходимы данные в формате YYYY-MM-DD" ) << '\n';
  std::cout << paramhelp( kHourPeriod,                "             Указать период в часах для получения погоды по станции. Считается от указанной даты -d назад на заданное количество часов" ) << '\n';
  std::cout << paramhelp( kHourParam,                 "             За какой срок (00,01,...,23). Если задан срок - то фактическая погода запрашивается за этот час. По умолчанию запрашивается последняя погода за сутки" ) << '\n';
  std::cout << paramhelp( kFormat,                    "             Задать формат выгрузки данных (json, csv)" ) << '\n';
  std::cout << paramhelp( kFile,                      "             Задать путь и имя файла для сохранения" ) << '\n'<< '\n';

  std::cout << paramhelp( kForecastModel,             "Составить прогноз по точке. По координатам или по станции с заданной заблаговременностью. " ) << '\n';
  std::cout << paramhelp( kForecastStart,             "             На какую дату прогноз (дата начала действия прогноза) в формате YYYY-MM-DDTHH:ii:ss" ) << '\n';
  std::cout << paramhelp( kHourPeriod,                "             Период прогноза в ч. по умолчанию - 24 часа" ) << '\n';
  std::cout << paramhelp( kStantion,                  "             Код станции по которой необходим прогноз (27063, ULLI, ...)" ) << '\n';
  std::cout << paramhelp( kLatLon,                    "             Координаты по которым необходим прогноз в градусах через ,  (широта,долгота)" ) << '\n';
 

}

struct FieldData
{
  QDateTime dt;
  descr_t descr;
  int     hour;
  float   value;
  QString text;
};
uint qHash(const FieldData& key) { return qHash(key.dt.toString() + QString::number(key.descr)); }
bool operator==(const FieldData& fd1, const FieldData& fd2) { return qHash(fd1) == qHash(fd2); }
/**
 * Приводим в соответствие с пуансоном данные метеотабло
 * @resp - ответ от сервиса с прото сообщением
 * @weather_list - переменная в которую будет записан отформатированная метеоинформация
 * @weather_header - заголовок
 */
void formatWeatherData(meteo::surf::DataReply *resp, QStringList &weather_list, QStringList &weather_header)
{
  // check nullptr of resp
  if (!resp) return;
  // if (!weather_list) return;
  // if (!weather_header) return;


  bool ok = false;
  meteo::puanson::proto::Puanson punch = meteo::map::WeatherLoader::instance()->punchparams( "is", &ok );
  if ( false == ok ) {
    warning_log << QObject::tr("Шаблон наноски '%1' не найден").arg("is");
  }
  weather_header.clear();
  weather_header  <<"Дата" << "Название" << "Код станции" << "Срок" << "N" << "C" << "h" 
                  << "Явления" << "Вид-ть" << "dd" << "ff" << "T" << "Td" << "RR" << "P";
  // строка, которую будем заполнять данными и добавим в weather_list
  QStringList weather_line;
  for (int i = 0; i < weather_header.size(); ++i) {
    // обнуляем
    weather_line<<"";
  }

  const uint mDATE      = 0;
  const uint mNAME      = 1;
  const uint mSTANTION  = 2;
  const uint mHOURMIN   = 3;  
  const uint mN         = 4;
  const uint mC         = 5;
  const uint mH         = 6;  
  const uint mWW        = 7;
  const uint mVV        = 8;
  const uint mDD        = 9;
  const uint mFF        = 10;
  const uint mT         = 11;
  const uint mTD        = 12;  
  const uint mRR        = 13;
  const uint mP         = 14;  
  // const uint mCOUNTRY   = 15;
  QStringList descriptors;
  descriptors << "N" << "Nh" << "C" << "Cn" << "CH" << "CM" << "CL" 
              << "h" << "hgr"<< "w" << "w_w_" << "w_tr"<< "V" << "VV"
              << "P" << "T" << "Td" << "RR" << "dd" << "ff";

  QHash<QString,::meteo::puanson::proto::CellRule>  templParams_; //!< метеопараметр и правило его отображения
  meteo::puanson::proto::Puanson tmpl = meteo::map::WeatherLoader::instance()->punchlibraryspecial().value("meteotablo");
  for ( int i=0,isz=tmpl.rule_size(); i<isz; ++i ) {
    QString param = pbtools::toQString(tmpl.rule(i).id().name());
    templParams_[param] = tmpl.rule(i);
  }

  for ( auto data : resp->meteodata_proto() ) {
    
    QString stindex;
    if ( false == data.station_info().has_cccc() ) {
      error_log << QObject::tr("Неизвестная станция");
      continue;
    }
    else {
      stindex = QString::fromStdString(data.station_info().cccc()).remove("'");
      // записываем название станции
      weather_line[mNAME] = QString::fromStdString(data.station_info().name_rus()).remove("'");
      // записываем номер станции
      weather_line[mSTANTION] = stindex;
    }
    
    QDateTime dt = QDateTime::fromString(QString::fromStdString(data.dt()), Qt::ISODate);
    QTime time = dt.time();
    QDate date = dt.date();
    
    weather_line[mHOURMIN] = time.toString("hh:mm");
    weather_line[mDATE] = date.toString("yyyy.MM.dd");

    bool shtihl = false;
    bool hasGust = false;
    QString gustValue;
    for(auto param : data.param()) {
      if ( "ff" == param.descrname() && 0 == param.value() ) {
        shtihl = true;
      }
      if ( "fx" == param.descrname() ) {
        hasGust = true;
        auto code = QString::fromStdString(param.code());
        gustValue = meteo::map::stringFromRuleValue(param.value(), templParams_["fx"], code);
      }
    }

    QList<float> Nhs;
    QStringList N{"N", "Nh"};
    QStringList C{"C", "Cn", "CH", "CM", "CL"};
    QStringList vngo{"h"};
    QStringList ws{"w", "w_w_", "w_tr"};
    QStringList vs{"V", "VV"};

    auto cloudMap = meteo::map::stringForCloud(&data, punch);
    for ( auto descr : descriptors ) {
      
      meteo::surf::MeteoParamProto param;
      for ( auto p : data.param() ) {
        if ( descr == QString::fromStdString(p.descrname())) {
          param = p;
          break;
        }
        else if ( true == ws.contains(descr) && true == ws.contains(QString::fromStdString(p.descrname()))) {
          descr = QString::fromStdString(p.descrname());
          param = p;
          break;
        }
        else if ( true == vs.contains(descr) && true == vs.contains(QString::fromStdString(p.descrname()))) {
          descr = QString::fromStdString(p.descrname());
          param = p;
          break;
        }
        else if ( true == N.contains(descr) && true == N.contains(QString::fromStdString(p.descrname()))) {
          descr = QString::fromStdString(p.descrname());
          param = p;
          break;
        }
        else if ( true == C.contains(descr) && true == C.contains(QString::fromStdString(p.descrname()))) {
          descr = QString::fromStdString(p.descrname());
          param = p;
          break;
        }
        else if ( true == vngo.contains(descr) && true == vngo.contains(QString::fromStdString(p.descrname()))) {
          descr = QString::fromStdString(p.descrname());
          param = p;
          break;
        }
      }
      
      if ( true == templParams_.contains(descr)) {
        if (N.contains(QString::fromStdString(param.descrname())) ) {
          weather_line[mN] = cloudMap["N"];
        }
        else if (C.contains(QString::fromStdString(param.descrname())) ) {
          weather_line[mC] = cloudMap["C"];
        }
        else if (vngo.contains(QString::fromStdString(param.descrname())) ) {
          weather_line[mH]= cloudMap["h"];
        }
        else if ( "ff" == param.descrname() && true == shtihl ) {
          weather_line[mFF] = QObject::tr("штиль");
        }
        else if ( "ff" == param.descrname() && !shtihl  ) {
          auto code = QString::fromStdString(param.code());
          QString windValue = meteo::map::stringFromRuleValue(param.value(), templParams_[descr], code);
          weather_line[mFF] = windValue;
          if ( hasGust ){
           weather_line[mFF] += "/" + gustValue ;
          }
        }
        else if ( "dd" == param.descrname()  ) {
          if ( true == shtihl ){
            weather_line[mDD]= QObject::tr("") ;  
          }else{
            weather_line[mDD]= meteo::map::stringFromRuleValue(param.value(), templParams_[descr], QString::fromStdString(param.code())) ;
          }
          
        } else if ( "P" == param.descrname() ){
          weather_line[mP] = meteo::map::stringFromRuleValue(param.value(), templParams_[descr], QString::fromStdString(param.code()));
        } else if ( "T" == param.descrname() ){
          weather_line[mT] = meteo::map::stringFromRuleValue(param.value(), templParams_[descr], QString::fromStdString(param.code()));
        } else if ( "Td" == param.descrname() ){
          weather_line[mTD] = meteo::map::stringFromRuleValue(param.value(), templParams_[descr], QString::fromStdString(param.code()));
        } else if ( "RR" == param.descrname() ){
          weather_line[mRR] = meteo::map::stringFromRuleValue(param.value(), templParams_[descr], QString::fromStdString(param.code()));
        } else if ( vs.contains( QString::fromStdString(param.descrname()) ) ){
          weather_line[mVV] = meteo::map::stringFromRuleValue(param.value(), templParams_[descr], QString::fromStdString(param.code()));
        } else if ( ws.contains( QString::fromStdString(param.descrname()) ) ){
          weather_line[mWW] = meteo::map::stringFromRuleValue(param.value(), templParams_[descr], QString::fromStdString(param.code()));
          // weather_line[mWW] += QString::fromStdString(param.code());
        }
        
        std::string str;
        param.SerializeToString(&str);
                
        // if ( "w_w_" == param.descrname() ) {
        //   weather_line[mWW]+= QString::fromStdString(param.code());
        // }
        if ( "w_tr" == param.descrname() ) {
          weather_line[mWW] = QString::fromStdString(param.code());
        }
      }
    }

    // сохраняем строку
    weather_list << weather_line.join(";");
    for (int i = 0; i < weather_header.size(); ++i) {
      // обнуляем
      weather_line[i]="";
    }
  }
  return;
}


/**
 * @brief Получаем фактическую погоду в формате метеотабло и сохраняем в файл или выводим на экран
 * 
 * @param country_ код стран, по которым запрашивается погода
 * @param stantiontype_ тип станций, которые запрашиваются
 * @param stantions_ номера станций для запроса
 * @param req_date_ дата за которую запрашивается погода
 * @param filename имя и путь файла для сохранения
 * @param format формат экспорта, csv, json
 * @param query_type тип запроса - либо последнее значение за сутки либо погода за конкретный срок
 * @param period заданный период в часах для получения погоды по станции от заданной даты назад
 */
void getFactWeather( QList<int>* country_, QList<int>* stantiontype_, 
  QStringList* stantions_, QDateTime *req_date_, 
  QString filename="default", QString format="csv", QString query_type="last_value", int period=24 ){

  // если нужна погода по станции
  // если указана одна станция, то запрос меняется на "погода по станции"
  bool single_stantion = false;

  // создаем запрос
  meteo::surf::DataRequest request;

  // заполняем запрос
  request.set_as_proto(true);
  request.set_date_end(req_date_->toString(Qt::ISODate).toStdString());
  request.set_date_start(req_date_->toString(Qt::ISODate).toStdString());
  // # тип запроса
  request.set_query_type(meteo::surf::kTermValue);
  // если запрашиваем последнее актуальное значение, то отступаем назад на сутки, чтобы захватить максимум значений
  if ( query_type=="last_value" ){
    // # тип запроса
    request.set_query_type(meteo::surf::kLastValue);
    QDateTime req_date_end_(*req_date_);
    req_date_end_ = req_date_end_.addSecs( -period*60*60 );
    request.set_date_start(req_date_end_.toString(Qt::ISODate).toStdString());
  }
  //
  // что запрашиваем
  //

  // обязательные параметры
  request.set_level_p(0);
  request.set_type_level(1);  
  
  // # если есть поиск по станциям - добавляем их в запрос
  if (stantions_ != nullptr){
    for (int i = 0; i < stantions_->count(); i++){
      request.add_station(stantions_->at(i).toStdString());
    }
    // если у нас задана только одна станция, то запрос по этой станции и тип запроса тоже меняется
    if ( stantions_->count()==1 ){
      single_stantion = true;
      // устанавливаем принудительно тип запроса
      request.set_query_type(meteo::surf::kLastValue);
      request.set_date_start(req_date_->addSecs( -period*60*60 ).toString(Qt::ISODate).toStdString());
    }
  }

  // # добавляем типы станций
  if (stantiontype_ != nullptr){
    for (int i = 0; i < stantiontype_->count(); i++){
      request.add_type(stantiontype_->at(i));
    }
  }
  // если типы станций отсуствуют, то задаем синоптические
  if ( stantiontype_==nullptr || stantiontype_->count()==0 ){
    request.add_type(meteo::surf::kSynopFix);  
    if ( single_stantion ){
      request.add_type(meteo::surf::kAirportRegType);
      request.add_type(meteo::surf::kAerodrome);
    }
  } 

  // # страны
  // страны  не запрашиваются если задана одна станция.
  // в этом случае работет "погода по станции"
  if (country_ != nullptr && !single_stantion){
    for (int i = 0; i < country_->count(); i++){
      request.set_country(country_->at(i));
    }
  }

  // request.add_type(meteo::surf::kAirportRegType);
  // request.add_type(meteo::surf::kAerodrome);

  request.add_meteo_descrname("T");
  request.add_meteo_descrname("Td");
  request.add_meteo_descrname("P");
  request.add_meteo_descrname("RR");
  request.add_meteo_descrname("C");
  request.add_meteo_descrname("N");
  request.add_meteo_descrname("Nh");
  request.add_meteo_descrname("U");
  request.add_meteo_descrname("ff");
  request.add_meteo_descrname("dd");
  request.add_meteo_descrname("w");
  request.add_meteo_descrname("w_w_");
  request.add_meteo_descrname("w_tr");
  request.add_meteo_descrname("H");
  request.add_meteo_descrname("h");
  request.add_meteo_descrname("VV");
  request.add_meteo_descrname("V");
  request.add_meteo_descrname("hgr");
  request.add_meteo_descrname("CL");
  request.add_meteo_descrname("CM");
  request.add_meteo_descrname("CH");
  request.add_meteo_descrname("Cn");
  request.add_meteo_descrname("name");

  meteo::rpc::Channel* ctrl_ = meteo::global::serviceChannel( meteo::settings::proto::kSrcData );
  
  if ( 0 == ctrl_ ) {
    error_log << QObject::tr("Не удалось подключиться к сервису данных.");
    delete ctrl_;
    return;
  }
  
  
  debug_log<<QObject::tr("Запрос данных...");
  debug_log<<request.date_start()<<" ... "<<request.date_end();
  // debug_log << request.Utf8DebugString();//GetMeteoDataByDay
  meteo::surf::DataReply * reply = nullptr;
  if ( single_stantion ){
    reply =  ctrl_->remoteCall( &meteo::surf::SurfaceService::GetMeteoDataOnStation, request,  30000000, true);
  }else{
    reply = ctrl_->remoteCall( &meteo::surf::SurfaceService::GetMeteoDataByDay, request,  30000000, true);
  }
  
  if ( nullptr == reply ) {
    error_log << QObject::tr("При попытке получить фактическую погоду ответ от сервиса данных не получен");
    delete reply;
    ctrl_->disconnect();
    delete ctrl_;
    return;
  }

  ctrl_->disconnect();
  delete ctrl_;

  if (format=="json"){
    std::string json_string;
    google::protobuf::util::MessageToJsonString(*reply,&json_string);
    if ( filename=="default" ){
      std::cout<<json_string;  
    }else{
      QFile file(filename);
      if (file.open(QIODevice::ReadWrite | QIODevice::Text)) {
          QTextStream stream(&file);
          stream << QString::fromStdString(json_string) << '\n';
      }
    }    
  }else if (format=="csv"){
    if ( filename=="default" ){
      filename+="."+format;
    }
    QFile file(filename);
    if (file.open(QIODevice::ReadWrite | QFile::Truncate)) {
        QTextStream stream(&file);
        QStringList *weather_list_ = new QStringList();
        QStringList *weather_header_ = new QStringList();
        formatWeatherData(reply,*weather_list_,*weather_header_ );
        stream << weather_header_->join(";") << '\n';
        for (int i = 0; i < weather_list_->count(); i++) {
          //write to stream joined params
          stream << weather_list_->at(i) << '\n';
        }
        delete weather_list_;
        delete weather_header_;
    }
  }else {
    QStringList *weather_list_ = new QStringList();
    QStringList *weather_header_ = new QStringList();
    formatWeatherData(reply,*weather_list_,*weather_header_ );
    std::cout<<'\n';
    std::cout<< QString("|%1 [%2]|%3|%4|%5|%6|%7|%8|%9|%10|%11|%12|%13|")
                  .arg( weather_header_->at(1),-25 )
                  .arg( weather_header_->at(2),11 )
                  .arg( weather_header_->at(3),5 )
                  .arg( weather_header_->at(4),7 )
                  .arg( weather_header_->at(5),9 )
                  .arg( weather_header_->at(6),11 )
                  .arg( weather_header_->at(7),6 )
                  .arg( weather_header_->at(8),5 )
                  .arg( weather_header_->at(9),5 )
                  .arg( weather_header_->at(10),5 )
                  .arg( weather_header_->at(11),5 )
                  .arg( weather_header_->at(12),5 )
                  .arg( weather_header_->at(14),6 ).toStdString() << '\n' ;
    for (int i = 0; i < weather_list_->count(); i++) {
      //write to stream joined params
      QStringList w_;
      w_ = weather_list_->at(i).split(";");
      std::cout<< QString("|%1 [%2]|%3|%4|%5|%6|%7|%8|%9|%10|%11|%12|%13|")
                  .arg( w_.at(1),-25 )
                  .arg( w_.at(2),11 )
                  .arg( w_.at(3),5 )
                  .arg( w_.at(4),7 )
                  .arg( w_.at(5),9 )
                  .arg( w_.at(6),11 )
                  .arg( w_.at(7),6 )
                  .arg( w_.at(8),5 )
                  .arg( w_.at(9),5 )
                  .arg( w_.at(10),5 )
                  .arg( w_.at(11),5 )
                  .arg( w_.at(12),5 )
                  .arg( w_.at(14),6 ).toStdString() << '\n' ;
    }
    delete weather_list_;
    delete weather_header_;
  }
  
  // debug_log << reply->Utf8DebugString();
  
  delete reply;
}


/**
 * @brief Get the Forecast Weather object
 * 
 * @param stantions_ указатель на объект класса QStringList, содержащий список станций, для которых необходимо получить прогноз погоды;
 * @param forec_date_  указатель на объект класса QDateTime, содержащий дату и время, на которые необходимо получить прогноз погоды;
 * @param latlon - указатель на объект класса QList<QPair<float, float>>, содержащий координаты широты и долготы для каждой из станций;
 * @param filename_ строка, содержащая название файла, в который будет записан полученный прогноз погоды. Значение по умолчанию - "default";
 * @param period_ period_ - целочисленное значение, указывающее на период времени, на который необходимо получить прогноз погоды (в часах). Значение по умолчанию - 24.
 */
void getForecastWeather( QStringList* stantions_,  QDateTime *forec_date_, QList<QPair<float,float>>* latlon, QString filename_="default", int period_=24 ){
  // формируем запрос прогностических полей по параметрам
  Q_UNUSED(stantions_);
  Q_UNUSED(forec_date_);
  Q_UNUSED(filename_);
  Q_UNUSED(period_);
  Q_UNUSED(latlon);
  
  // окончание прогноза
  QDateTime end = forec_date_->addSecs(period_ * 3600);

  meteo::field::DataRequest req;
  
  req.add_meteo_descr(TMeteoDescriptor::instance()->descriptor("T"));
  req.add_meteo_descr(TMeteoDescriptor::instance()->descriptor("Td"));
  req.add_meteo_descr(TMeteoDescriptor::instance()->descriptor("P"));
  req.add_meteo_descr(TMeteoDescriptor::instance()->descriptor("U"));
  req.add_meteo_descr(TMeteoDescriptor::instance()->descriptor("dd"));
  req.add_meteo_descr(TMeteoDescriptor::instance()->descriptor("ff"));
  req.add_meteo_descr(TMeteoDescriptor::instance()->descriptor("N"));
  req.add_meteo_descr(TMeteoDescriptor::instance()->descriptor("C"));
  req.add_meteo_descr(TMeteoDescriptor::instance()->descriptor("V"));
  req.add_meteo_descr(TMeteoDescriptor::instance()->descriptor("h"));
  req.add_meteo_descr(TMeteoDescriptor::instance()->descriptor("w"));

  req.set_forecast_start(pbtools::toString(forec_date_->toString(Qt::ISODate)));
  req.set_forecast_end(pbtools::toString(end.toString(Qt::ISODate)));

  // добавляем все уровни для разных моделей
  req.add_level(0);
  req.add_level(2);
  req.add_level(10);
  req.add_type_level(1);
  req.add_type_level(101);
  req.add_type_level(103);
  req.add_type_level(200);

  req.set_need_field_descr(true);
  
  for (int i = 0; i < latlon->size(); i++) {
    float lat = latlon->at(i).first;
    float lon = latlon->at(i).second;
    // cout << "Lat: " << lat << ", Lon: " << lon << endl;
    meteo::surf::Point* p = req.add_coords();
    p->set_fi(MnMath::deg2rad(lat));
    p->set_la(MnMath::deg2rad(lon));
  }  

  meteo::rpc::Channel* channel = meteo::global::serviceChannel(meteo::settings::proto::kField);
  if ( nullptr == channel ) {
    error_msg << QObject::tr("Ошибка: отсутствует подключение к сервису данных.");
  }
  // connect to GetForecastValues forecast service
  meteo::field::ValueDataReply* resp = channel->remoteCall(&meteo::field::FieldService::GetForecastValues, req, 180000);
  channel->disconnect();
  delete channel;

  if ( nullptr == resp ) {
    error_msg << QObject::tr("Ошибка: превышено время ожидания ответа от сервиса.");
    return;
  }
  debug_log << "Данные получены, всего: "<<resp->data_size();
  // QList<TMeteoData> forecastList = toMeteoData(*resp);
  // исключаем дубликаты параметров за один срок
  QSet<FieldData> filtered;
  for ( int i=0,isz=resp->data_size(); i<isz; ++i ) {
    const meteo::field::OnePointData& d = resp->data(i);
    QString dts = pbtools::toQString(d.fdesc().date()).replace("T", " ");
    QDateTime dt = QDateTime::fromString(dts, Qt::ISODate);
    dt = dt.addSecs(d.fdesc().hour());

    FieldData data;
    data.descr = d.fdesc().meteodescr();
    data.dt = dt;
    data.hour = d.fdesc().hour();
    data.text = pbtools::toQString(d.text());
    data.value = d.value();

    QSet<FieldData>::const_iterator it = filtered.constFind(data);
    if ( it == filtered.constEnd() || d.fdesc().hour() < it->hour  ) {
      filtered.insert(data);
    }
  }

  QHash<QString,TMeteoData> mdHash;

  foreach ( const FieldData& data, filtered ) {
    QString dts = data.dt.toString();

    if ( !mdHash.contains(dts) ) {
      TMeteoData md;
      md.setDateTime(data.dt);
      md.add("h0", TMeteoParam("", 0, control::RIGHT)); // special case: для наноски пуансонов
      mdHash.insert(dts, md);
    }

    TMeteoParam param(data.text, data.value, control::RIGHT);
    debug_log << data.dt << data.text << data.value;
    mdHash[dts].add(data.descr, param);
  }

  // debug_log<<mdHash.values();

  delete resp;
}

/**
 * @brief Получаем список стран
 * 
 */
void getCountryList(){
  meteo::sprinf::CountryRequest countriesRequest;
  
  meteo::rpc::Channel* ctrl_ = meteo::global::serviceChannel( meteo::settings::proto::kSprinf );
  
  if ( 0 == ctrl_ ) {
    error_log << QObject::tr("Не удалось подключиться к справочной информации.");
    delete ctrl_;
    return;
  }

  meteo::sprinf::Countries* countries_;
  countries_ = ctrl_->remoteCall( &meteo::sprinf::SprinfService::GetCountry, countriesRequest, 300000 );
  if ( nullptr == countries_ ) {    
    error_log << QObject::tr("возращено значение nullptr.");
    delete ctrl_;
    delete countries_;
    return;
  } else if ( true == countries_->has_result() && false == countries_->result() ) {
    error_log << QObject::tr("Не удалось получить список стран. Пустой ответ от сервиса.");
    delete ctrl_;
    delete countries_;
    return;
  }

  std::cout<<'\n';
  // message Country {
  //   optional int32        number          = 1;
  //   optional string       code            = 2;
  //   repeated string       icao            = 3;
  //   optional string       name            = 4;
  //   optional string       name_en         = 5;
  //   optional string       name_ru         = 6;
  //   optional string       wmocode         = 7;
  //   optional int32        wmonumber       = 8;
  // }
  std::cout<< QString("|%1|%2|%3|")
                  .arg( "ID",-5 )
                  .arg( "Название",-41 )
                  .arg( "Код страны",11 ).toStdString() << '\n' ;
  for (int i = 0; i < countries_->country_size(); i++) {
    std::cout<< QString("|%1|%2|%3|")
                .arg( countries_->country(i).number(),-5 )
                .arg( QString::fromStdString(countries_->country(i).name_ru()) ,-41 )
                .arg( QString::fromStdString(countries_->country(i).code()) ,11 ).toStdString() << '\n' ;    
  }

  return;
}

/**
 * Главная функция
 * @param  argc [description]
 * @param  argv [description]
 * @return      [description]
 */
int main( int argc, char* argv[] ) {
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

  QString format="csv";
  // проверяем наличие параметров
  if ( true == options->installed(kFormat) ) {
    format = options->at(kFormat).value();
  }

  QDateTime dt = QDateTime::currentDateTimeUtc();
  // берем текущий час
  QString kHour = dt.toString("hh") ;
  // по умолчанию тип запроса - последнее значение
  // если указан час - значит это точный запрос за конкретный срок
  QString query_type = "last_value";
  if (options->at(kHourParam).value() !=QString()){
    kHour = options->at(kHourParam).value();
    query_type = "term_value";
  }
  
  int hour = kHour.toInt();
  dt.setTime( QTime( hour,0,0,0) );
  QString date = dt.toString(Qt::ISODate);
  if ( options->at(kDate).value() != QString() ) {
    dt.setDate( QDate::fromString(options->at(kDate).value(), "yyyy-MM-dd") );
  }

  // период для получения погоды по станции (например)
  // указывается в часах и отступает от заданной даты назад на указанное количество часов
  int period = 24;
  if (options->at(kHourPeriod).value() !=QString()){
    period = options->at(kHourPeriod).value().toInt();
  }
  // список стран
  QList<int> country = QList<int>();
  if (options->at(kCountry).value() !=QString()){
    country << options->at(kCountry).value().toInt();
  }
  // список станций
  QStringList stantion = QStringList();
  if (options->at(kStantion).value() !=QString()){
    stantion << options->at(kStantion).value();
  }
  // координаты точек
  QList< QPair<float,float> > latlon;
  if (options->at(kLatLon).values().count()>0){
    QStringList latlon_str = options->at(kLatLon).values();
    if ( options->at(kLatLon).values().count()==2 ){
      // latitude longitude
      latlon << QPair<float,float>(latlon_str[0].toFloat(), latlon_str[1].toFloat());
    }else{
      // error index out of range
      qDebug() << "Error: LatLon option has wrong format";
    }
  }
  // типы станций
  QList<int> stantiontype = QList<int>();
  if (options->at(kStantionType).value() !=QString()){
    int sttype_ = 1;
    if ( options->at(kStantionType).value()=="kSynopFix" ){
      sttype_ = meteo::surf::kSynopFix;
    }else if ( options->at(kStantionType).value()=="kAirportRegType" ){
      sttype_ = meteo::surf::kAirportRegType;
    }else if ( options->at(kStantionType).value()=="kAerodrome" ){
      sttype_ = meteo::surf::kAerodrome;
    }else if ( options->at(kStantionType).value()=="kSynopMob" ){
      sttype_ = meteo::surf::kSynopMob;
    }
    stantiontype << sttype_;
  }

  
  //
  // Получение фактической погоды
  //
  if ( true == options->installed(kFactweather) ) {
    QString format = options->at(kFormat).value();
    QString filename = options->at(kFile).value();
    if (filename.isEmpty()){
      filename="default";
    }
    getFactWeather(&country,&stantiontype,&stantion,&dt,filename,format,query_type,period);
    delete app;
    return EXIT_SUCCESS;
  }


  //
  // Получение прогнозов по координатам
  // 
  if ( true == options->installed(kForecastModel) ) {
    QString filename = options->at(kFile).value();
    getForecastWeather(&stantion,&dt,&latlon,filename,period);
    // kLatLon
  }


  // 
  // Получение списка стран
  // 
  if ( true == options->installed(kCounrtyList) ){
    getCountryList();
    delete app;
    return EXIT_SUCCESS;
  }
  
  delete app;
  return EXIT_SUCCESS;
}



