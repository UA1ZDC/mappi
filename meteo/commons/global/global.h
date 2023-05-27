#ifndef METEO_COMMONS_GLOBAL_GLOBAL_H
#define METEO_COMMONS_GLOBAL_GLOBAL_H

#include <qdir.h>
#include <qstringlist.h>

#include <cross-commons/singleton/tsingleton.h>
#include <cross-commons/debug/tmap.h>
#include <commons/geobasis/geopoint.h>

#include <sql/dbi/dbi.h>
#include <sql/nosql/document.h>
#include <sql/psql/connectiondict.h>

#include <commons/proto/mongo.pb.h>
#include <meteo/commons/proto/climat.pb.h>
#include <meteo/commons/proto/cron.pb.h>
#include <meteo/commons/proto/locsettings.pb.h>
#include <meteo/commons/proto/map_document.pb.h>
#include <meteo/commons/proto/surface_service.pb.h>
#include <meteo/commons/proto/services.pb.h>
#include <meteo/commons/proto/weather.pb.h>
#include <meteo/commons/proto/map_radar.pb.h>

#include <commons/meteo_data/meteo_data.h>
#include <meteo/commons/global/log.h>
#include <meteo/commons/rpc/rpc.h>
#include <cross-commons/debug/tlog.h>

#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

class TMeteoData;

