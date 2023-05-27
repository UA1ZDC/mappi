#include <boost/python.hpp>

#include <qbytearray.h>
#include <cross-commons/app/paths.h>
#include <cross-commons/debug/tlog.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/settings/settings.h>
#include <qcoreapplication.h>

#include <meteo/commons/proto/map_radar.pb.h>

//#include </usr/include/python3.9/memoryobject.h>

bool init() {
  static bool initDone = false;
  if ( false == initDone ){
    TAPPLICATION_NAME("meteo");
    ::meteo::gSettings(new meteo::Settings);
    if ( false == meteo::gSettings()->load() ){
      return false;
    }    
    initDone = true;
  }

  return initDone;
}

std::string serviceAddress( int code)
{
  if ( false == init() ){
    return "EXIT_FAILURE";
  }

  return  meteo::global::serviceAddress(static_cast<meteo::settings::proto::ServiceCode>(code))
                        .toStdString();
}

std::string serviceAddressDiagnostic()
{
  if ( false == init() ){
    return "EXIT_FAILURE";
  }
  auto serviceCode = meteo::settings::proto::ServiceCode::kDiagnostic;
  return meteo::global::serviceAddress(serviceCode).toStdString();
}

std::string serviceAddressInterTaskService()
{
  if ( false == init() ) {
    return "EXIT_FAILURE";
  }
  auto serviceCode = meteo::settings::proto::ServiceCode::kInterTaskService;
  return meteo::global::serviceAddress(serviceCode).toStdString();
}


std::string mongoQuery(const boost::python::object& oidPythonObject)
{
  if ( false == init() ){
    return std::string();
  }

  const char* queryName = boost::python::extract<const char*>( boost::python::str(oidPythonObject).encode("utf-8"));
  if ( false == meteo::global::kMongoQueriesNew.contains(queryName)){
    return std::string();
  }
  return meteo::global::kMongoQueriesNew[queryName].toStdString();
}

std::string confInterHost()
{
  if ( false == init() ){
    return std::string();
  }

  auto conf = meteo::global::mongodbConfInter();  
  return conf.host().toStdString();
}


ushort confInterPort()
{
  if ( false == init() ){
    return 0;
  }

  auto conf = meteo::global::mongodbConfInter();
  return conf.port();
}

boost::python::object dbDjango()
{
  if ( false == init() ){
    return boost::python::object();
  }
  auto c = ::meteo::gSettings()->dbConn(meteo::global::dbnames::dbDjango);

  boost::python::object mode = boost::python::import("dbsettings_pb2");
  boost::python::object proto = mode.attr("DbConnection")(); //Вызов конструктора объекта PYTHON
  proto.attr("conn_name") = c.conn_name();
  proto.attr("human_name") = c.human_name();
  proto.attr("name") = c.name();
  proto.attr("host") = c.host();
  proto.attr("port") = c.port(); //TODO проверить корректность работы python-модуля после string -> uint16 
  proto.attr("login") = c.login();
  proto.attr("password") = c.password();
  return proto;
}

boost::python::object meteoparam_props( uint64_t descr )
{
  if ( false == init() ){
    return boost::python::dict();
  }
  boost::python::dict props;
  QMap< QString, QString > props_map = meteo::global::meteoparamProps(descr);

  props["name"] = props_map["name"].toStdString();
  props["descriptor"] = props_map["descriptor"].toStdString();
  props["units"] = props_map["units"].toStdString();
  props["unitsRu"] = props_map["unitsRu"].toStdString();
  props["description"] = props_map["description"].toStdString();
  return props;
}

boost::python::object meteoparam_props_by_name( const char* descrname )
{
  if ( false == init() ){
    return boost::python::dict();
  }
  boost::python::dict props;
  QMap< QString, QString > props_map = meteo::global::meteoparamPropsByName(descrname);

  props["name"] = props_map["name"].toStdString();
  props["descriptor"] = props_map["descriptor"].toStdString();
  props["units"] = props_map["units"].toStdString();
  props["unitsRu"] = props_map["unitsRu"].toStdString();
  props["description"] = props_map["description"].toStdString();
  return props;
}

boost::python::object meteoparam_value_descriptions( int descriptor )
{
  if ( false == init() ){
    return boost::python::dict();
  }
  QString strdescr = QString::number(descriptor);

  int argc = 0;
  char* argv[1];
  
  QCoreApplication app( argc, argv);
  Q_UNUSED(app);
  meteo::global::loadBufrTables( QStringList() << strdescr );
  auto descriptions = meteo::global::bufrTable(strdescr);
  boost::python::dict props;
  for ( auto it = descriptions.begin(), end = descriptions.end(); it != end; ++it ) {
    props[ QString::number( it.key() ).toStdString() ] = it.value().toStdString();
  }
  boost::python::dict pydescription;
  pydescription["descriptor"] = strdescr.toStdString();
  pydescription["values"] = props;
  return pydescription;
}