namespace meteo {

namespace puanson {
namespace proto {
class Puanson;
}
}

namespace global {

void logoutToDb(tlog::Priority priority, const QString& facility, const QString& fileName, int line, const QString& m);

namespace dbnames {
static const auto dbDjango = QObject::tr("django");
}


static const QString kGridLoaderCommon("grid.common");
static const QString kStationLoaderCommon("station.common");
static const QString kCitiesLoaderCommon("cities.common");

static const QString kMapTemporaryDir( QDir::homePath() + QString("/.meteo/map") );
static const QString kMapColorSettingsFilename( QDir::homePath() + QString("/.meteo/map/colors") );
static const QString kMapGridSettingsFilename( QDir::homePath() + QString("/.meteo/map/grid") );
static const QString kShapeMapColorSettingsFilename( QDir::homePath() + QString("/.meteo/map/colors.shp") );
//static const QString kLastCutParamsFilename( QDir::homePath() + QString("/.meteo/map/lastcut") );
static const QString kStationSynopCacheFile( QDir::homePath() + QString("/.meteo/map/station.synop.cache") );
static const QString kStationAeroCacheFile( QDir::homePath() + QString("/.meteo/map/station.aero.cache") );
static const QString kStationAirportCacheFile( QDir::homePath() + QString("/.meteo/map/station.airport.cache") );
static const QString kStationAerodromeCacheFile( QDir::homePath() + QString("/.meteo/map/station.aerodrome.cache") );

static const QString kCustomStationsListFilename( QDir::homePath() + QString("/.meteo/map/stations/stations_list") );
static const QString kForecastMethodsDir( MnCommon::varPath("meteo") + QString("/forecast/scripts/") );
static const QString kForecastMethodsValidDir( MnCommon::varPath("meteo") + QString("/forecast/methods_list/") );

static const QString kLastNabludeniaTypes( QDir::homePath() + QString("/.meteo/map/nabludenia_types") );
static const QString kLastRadarDescriptors( QDir::homePath() + QString("/.meteo/map/radar_descriptors") );
static const QString kLastDocParamsFileNameNach( "lastdoc" );
static const QString kLastDocParamsPath( QDir::homePath() + QString("/.meteo/map/") );
static const QString kDocumentViewerConfigPath ( QDir::homePath() + QString("/.meteo/documentviewer/")  );

static const QString kVoLayerFileName( MnCommon::sharePath("meteo") + QString("/geo/vo.map_ptkpp") );

static const QString kRFBorderFileName( MnCommon::sharePath("meteo") + QString("/geo/rf.map_ptkpp") );

static const QString kForecastWidgetFileName( MnCommon::sharePath("meteo") + QString("/forecastwidget/forecast.json") );
static const QString kForecastWidgetFileNameOrnitolog( MnCommon::sharePath("meteo") + QString("/forecastwidget/ornitolog.json") );

static const QByteArray kBeginPtkppDocument("BEGIN_PTKPP_DOCUMENT");
static const QByteArray kFinishPtkppDocument("FINISH_PTKPP_DOCUMENT");

static const QString kFaxTiffPath ( MnCommon::varPath("meteo") + QString("faxes"));

QMap< QString, QString > loadMongoFuncsNew();
static const QMap< QString, QString > kMongoFuncsNew = loadMongoFuncsNew();

static const QByteArray kMapPtkppPostfix("_M");
static const QByteArray kCutPtkppPostfix("_C");
static const QByteArray kAeroPtkppPostfix("_A");
static const QByteArray kOceanPtkppPostfix("_O");
static const QByteArray kMgPtkppPostfix("_G");
static const QByteArray kFormalPtkppPostfix("_F");

static const QString kCitySettingsFileName( MnCommon::sharePath("meteo") + "/geo/city.pbf");
static const QString kDisplaySettingsFileName( MnCommon::sharePath("meteo") + "/geo/displaysettings.pbf");

static const TMap<QString, QString> kTranslit = TMap<QString, QString>()
   << QPair<QString, QString>(QObject::tr("А"), "A")  << QPair<QString, QString>(QObject::tr("Б"), "B")
   << QPair<QString, QString>(QObject::tr("В"), "V")  << QPair<QString, QString>(QObject::tr("Г"), "G")
   << QPair<QString, QString>(QObject::tr("Д"), "D")  << QPair<QString, QString>(QObject::tr("Е"), "E")
   << QPair<QString, QString>(QObject::tr("Ё"), "YO") << QPair<QString, QString>(QObject::tr("Ж"), "ZH")
   << QPair<QString, QString>(QObject::tr("З"), "Z")  << QPair<QString, QString>(QObject::tr("И"), "I")
   << QPair<QString, QString>(QObject::tr("Й"), "J")  << QPair<QString, QString>(QObject::tr("К"), "K")
   << QPair<QString, QString>(QObject::tr("Л"), "L")  << QPair<QString, QString>(QObject::tr("М"), "M")
   << QPair<QString, QString>(QObject::tr("Н"), "N")  << QPair<QString, QString>(QObject::tr("О"), "O")
   << QPair<QString, QString>(QObject::tr("П"), "P")  << QPair<QString, QString>(QObject::tr("Р"), "R")
   << QPair<QString, QString>(QObject::tr("С"), "S")  << QPair<QString, QString>(QObject::tr("Т"), "T")
   << QPair<QString, QString>(QObject::tr("У"), "U")  << QPair<QString, QString>(QObject::tr("Ф"), "F")
   << QPair<QString, QString>(QObject::tr("Х"), "H")  << QPair<QString, QString>(QObject::tr("Ц"), "C")
   << QPair<QString, QString>(QObject::tr("Ч"), "CH") << QPair<QString, QString>(QObject::tr("Ш"), "SH")
   << QPair<QString, QString>(QObject::tr("Щ"), "SHCH")  << QPair<QString, QString>(QObject::tr("Ъ"), "")
   << QPair<QString, QString>(QObject::tr("Ы"), "y")  << QPair<QString, QString>(QObject::tr("Ь"), "")
   << QPair<QString, QString>(QObject::tr("Э"), "eh") << QPair<QString, QString>(QObject::tr("Ю"), "yu")
   << QPair<QString, QString>(QObject::tr("Я"), "ya");


//нет Ъ и Ё, трансилит ГМС телеграмм
static const TMap<uint8_t, QString> kTranslitKoi7 = TMap<uint8_t, QString>()
   << QPair<uint8_t, QString>(0x61, QObject::tr("А")) << QPair<uint8_t, QString>(0x62, QObject::tr("Б"))
   << QPair<uint8_t, QString>(0x77, QObject::tr("В")) << QPair<uint8_t, QString>(0x67, QObject::tr("Г"))
   << QPair<uint8_t, QString>(0x64, QObject::tr("Д")) << QPair<uint8_t, QString>(0x65, QObject::tr("Е"))
   << QPair<uint8_t, QString>(0x76, QObject::tr("Ж"))
   << QPair<uint8_t, QString>(0x7a, QObject::tr("З")) << QPair<uint8_t, QString>(0x69, QObject::tr("И"))
   << QPair<uint8_t, QString>(0x6a, QObject::tr("Й")) << QPair<uint8_t, QString>(0x6b, QObject::tr("К"))
   << QPair<uint8_t, QString>(0x6c, QObject::tr("Л")) << QPair<uint8_t, QString>(0x6d, QObject::tr("М"))
   << QPair<uint8_t, QString>(0x6e, QObject::tr("Н")) << QPair<uint8_t, QString>(0x6f, QObject::tr("О"))
   << QPair<uint8_t, QString>(0x70, QObject::tr("П")) << QPair<uint8_t, QString>(0x72, QObject::tr("Р"))
   << QPair<uint8_t, QString>(0x73, QObject::tr("С")) << QPair<uint8_t, QString>(0x74, QObject::tr("Т"))
   << QPair<uint8_t, QString>(0x75, QObject::tr("У")) << QPair<uint8_t, QString>(0x66, QObject::tr("Ф"))
   << QPair<uint8_t, QString>(0x68, QObject::tr("Х")) << QPair<uint8_t, QString>(0x63, QObject::tr("Ц"))
   << QPair<uint8_t, QString>(0x7e, QObject::tr("Ч")) << QPair<uint8_t, QString>(0x7b, QObject::tr("Ш"))
   << QPair<uint8_t, QString>(0x7d, QObject::tr("Щ"))
   << QPair<uint8_t, QString>(0x79, QObject::tr("Ы")) << QPair<uint8_t, QString>(0x78, QObject::tr("Ь"))
   << QPair<uint8_t, QString>(0x7c, QObject::tr("Э")) << QPair<uint8_t, QString>(0x60, QObject::tr("Ю"))
   << QPair<uint8_t, QString>(0x71, QObject::tr("Я"));

static const int kCenterWeather(250);
static const int kModelSurface(250);
static const int kModelAero(250);

static const QString kCenterWeatherName( QObject::tr("Наблюдения за погодой") );

QMap< int, QPair<QString, int> > kMeteoCenters();
QMap< int, QString > kLevelTypes();
QMap< int, QStringList> kStandardLevels();
meteo::sprinf::GmiTypes kGmiTypes();


QString lastDocParamsFileNamePostfix( meteo::map::proto::DocumentType type );
QString getDocumentFormatExt( meteo::map::proto::DocumentFormat type );
QString lastDocParamsFileName( meteo::map::proto::DocumentType type );

QString kPunchPath();
QString kWeatherPatternPath();
QString kWeatherJobsPath();
QString kDocumentJobsPath();
QString kDocumentPatternPath();
QString kFormalJobsPath();
QString kPunchMapPath();
QString kIsoParamPath();
QString kRadarParamPath();
QString kOrnamentPath();
QString kClimatConfPath();
QString kTranslitFunc(QString txt);
QString kTranslitFuncReverse( QString txt);
QString translitFuncKoi7(QString txt);
QString translitNameFuncKoi7(QString txt);

map::proto::RadarColors radarcolors();
map::proto::FieldColors isocolors();

QString cornumber2bbb( int cornumber );
int bbb2cornumber( const QString& bbb );

bool createMapSettingsDir();

void setLogHandler(); //< Устанавливает обработчик логов.

//bool loadStation(const meteo::sprinf::MultiStatementRequest& req, sprinf::Stations& stations); //TODO ЗАПРЕЩЕНО неконстантные ссылки. Кто добавил - УБРАТЬ!
bool loadStations(const meteo::sprinf::MultiStatementRequest& req, sprinf::Stations* result); //здесь указатель
bool loadStationTypes(meteo::sprinf::StationTypes* types, const meteo::sprinf::MultiStatementRequest& req);
bool loadStationByCoord(const meteo::GeoPoint& point, sprinf::Station* station );

QString telegramsPath();

ConnectProp dbConf(const QString& connectionName);
ConnectProp mongodbConfTelegram();
ConnectProp mongodbConfMeteo();
ConnectProp mongodbConfInter();
ConnectProp mongodbConfDocument();
ConnectProp mongodbConfForecast();
ConnectProp mongodbConfClimat();
ConnectProp mongodbConfJournal();
ConnectProp mongodbConfObanal();
ConnectProp mongodbConfUsers();

Dbi* db(const ConnectProp& prop);
Dbi* dbForecast();    //!< БД для прогнозов
Dbi* dbTelegram();    //!< БД для принятых телеграмм
Dbi* dbMeteo();       //!< БД для раскодированной информации
Dbi* dbDocument();    //!< БД для метеопродукции
Dbi* dbSprinf();
Dbi* dbClimat();      //!< БД для климатичесих данных
Dbi* dbObanal();      //!< БД для результатов ОА
Dbi* dbUsers();       //!< БД для пользовательских настроек
Dbi* dbJournal();     //!< БД журналирования
Dbi* dbInter();       //!< БД для пользовательских настроек

DbiQuery* dbqueryByName( const ConnectProp& params, const QString& queryName );
DbiQuery* dbquery( const ConnectProp& params, const QString query = QString() );

QString applicationConfName();

meteo::settings::Location punkt();

meteo::map::proto::Document lastBlankParams( meteo::map::proto::DocumentType type );
bool saveLastBlankParams( const meteo::map::proto::Document& doc );

QString lastDocPath();
void saveLastDocPath( const QString& path );

meteo::climat::ClimatSaveProp climatSettings(); //!< Получение настроек для климатического описания
uint currentStationIndex(); //!< Индекс станции из настроек пункта

// Загружаем настройки для обучения модели
meteo::cron::Settings loadMLSettings();
// Сохраняем  настройки для обучения модели
bool saveMLSettings(const meteo::cron::Settings &settings);

// Загружаем список станций, по которым считаются климатические показатели из файла настроек
meteo::climat::ClimatSaveProp loadClimatStations();
// Сохраняем список станций, по которым считаются климатические показатели в файл настроек
bool saveClimatStations(const meteo::climat::ClimatSaveProp &stations);

bool isRoot();
QString currentUserLogin();
QString currentUserName();
QString currentUserShortName();
QString wmoId();
QString hmsId();

QString rank( int code );
QString currentUserRank();
QString currentUserSurname();
QString currentUserDepartment();

QStringList geoLoaders();

QString serviceTitle(meteo::settings::proto::ServiceCode code, bool* ok = nullptr);
QString serviceAddress( meteo::settings::proto::ServiceCode code, bool* ok = nullptr );
rpc::Channel* serviceChannel( const QString& address );
rpc::Channel* serviceChannel( meteo::settings::proto::ServiceCode code );

int serviceTimeout( meteo::settings::proto::ServiceCode code );
meteo::settings::proto::Service service( meteo::settings::proto::ServiceCode code, bool* ok = nullptr );

void addGeoLoader( const QString& loader );

bool isEqual( const google::protobuf::Message& msg1, const google::protobuf::Message& msg2 );
bool isEqual( const google::protobuf::Message* msg1, const google::protobuf::Message* msg2 );
bool isEqual( const google::protobuf::Message& msg1, const google::protobuf::Message* msg2 );
bool isEqual( const google::protobuf::Message* msg1, const google::protobuf::Message& msg2 );

bool protomsg2arr( const google::protobuf::Message& msg, QByteArray* arr );
bool arr2protomsg( const QByteArray& arr, google::protobuf::Message* msg );

bool json2meteodata(const DbiEntry *json, TMeteoData* md );
bool json2meteoproto(const DbiEntry *json, surf::MeteoDataProto* proto, bool as_tree = false );

surf::MeteoDataProto meteodata2proto( const TMeteoData& md );


void removeComplexDuplication( surf::MeteoDataProto* md );

void saveSize( QWidget* wgt );
void loadSize( QWidget* wgt );


bool askUser( QWidget* wgt, const QString& message);

/*!
 * \brief - получить таблицу со значениями параметров codes и их описанием от сервиса справки загрузить ее в память
 * \param code - символьное обозначение метеопараметра ( code = str(descr) )
 */
void loadBufrTables( const QStringList& codes );

/*!
 * \brief - проверить наличие в памяти таблицы значений для параметра code (перед вызовом loadBufrTables)
 * \param code - символьное обозначение метеопараметра
 * \return
 */
bool hasBufrTable( const QString& code );

/*!
 * \brief - возвращает таблицу со значениями параметра code
 * \param code - символьное обозначение метеопараметра
 * \return - таблица со значениями (key) параметра и их описанием (value)
 */
QMap< int, QString > bufrTable( const QString& code );

/*!
 * \brief - возвращает описание значения value параметра code
 * \param code - символьное обозначение метеопараметра
 * \param value - значение метеопараметра code
 * \return - описание значения метеопараметра code
 */
QString bufrValueDescription( const QString& code, int value );

/*!
 * \brief - возвращает давление на уровне станции
 * \param h - высота станции
 * \param P - давление на уровне моря
 * \param T - температура воздуха
 * \return - давление
 */
float pressureOnStationLevel( float h, float P, float T = -9999.0);

/*!
 * \brief - возвращает давление приведённое к уровню моря
 * \param h - высота станции
 * \param P - давление на уровне стации
 * \param T - температура воздуха
 * \return - давление
 */
float pressureOnOceanLevel( float h, float P, float T = -9999.0);

/*! 
 * \brief Возвращает описание метеопараметра по его дескриптору
 * \param descr - дескриптор метеопараметра
 * \return - Контейнер со свойствами метеопараметра
 */
QMap< QString, QString > meteoparamProps( descr_t descr );
QMap< QString, QString > meteoparamPropsByName( const QString& descrname );

template<class T> QByteArray toByteArray( const T& t );
template<class T> int32_t fromByteArray( const QByteArray& arr, T* t );
template<class T> int32_t fromByteArray( const char* arr, T* t );


template<class T> QByteArray toByteArray( const T& t )
{
  uint32_t szof( sizeof(T) );
  QByteArray loc( szof, '\0' );
  ::memcpy( loc.data(), &t, szof );
  return loc;
}

template<class T> int32_t fromByteArray( const QByteArray& arr, T* t )
{
  if ( 0 == t ) {
    return -1;
  }
  uint32_t szof( sizeof(T) );
  if ( szof > arr.size() ) {
    return -1;
  }
  ::memcpy( t, arr.data(), szof );
  return szof;
}

template<class T> int32_t fromByteArray( const char* data, T* t )
{
  if ( 0 == t ) {
    return -1;
  }
  uint32_t szof( sizeof(T) );
  ::memcpy( t, data, szof );
  return szof;
}

const QMap< QString, QString >& moduleNames();

}
}

namespace meteo {
namespace internal {

enum DbType { kDbPtkpp,
              kDbMeteo,
              kDbMeteoLocal,
              kDbObanal,
              kDbForecast,
              kDbClimat,
              kDbJournal,
              kDbMappi,
              kDbDjango,
              kDbTypeMax };

class Global
{
public:
  Global();
  ~Global();

  ConnectProp dbConf(const QString& connectionName) const;
  ConnectProp dbConfDjango() const;

  ConnectProp mongodbConfTelegrams() const;
  ConnectProp mongodbConfMeteo() const;
  ConnectProp mongodbConfInter() const;
  ConnectProp mongodbConfDocument() const;
  ConnectProp mongodbConfSprinf() const;
  ConnectProp mongodbConfForecast() const;
  ConnectProp mongodbConfClimat() const;
  ConnectProp mongodbConfJournal() const;
  ConnectProp mongodbConfObanal() const;
  ConnectProp mongodbConfUser() const;

  Dbi* db(const ConnectProp& prop);
  Dbi* dbForecast();
  Dbi* dbTelegram();
  Dbi* dbMeteo();
  Dbi* dbDocument();
  Dbi* dbSprinf();
  Dbi* dbObanal();
  Dbi* dbClimat();
  Dbi* dbUsers();
  Dbi* dbInter();
  Dbi* dbJournal();