float pressureOnStationLevel(float h, float P, float T) {
  return meteo::global::pressureOnStationLevel( h, P, T );
}

float pressureOnOceanLevel(float h, float P, float T) {
  return meteo::global::pressureOnOceanLevel( h, P, T );
}

/**
 * @brief Конвертируем char в объект питона
 * 
 * @param data 
 * @param size 
 * @return boost::python::object 
 */
boost::python::object byte_to_object(char *data, int size) {
  boost::python::object memoryView(boost::python::handle<>(PyMemoryView_FromMemory(data, size, PyBUF_READ)));
  return memoryView;
}

/**
 * @brief Загрузка настроек сохранения 
 * 
 * @return boost::python::object 
 */
boost::python::object loadMLSettings(){
  meteo::cron::Settings settings;
  std::string result;

  settings = meteo::global::loadMLSettings();  
  settings.SerializeToString(&result);

  // надо копировать в новую переменную, т.к. старый очищается
  char* copied_result = new char[result.size()];
  memcpy( copied_result, result.data(), result.size() );

  return byte_to_object( copied_result, result.size() );
  // return result;
}

/**
 * @brief Сохранение настроек восстановления полей 
 * 
 * @param settings 
 * @return true 
 * @return false 
 */
bool saveMLSettings(std::string st_object) {
  meteo::cron::Settings settings;
  if (settings.ParseFromString(st_object) ){
    meteo::global::saveMLSettings(settings);
    return true;
  }
  return false;
}

/**
 * @brief Загрузка климатических станций из файла настроек
 * 
 * @return boost::python::object 
 */
boost::python::object loadClimatStations(){
  meteo::climat::ClimatSaveProp stations;
  std::string result;

  stations = meteo::global::loadClimatStations();  
  stations.SerializeToString(&result);

  // надо копировать в новую переменную, т.к. старый очищается
  char* copied_result = new char[result.size()];
  memcpy( copied_result, result.data(), result.size() );

  return byte_to_object( copied_result, result.size() );
}

/**
 * @brief Сохранение климатических станций в файл настроек
 * 
 * @param stations 
 * @return true 
 * @return false 
 */
bool saveClimatStations(std::string st_object) {
  meteo::climat::ClimatSaveProp stations_proto;
  if (stations_proto.ParseFromString(st_object) ){
    meteo::global::saveClimatStations(stations_proto);
    return true;
  }
  return false;
}

boost::python::object radarColors() {
  init();

  meteo::map::proto::RadarColors colors = meteo::global::radarcolors();
  std::string str;
  colors.SerializePartialToString(&str);

  char* copied_result = new char[str.size()];
  memcpy( copied_result, str.data(), str.size() );

  return byte_to_object( copied_result, str.size() );
}

boost::python::object isoColors() {
  init();

  meteo::map::proto::FieldColors colors = meteo::global::isocolors();
  std::string str;
  colors.SerializePartialToString(&str);

  char* copied_result = new char[str.size()];
  memcpy( copied_result, str.data(), str.size() );

  return byte_to_object( copied_result, str.size() );
}

// возращаем рабочие пути
std::string etcPath(){
  return MnCommon::etcPath().toStdString();
}
// возращаем рабочие пути
std::string sharePath(){
  return MnCommon::sharePath().toStdString();
}
// возращаем рабочие пути
std::string libPath(){
  return MnCommon::libPath().toStdString();
}
// возращаем рабочие пути
std::string binPath(){
  return MnCommon::binPath().toStdString();
}


BOOST_PYTHON_MODULE(libglobalwrap)
{
    boost::python::def( "serviceAddress", serviceAddress, boost::python::args("ok") );
    boost::python::def( "serviceAddressDiagnostic", serviceAddressDiagnostic);
    boost::python::def( "serviceAddressInterTaskService", serviceAddressInterTaskService);    
    boost::python::def( "confInterHost", confInterHost );
    boost::python::def( "confInterPort", confInterPort );    
    boost::python::def( "mongoQuery", mongoQuery );
    boost::python::def( "pressureOnStationLevel", pressureOnStationLevel );
    boost::python::def( "pressureOnOceanLevel", pressureOnOceanLevel );


    boost::python::def( "dbDjango", dbDjango );
    boost::python::def( "meteoparam_props", meteoparam_props );
    boost::python::def( "meteoparam_props_by_name", meteoparam_props_by_name );
    boost::python::def( "meteoparam_value_descriptions", meteoparam_value_descriptions );
    boost::python::def( "loadClimatStations", loadClimatStations );
    boost::python::def( "saveClimatStations", saveClimatStations );
    boost::python::def( "loadMLSettings", loadMLSettings );
    boost::python::def( "saveMLSettings", saveMLSettings );
    
    boost::python::def( "etcPath", etcPath );
    boost::python::def( "sharePath", sharePath );
    boost::python::def( "libPath", libPath );
    boost::python::def( "binPath", binPath );

    boost::python::def( "radarColors", radarColors );
    boost::python::def( "isoColors", isoColors );
}