  Psql* dbDjango();

  settings::Location punkt() const;

  climat::ClimatSaveProp climatSettings() const { return climat::ClimatSaveProp(); }
  uint currentStationIndex() const { return 0; }

  bool isRoot() const { return false; }
  QString currentUserLogin() const;
  QString currentUserName() const;
  QString currentUserShortName() const ;

  QString wmoId() const;
  QString hmsId() const;

  QString rank(int code) const;
  QString currentUserRank() const;
  QString currentUserSurname() const;
  QString currentUserDepartment() const;

  QStringList geoLoaders() const { return geoloaders_; }

  QString serviceTitle(settings::proto::ServiceCode code, bool* ok = nullptr) const;
  QString serviceAddress(settings::proto::ServiceCode code, bool* ok = nullptr) const;
  rpc::Channel* serviceChannel(settings::proto::ServiceCode code) const;
  rpc::Channel* serviceChannel(const QString& addr) const;

  int serviceTimeout(settings::proto::ServiceCode code) const;
  settings::proto::Service service(settings::proto::ServiceCode code, bool* ok = nullptr) const;

  void addGeoLoader( const QString& loader );

  /*!
   * \brief - получить таблицу со значениями параметров codes и их описанием от сервиса справки загрузить ее в память
   * \param code - символьное обозначение метеопараметра ( code = str(descr) )
   */
  void loadBufrTables( const QStringList& codes );

  /*!
   * \brief - проверить наличие в памяти таблицы значений для параметра code (перед вызовом loadBufrTables)
   * \param code - символьное обозначение метеопараметра
   * \return
   */
  bool hasBufrTable( const QString& code ) const ;

  /*!
   * \brief - возвращает таблицу со значениями параметра code
   * \param code - символьное обозначение метеопараметра
   * \return - таблица со значениями (key) параметра и их описанием (value)
   */
  QMap< int, QString > bufrTable( const QString& code ) const ;

  /*!
   * \brief - возвращает описание значения value параметра code
   * \param code - символьное обозначение метеопараметра
   * \param value - значение метеопараметра code
   * \return - описание значения метеопараметра code
   */
  QString bufrValueDescription( const QString& code, int value ) const ;
  
  DbiQuery* dbqueryByName( const ConnectProp& params, const QString& queryName ) const ;
  DbiQuery* dbquery( const ConnectProp& params, const QString query = QString() ) const ;

protected:
  Dbi* createDatabase( const ConnectProp& params ) const ;

private:
  ConnectionDict dbConn_;
  QStringList geoloaders_;
  QMap< QString, QMap< int, QString > > bufrtables_;

  friend class TSingleton<Global>;
};

}

using Global = TSingleton<internal::Global>;

}

namespace meteo {

bool dbusSendNotification(const QString &title, const QString &message, const QString &icon,
                          int timeout, const QHash<QString, QString> &buttons);
bool dbusSendNotification(const QString &title, const QString &message, const QString &icon,
                          int timeout, const QStringList &buttons = QStringList());
bool dbusSendNotification(const QString &title, const QString &message, const QString &icon,
                          int timeout, const QString& buttonName, const QString &buttonText);

void dbusConnect(QObject *receiver, const char *slot);
void dbusDisconnect(QObject *receiver, const char *slot);

}

#endif
