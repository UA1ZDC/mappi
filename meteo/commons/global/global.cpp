#include "global.h"

#include <qwidget.h>
#include <qmessagebox.h>
#include <qpushbutton.h>
#include <qmutex.h>

#include <cross-commons/app/paths.h>

#include <commons/textproto/pbcache.h>
#include <commons/textproto/tprototext.h>
#include <commons/textproto/pbtools.h>

#include <meteo/commons/settings/tusersettings.h>
#include <meteo/commons/punchrules/punchrules.h>
#include <meteo/commons/global/common.h>

#include <sql/nosql/nosqlquery.h>
#include <sql/psql/psqlquery.h>
#include <sql/psql/psql.h>

#include <commons/funcs/fs.h>
#include <commons/funcs/selfaddress.h>
#include <qhostinfo.h>
#include <QtDBus>

#include "radarparams.h"
#include "weatherloader.h"

template<> ::meteo::internal::Global* meteo::Global::_instance = nullptr;

namespace meteo {
namespace global {
static const QMap< QString, QString > kModuleNames = TMap< QString, QString >()
  << qMakePair( QString("vizdoc"), QObject::tr( "ПК визуализации и документирования" ))
  << qMakePair( QString("recgmi"), QObject::tr( "ПК приема (передачи) ГМИ" ))
  << qMakePair( QString("sortgmi"), QObject::tr( "ПК сортировки ГМИ" ))
  << qMakePair( QString("contrdiag"), QObject::tr( "ПК самоконтроля и диагностики" ))
  << qMakePair( QString("settings"), QObject::tr( "ПК настройки" ))
  << qMakePair( QString("codecontrol"), QObject::tr( "ПК раскодирования и контроля ГМИ" ))
  << qMakePair( QString("obanal"), QObject::tr( "ПК объективного анализа" ))
  << qMakePair( QString("prepare"), QObject::tr( "ПК подготовки исходных данных" ))
  << qMakePair( QString("dbtask"), QObject::tr( "ПК обслуживания базы данных" ))
  << qMakePair( QString("dbconn"), QObject::tr( "ПК взаимодействия с базой данных" ))
  << qMakePair( QString("applied"), QObject::tr( "ПК прикладных задач" ))
  << qMakePair( QString("vko"), QObject::tr( "ПК автоматизированного составления военно-климатических описаний" ))
  << qMakePair( QString("rpc"), QObject::tr( "Удаленный вызов процедур" ))
  << qMakePair( QString("journal"), QObject::tr( "ПК журналирования" ))
  << qMakePair( QString("wrb"), QObject::tr( "ПК прикладных задач" ))
  << qMakePair( QString("puansoneditor"), QObject::tr( "ПК визуализации и документирования" ))
  << qMakePair( QString("global"), QObject::tr( "ПК журналирования" ))
  << qMakePair( QString("forecast"), QObject::tr( "ПК прогнозирования" ))
  << qMakePair( QString("prototext"), QObject::tr( "ПК настройки" ))
  << qMakePair( QString("oceandiag"), QObject::tr( "ПК визуализации и документирования" ))
  << qMakePair( QString("procview"), QObject::tr( "ПК самоконтроля и диагностики" ))
  << qMakePair( QString("monitor"), QObject::tr( "ПК самоконтроля и диагностики" ))
  << qMakePair( QString("procident"), QObject::tr( "ПК визуализации и документирования" ))
  << qMakePair( QString("aerodiag"), QObject::tr( "ПК прикладных задач" ))
  << qMakePair( QString("customui"), QObject::tr( "ПК визуализации и документирования" ))
  << qMakePair( QString("forecastwidget"), QObject::tr( "ПК прогнозирования" ))
  << qMakePair( QString("decodeviewer"), QObject::tr( "ПК визуализации и документирования" ))
  << qMakePair( QString("nabludenia"), QObject::tr( "ПК визуализации и документирования" ))
  << qMakePair( QString("tlgeditor"), QObject::tr( "ПК визуализации и документирования" ))
  << qMakePair( QString("settings"), QObject::tr( "ПК настройки" ))
  << qMakePair( QString("sprinf"), QObject::tr( "ПК взаимодействия с базой данных" ))
  << qMakePair( QString("map"), QObject::tr( "ПК визуализации и документирования" ))
  << qMakePair( QString("cron"), QObject::tr( "ПК самоконтроля и диагностики" ))
  << qMakePair( QString("planner"), QObject::tr( "ПК журналирования" ))
  << qMakePair( QString("verticalcut"), QObject::tr( "ПК прикладных задач" ))
  << qMakePair( QString("msgviewer"), QObject::tr( "ПК визуализации и документирования" ))
  << qMakePair( QString("radar"), QObject::tr( "ПК визуализации и документирования" ))
  << qMakePair( QString("sbor"), QObject::tr( "ПК сбора/распространения ГМИ" ))
  << qMakePair( QString("inter"), QObject::tr( "ПК взаимодействия" ))
  << qMakePair( QString("docmaker"), QObject::tr( "ПК формирования документов" ))
  << qMakePair( QString("meteo.settings"), QObject::tr( "ПК настройки" ));

static QMap< QByteArray, sprinf::Stations > stationcache_;
static QMutex stationmutex_(QMutex::Recursive);

static void insertLogIntoDataBase(tlog::Priority priority, const QString& sender, const QString& fileName, int line, const QString &message ){
  static auto insertJournalLogQueryName = QObject::tr("insert_journal");
  auto dt = QDateTime::currentDateTimeUtc();

  QString msg = message;
  msg.replace("%", QObject::tr("недостающий_аргумент"));

  std::unique_ptr<Dbi> db(global::dbJournal());
  if ( nullptr == db ){
    error_log << meteo::msglog::kDbConnectFailed;
    return;
  }

  std::unique_ptr<DbiQuery> query(db->queryByName(insertJournalLogQueryName));
  if ( nullptr == query ) {
    error_log << msglog::kDbRequestNotFound.arg(insertJournalLogQueryName);
    return;
  }
  query->arg( "priority", priority)
       .arg( "sender", sender)
       .arg( "username", currentUserName() )
       .arg( "filename", fileName )
       .arg( "linenumber", line )
       .arg( "ip", MnCommon::selfIpAddress() )
       .arg( "host", QHostInfo::localHostName() )
       .arg( "dt", dt )
       .arg( "message", msg.simplified() );

  if ( false == query->exec() ) {
    error_log << meteo::msglog::kDbRequestFailed.arg(query->query());
    return;
  }
  const DbiEntry& result = query->result();
  auto ok = qRound(result.valueDouble("ok"));
  if ( ok != 1 ){
    error_log << meteo::msglog::kDbRequestFailed.arg(query->query())
              << result.jsonExtendedString();
  }
}

const QMap< QString, QString >& moduleNames()
{
  return kModuleNames;
}

void logoutToDb(tlog::Priority priority, const QString &facility, const QString& fileName, int line, const QString &message )
{
  fMsgHandler old = TLog::setMsgHandler( TLog::stdOut );
  QString moduleName = kModuleNames[facility];

  QString sender = facility;

  if (false == moduleName.isEmpty()) {
    sender = moduleName;
  }
  TLog::stdOut( priority, sender, fileName, line, message );
  auto conf = global::Settings::instance()->displayMessageConfig();

  meteo::settings::RegLevel reg = conf.debug();

  switch(priority){
    case tlog::kDebug:
    case tlog::kNone:
      reg = conf.debug();
       break;
    case tlog::kInfo:
      reg = conf.info();
      break;
    case tlog::kWarning:
      reg = conf.warning();
      break;
    case tlog::kError:
      reg = conf.error();
      break;
    case tlog::kCritical:
      reg = conf.critical();
      break;
  }
  if ( reg != meteo::settings::kNone  )  {
    insertLogIntoDataBase(priority, sender, fileName, line, message);
  }

  TLog::setMsgHandler(old);
}

QMap< QString, QString > loadMongoFuncsNew()
{
  QMap< QString, QString > list;
  QStringList files = meteo::fileList( MnCommon::sharePath("meteo") + "/mongo.funcs", "js" );

  for ( auto f : files ) {
    QFile file(f);
    if ( false == file.open(QIODevice::ReadOnly) ) {
      error_log << QObject::tr("Не удалось открыть файл '%1' для чтения. Ошибка = %2")
        .arg(f)
        .arg( file.errorString() );
      continue;
    }
    QString str = QString::fromUtf8( file.readAll() );
    list.insert( QFileInfo(file).baseName(), str );
  }
  return list;
}


ConnectProp dbConf(const QString& connectionName) { return ::meteo::Global::instance()->dbConf(connectionName); }
ConnectProp mongodbConfObanal() { return ::meteo::Global::instance()->mongodbConfObanal(); }
ConnectProp mongodbConfTelegram()  { return ::meteo::Global::instance()->mongodbConfTelegrams(); }
ConnectProp mongodbConfMeteo()     { return ::meteo::Global::instance()->mongodbConfMeteo(); }
ConnectProp mongodbConfInter()     { return ::meteo::Global::instance()->mongodbConfInter(); }
ConnectProp mongodbConfDocument()  { return ::meteo::Global::instance()->mongodbConfDocument(); }
ConnectProp mongodbConfForecast()  { return ::meteo::Global::instance()->mongodbConfForecast(); }
ConnectProp mongodbConfClimat()    { return ::meteo::Global::instance()->mongodbConfClimat(); }
ConnectProp mongodbConfJournal()   { return ::meteo::Global::instance()->mongodbConfJournal(); }
ConnectProp mongodbConfUsers()     { return ::meteo::Global::instance()->mongodbConfUser(); }


Dbi* db(const ConnectProp& prop) { return ::meteo::Global::instance()->db(prop); }
Dbi* dbForecast(){ return ::meteo::Global::instance()->dbForecast();   }
Dbi* dbTelegram() { return ::meteo::Global::instance()->dbTelegram();   }
Dbi* dbMeteo() { return ::meteo::Global::instance()->dbMeteo(); }
Dbi* dbDocument() { return ::meteo::Global::instance()->dbDocument(); }
Dbi* dbSprinf() { return ::meteo::Global::instance()->dbSprinf();}
Dbi* dbClimat() { return ::meteo::Global::instance()->dbClimat();}
Dbi* dbObanal() { return ::meteo::Global::instance()->dbObanal(); }
Dbi* dbUsers() { return ::meteo::Global::instance()->dbUsers();}
Dbi* dbInter() { return ::meteo::Global::instance()->dbInter();}
Dbi* dbJournal() { return ::meteo::Global::instance()->dbJournal(); }
DbiQuery* dbquery( const ConnectProp& params, const QString query )
{
  return ::meteo::Global::instance()->dbquery( params, query );
}

DbiQuery* dbqueryByName( const ConnectProp& params, const QString& query )
{
  return ::meteo::Global::instance()->dbqueryByName( params, query );
}

QString wmoId()
{ return ::meteo::Global::instance()->wmoId();  }

QString hmsId()
{ return ::meteo::Global::instance()->hmsId();  }

meteo::settings::Location punkt()
{ return ::meteo::Global::instance()->punkt(); }

QStringList geoLoaders()
{ return ::meteo::Global::instance()->geoLoaders(); }

bool isRoot()
{ return ::meteo::Global::instance()->isRoot(); }

QString rank( int code )
{ return ::meteo::Global::instance()->rank(code); }

QString currentUserRank()
{ return ::meteo::Global::instance()->currentUserRank(); }

QString currentUserSurname()
{ return ::meteo::Global::instance()->currentUserSurname(); }

QString currentUserDepartment()
{ return ::meteo::Global::instance()->currentUserDepartment(); }

QString currentUserLogin()
{ return ::meteo::Global::instance()->currentUserLogin(); }

QString currentUserName(){
    return meteo::Global::instance()->currentUserName();
}



QString currentUserShortName()
{ return ::meteo::Global::instance()->currentUserShortName(); }

QString serviceTitle(settings::proto::ServiceCode code, bool* ok)
{ return ::meteo::Global::instance()->serviceTitle(code,ok); }

QString serviceAddress(settings::proto::ServiceCode code, bool* ok)
{ return ::meteo::Global::instance()->serviceAddress(code,ok); }

int serviceTimeout( meteo::settings::proto::ServiceCode code )
{ return ::meteo::Global::instance()->serviceTimeout(code); }

rpc::Channel* serviceChannel( meteo::settings::proto::ServiceCode code )
{ return ::meteo::Global::instance()->serviceChannel(code); }

rpc::Channel *serviceChannel(const QString &address)
{
  return ::meteo::Global::instance()->serviceChannel(address);
}

meteo::settings::proto::Service service( meteo::settings::proto::ServiceCode code, bool* ok )
{ return ::meteo::Global::instance()->service(code, ok); }

void addGeoLoader( const QString& loader )
{
  return ::meteo::Global::instance()->addGeoLoader(loader);
}


QMap< int, QPair<QString, int> > kMeteoCenters()
{
  QMap< int, QPair<QString, int> > centers;
  QFileInfo fi( QDir::homePath() + QString("/.meteo/map/centers.cache") );
  meteo::sprinf::MeteoCenters proto;
  if ( false == fi.exists() || false == TProtoText::fromFile( fi.absoluteFilePath(), &proto ) ) {
    if ( false == createMapSettingsDir() ) {
      warning_log << QObject::tr("Не удалось создать директорию для настроек %1").arg( fi.absolutePath() );
    }
    rpc::Channel* ch = serviceChannel( settings::proto::kSprinf );
    if ( nullptr == ch ) {
      return centers;
    }
    meteo::sprinf::MeteoCenterRequest req;
    meteo::sprinf::MeteoCenters* res = ch->remoteCall(&meteo::sprinf::SprinfService::GetMeteoCenters, req, 50000 );
    delete ch; ch = nullptr;
    if ( nullptr == res || (res->has_result() && false == res->result())) {
      return centers;
    }
    proto.CopyFrom( *res );
    delete res; res = nullptr;
    TProtoText::toFile( proto, fi.absoluteFilePath() );
  }
  for (int i = 0, sz = proto.center_size(); i < sz; ++i ) {
    QString name = pbtools::toQString( proto.center(i).short_name() );
    if ( true == name.isEmpty() ) {
      name = pbtools::toQString( proto.center(i).name() );
    }
    centers.insert( static_cast<int>(proto.center(i).id()) , QPair<QString, int>(name, proto.center(i).priority()));
  }
  return centers;
}

QMap< int, QString > kLevelTypes()
{
  QMap< int, QString > levels;
  QFileInfo fi( QDir::homePath() + QString("/.meteo/map/leveltypes.cache") );
  meteo::sprinf::LevelTypes proto;
  if ( false == fi.exists() || false == TProtoText::fromFile( fi.absoluteFilePath(), &proto ) ) {
    if ( false == createMapSettingsDir() ) {
      warning_log << QObject::tr("Не удалось создать директорию для настроек %1").arg( fi.absolutePath() );
    }
    rpc::Channel* ctrl = serviceChannel( settings::proto::kSprinf );

    if ( nullptr == ctrl ) {
      return levels;
    }
    meteo::sprinf::TypesRequest req;
    meteo::sprinf::LevelTypes* res = ctrl->remoteCall(&meteo::sprinf::SprinfService::GetLevelTypes, req, 50000 );
    delete ctrl; ctrl = nullptr;
    if ( nullptr == res || (res->has_result() && false == res->result())){
      return levels;
    }
    proto.CopyFrom( *res );
    delete res; res = nullptr;
    TProtoText::toFile( proto, fi.absoluteFilePath() );
  }
  for (int i = 0, sz = proto.level_size(); i < sz; ++i ) {
    QString name = pbtools::toQString( proto.level(i).name() );
    levels.insert( static_cast<int>(proto.level(i).type()) , name );
  }
  return levels;
}

QString lastDocParamsFileNamePostfix( meteo::map::proto::DocumentType type )
{
  QString postfix(".unc");
  switch ( type ) {
    case meteo::map::proto::kGeoMap:
      postfix = ".geo";
      break;
    case meteo::map::proto::kVerticalCut:
      postfix = ".cut";
      break;
    case meteo::map::proto::kAeroDiagram:
      postfix = ".aero";
      break;
    case meteo::map::proto::kOceanDiagram:
      postfix = ".ocean";
      break;
    case meteo::map::proto::kMeteogram:
      postfix = ".mg";
      break;
    case meteo::map::proto::kFormalDocument:
      postfix = ".fd";
      break;
  }
  return postfix;
}

/**
 * @brief Возвращает строку с расширением по типу
 * 
 * @param type meteo::map::proto::DocumentFormat
 * @return QString 
 */
QString getDocumentFormatExt( meteo::map::proto::DocumentFormat type )
{
  QString postfix(".unc");
  switch ( type ) {
    case meteo::map::proto::kJpeg:
      postfix = "jpeg";
      break;
    case meteo::map::proto::kBmp:
      postfix = "bmp";
      break;
    case meteo::map::proto::kPng:
      postfix = "png";
      break;
    case meteo::map::proto::kSxf:
      postfix = "sxf";
      break;
    case meteo::map::proto::kPtkpp:
      postfix = "ptkpp";
      break;
    case meteo::map::proto::kOdt:
      postfix = "odt";
      break;
    case meteo::map::proto::kOds:
      postfix = "ods";
      break;
    case meteo::map::proto::kOdp:
      postfix = "odp";
      break;
    case meteo::map::proto::kPdf:
      postfix = "pdf";
      break;
  };
  return postfix;
}

QString lastDocParamsFileName( meteo::map::proto::DocumentType type )
{ return kLastDocParamsPath + "/" + kLastDocParamsFileNameNach + lastDocParamsFileNamePostfix(type); }

climat::ClimatSaveProp climatSettings()
{ return ::meteo::Global::instance()->climatSettings(); }

uint currentStationIndex()
{ return ::meteo::Global::instance()->currentStationIndex(); }

/**
 * @brief Загружаем настройки для обучения модели
 * 
 * @return QList<sprinf::Station> 
 */
meteo::cron::Settings loadMLSettings()
{
  QString settingsDirPath = MnCommon::etcPath() + "cron.d"; 
  QString cronTasksFileName = QDir(settingsDirPath).absoluteFilePath("ml.cron.conf");

  meteo::cron::Settings cronConfig;
  TProtoText::fromFile(cronTasksFileName, &cronConfig);
  
  return cronConfig;
}

/**
 * @brief Загружаем настройки для обучения модели
 * 
 * @return QList<sprinf::Station> 
 */
bool saveMLSettings(const meteo::cron::Settings &settings)
{
  QString settingsDirPath = MnCommon::etcPath() + "cron.d"; 
  QString cronTasksFileName = QDir(settingsDirPath).absoluteFilePath("ml.cron.conf");

  if ( false == TProtoText::toFile(settings, cronTasksFileName) ) {
    return false;
  }
  return true;
}

/**
 * @brief Загружаем список станций, по которым считаются климатические показатели из файла настроек
 * 
 * @return QList<sprinf::Station> 
 */
meteo::climat::ClimatSaveProp loadClimatStations()
{
  meteo::climat::ClimatSaveProp stations;
  QFile ifile(meteo::global::kClimatConfPath());
  if ( false == ifile.open(QIODevice::OpenModeFlag::ReadOnly)){
    auto emessage = QObject::tr("Файл с настройками климата не найден."
                                    "Использованы настройки по умолчанию");
    // warning_log.msgBox() << emessage;
    return meteo::climat::ClimatSaveProp();
  }
  auto data = ifile.readAll();

  if ( false == TProtoText::fillProto(QString(data), &stations) ) {
    auto emessage = QObject::tr("Настройки климата прочитаны, но "
                                    "при разборе произошли ошибки."
                                    "Использованы настройки по умолчанию");
    // warning_log.msgBox() << emessage;
    return meteo::climat::ClimatSaveProp();
  }
  
  return stations;
}

/**
 * @brief Сохраняем список станций, по которым считаются климатические показатели в файл настроек
 * 
 * @param stations 
 * @return true 
 * @return false 
 */
bool saveClimatStations(const meteo::climat::ClimatSaveProp &stations)
{  
  if ( false == TProtoText::toFile(stations, meteo::global::kClimatConfPath()) ) {
    return false;
  }
  return true;
}


QString kPunchPath()
{
  return MnCommon::sharePath("meteo") + "/punch/";
}

QString kWeatherPatternPath()
{
  return MnCommon::sharePath("meteo") + "/weather/";
}

QString kWeatherJobsPath()
{
  return MnCommon::sharePath("meteo") + "/jobs/weather";
}

QString kDocumentJobsPath()
{
  return MnCommon::sharePath("meteo") + "/jobs/formal";
}

QString kDocumentPatternPath()
{
  return MnCommon::sharePath("meteo") + "/documents/";
}

QString kFormalJobsPath()
{
  return MnCommon::sharePath("meteo") + "/jobs/formal";
}

QString kPunchMapPath()
{
  return MnCommon::sharePath("meteo") + "/punch.map/";
}

QString kIsoParamPath()
{
  return MnCommon::etcPath("meteo") + "/isoline.conf.d/";
}

QString kRadarParamPath()
{
  return MnCommon::etcPath("meteo") + "/radar.conf.d/";
}

QString kClimatConfPath()
{
  return MnCommon::etcPath() + "/climat.conf";
}

QString kOrnamentPath()
{
  return MnCommon::sharePath("meteo") + "/ornament.conf.d/";
}

bool createMapSettingsDir()
{
  QFileInfo fi(kMapColorSettingsFilename);
  QDir dir (fi.path());
  if ( false == dir.exists( dir.absolutePath() ) ) {
    if ( false == dir.mkpath( dir.absolutePath() ) ) {
      error_log << QObject::tr("Не удалось создать директорию %1. Свойства картографической основы невозможно загрузить").arg( dir.absolutePath() );
      return false;
    }
  }
  return true;
}

QString kTranslitFunc(QString txt)
{
  txt.replace(QRegExp("\\W+"), "_");
  txt.replace(QRegExp("_+"), "_");
  QString res;

  for (int idx = 0; idx < txt.length(); idx++) {
    if (kTranslit.contains(txt[idx].toUpper())) {
      if (txt[idx].isUpper()) {
        res.append(kTranslit.value(txt[idx].toUpper()));
      }
      else {
        res.append(kTranslit.value(txt[idx].toUpper()).toLower());
      }
    } else {
      res.append(txt.at(idx));
    }
  }

  return res;
}

QString kTranslitFuncReverse( QString txt)
{
  txt.replace(QRegExp("\\W+"), "_");
  txt.replace(QRegExp("_+"), "_");
  for (int idx = 0; idx < txt.length(); idx++) {
    if (kTranslit.values().contains(txt[idx].toUpper())) {
      if (txt[idx].isUpper()) {
        txt.replace(idx, 1, kTranslit.key(txt[idx].toUpper()));
      }
      else {
        txt.replace(idx, 1, kTranslit.key(txt[idx].toUpper()).toLower());
      }
    }
  }
  return txt;
}

QString translitFuncKoi7(QString txt)
{
  for (int idx = 0; idx < txt.length(); idx++) {
    if (kTranslitKoi7.contains(txt[idx].cell())) {
      QString letter = kTranslitKoi7.value(txt[idx].cell());
      if (0 != idx && txt[idx - 1].isLetterOrNumber()) {
        letter = letter.toLower();
      }
      txt.replace(idx, 1, letter);
    }
  }

  return txt;
}

/**
 * функция транслитерации погоды
 * @param  txt [description]
 * @return     [description]
 */
QString translitNameFuncKoi7( QString input){

  QString txt=input.toUpper();

  // проходимся по всем буквам
  for (int idx = 0; idx < txt.length(); idx++) {
    // если есть такой ключ (буква в словаре)
    if (kTranslitKoi7.values().contains( QString(txt[idx]) )) {
      // забираем символ, который с ним связан
      QString letter="";

      QMapIterator<uint8_t, QString> it(kTranslitKoi7);
      while (it.hasNext()) {
        it.next();
        if ( it.value()==QString(txt[idx]) ){
          letter = it.key();
          break;
        }
      }

      // если это не начальная буква и предыдущая не цифра или буква, то уменьшаем регистр
      // заменяем в исходной строке
      txt.replace(idx, 1, letter);
    }
  }

  return txt;
}

map::proto::RadarColors radarcolors() {
  map::RadarParams params( kRadarParamPath() );
  map::proto::RadarColors colors = params.protoParams();
  return colors;
};

map::proto::FieldColors isocolors() {
  auto wl = map::WeatherLoader::instance();
  auto isoparams = wl->isolibrary();
  map::proto::FieldColors colors;
  for ( auto it = isoparams.begin(), end = isoparams.end(); it != end; ++it ) {
    auto color = colors.add_color();
    color->CopyFrom( *it );
  }
  return colors;
};

QString cornumber2bbb( int cornumber )
{
  if ( 1 > cornumber ) {
    return "";
  }
  if ( 24 < cornumber ) {
    cornumber = 24;
  }
  QString bbb("CC");
  uint16_t chnum = QChar('A').unicode() + uint16_t(cornumber) - 1;
  return bbb + QChar(chnum);
}

int bbb2cornumber( const QString& bbb )
{
  QRegExp cor_rx("(AA|aa|CC|cc)([A-Z|a-z])");
  QString locbbb = bbb.simplified();
  if ( 3 != locbbb.length() ) {
    warning_log << QObject::tr("tlg.BBB.length != 3 = %1. BBB = %2.")
      .arg(locbbb.length())
      .arg(locbbb);
  }
  if ( 3 > locbbb.length() ) {
    error_log << QObject::tr("tlg.BBB.length < 3. Группа BBB не обрабатывается");
  }
  int res = cor_rx.indexIn(locbbb);
  if ( -1 == res ) {
    return 0;
  }
  if ( 0 != res ) {
    return 0;
  }
  uint16_t start = QChar('A').unicode();
  int num = cor_rx.cap(2).toUpper()[0].unicode() - start + 1;
  if ( 0 > num ) {
    num = 0;
  }
  return num;
}

meteo::map::proto::Document defaultMapBlankParams( )
{
  meteo::map::proto::Document proto;
  auto location = meteo::global::Settings::instance()->location();
  auto coord = location.coord();
  bool hadData = true == coord.IsInitialized() &&
                 ( true == coord.has_lat_deg() || true == coord.has_lat_radian() ) &&
                 ( true == coord.has_lon_deg() || true == coord.has_lon_radian() );

  if ( true == hadData ) {
    proto.mutable_map_center()->CopyFrom(coord);
    proto.mutable_doc_center()->CopyFrom(coord);
  }
  else {
    auto mapCenter = proto.mutable_map_center();
    mapCenter->set_lon_deg(30.18);
    mapCenter->set_lat_deg(59.56);
    auto docCetner = proto.mutable_doc_center();
    docCetner->set_lon_deg(30.18);
    docCetner->set_lat_deg(59.56);
  }
  proto.set_geoloader(QObject::tr("ptkpp").toStdString());
  proto.set_projection(meteo::ProjectionType::kMercat);
  proto.set_scale(14);
  proto.set_punchorient(meteo::map::proto::PunchOrient::kNoOrient);
  proto.set_gridloader("grid.common");
  proto.set_text_towards_iso(false);
  proto.set_ruleview(meteo::kBadWithRect);
  proto.set_smooth_iso(true);
  proto.set_cross_type(meteo::map::proto::CrossType::kCrossLayer);
  proto.set_outlinecolor(4294967295);
  proto.set_cache(false);
  saveLastBlankParams(proto);
  return proto;
}

meteo::map::proto::Document defaultBlankParams( meteo::map::proto::DocumentType type)
{
  meteo::map::proto::Document ret_val;
  switch ( type ) {
    case meteo::map::proto::kGeoMap:
      return  defaultMapBlankParams();
    case meteo::map::proto::kVerticalCut:
    case meteo::map::proto::kAeroDiagram:
    case meteo::map::proto::kOceanDiagram:
    case meteo::map::proto::kMeteogram:
    case meteo::map::proto::kFormalDocument:
      //FIXME не всем документам это нужно, мб для каждого типа документов нужна своя defaultBlankParams?
      ret_val.set_projection(meteo::ProjectionType::kGeneral);
      ret_val.mutable_map_center()->set_lat_radian(0.0);
      ret_val.mutable_map_center()->set_lon_radian(0.0);
      ret_val.set_doctype(type);
      break;
  }

  return ret_val;
}

meteo::map::proto::Document lastBlankParams( meteo::map::proto::DocumentType doctype )
{

  QString filename = lastDocParamsFileName(doctype);
  meteo::map::proto::Document proto;
  proto.set_doctype(doctype);
  QFileInfo fi(filename);
  if ( false == fi.exists() ) {
    return defaultBlankParams(doctype);
  }
  QDir dir = fi.dir();
  if ( false == dir.exists() ) {
    if ( false == global::createMapSettingsDir() ) {
      error_log << QObject::tr("Не удалось создать директорию для сохранения параметров документа");
      return defaultBlankParams(doctype);
    }
  }
  QFile file( fi.absoluteFilePath() );
  if ( false == file.open( QIODevice::ReadOnly ) ) {
    error_log
      << QObject::tr("Не удалось открыть файл для чтения параметров документа")
      << "\n\t" << QObject::tr("Описание ошибки: %1")
      .arg( file.errorString() );
    return defaultBlankParams(doctype);
  }
  QByteArray arr = file.readAll();
  QString str = QString::fromLocal8Bit( arr.data(), arr.size() );
  if ( false == TProtoText::fillProto( str, &proto ) ) {
    error_log << QObject::tr("Не удалось заполнить прото-сообщение с параметрами документа из файла %1")
      .arg(fi.absoluteFilePath());
    return defaultBlankParams(doctype);
  }

  //FIXME не всем документам это нужно
  if ( meteo::map::proto::kGeoMap == doctype &&
       (false == proto.has_geoloader() ||
       false == proto.has_projection() ||
       false == proto.has_map_center() )){
    error_log << QObject::tr("Часть полей не инициализировано");
    auto defaults = defaultBlankParams(doctype);
    return defaults;
  }

  if ( ( QString("SXF") == pbtools::toQString( proto.geoloader() ) && meteo::kMercat == proto.projection() )
    || /*( QString("ptkpp") == pbtools::toQString( proto.geoloader() ) &&*/ ( meteo::kMercat == proto.projection() ) ) {
    proto.mutable_map_center()->set_lat_radian(0.0);
    proto.mutable_map_center()->set_lon_radian(0.0);
  }
  return proto;
}

bool saveLastBlankParams( const meteo::map::proto::Document& proto )
{
  QString filename = lastDocParamsFileName( proto.doctype() );
  QString str = TProtoText::toText(proto);
  QFileInfo fi(filename);
  QDir dir = fi.dir();
  if ( false == dir.exists() ) {
    if ( false == global::createMapSettingsDir() ) {
      error_log << QObject::tr("Не удалось создать директорию для сохранения параметров документа");
      return false;
    }
  }
  QFile file( fi.absoluteFilePath() );
  if ( false == file.open( QIODevice::WriteOnly | QIODevice::Truncate ) ) {
    error_log
      << QObject::tr("Не удалось открыть файл для сохранения параметров документа")
      << "\n\t" << QObject::tr("Описание ошибки: %1")
      .arg( file.errorString() );
    return false;
  }
  file.write(str.toUtf8());
  file.flush();
  file.close();
  return true;
}

QString lastDocPath()
{
  createMapSettingsDir();
  QFile file(kMapTemporaryDir + "/lastdocsavepath");
  if ( false == file.exists() ) {
    return QDir::homePath();
  }
  if ( 1024*1024 < QFileInfo(file).size() ) {
    return QDir::homePath();
  }
  file.open( QIODevice::ReadOnly );
  QByteArray arr = file.readAll();
  return QString::fromLocal8Bit(arr);
}

void saveLastDocPath( const QString& path )
{
  createMapSettingsDir();
  QFile file(kMapTemporaryDir + "/lastdocsavepath");
  file.open( QIODevice::WriteOnly | QIODevice::Truncate );
  file.write( path.toLocal8Bit() );
  file.flush();
  file.close();
}

bool isEqual( const google::protobuf::Message& msg1, const google::protobuf::Message& msg2 )
{
  return isEqual( &msg1, &msg2 );
}

bool isEqual( const google::protobuf::Message* msg1, const google::protobuf::Message* msg2 )
{
  if( ( msg1->GetDescriptor()->full_name() != msg2->GetDescriptor()->full_name() ) &&
      ( msg1->GetDescriptor()->name() != msg2->GetDescriptor()->name() )) {
    return false;
  }
  if ( msg1->SerializeAsString() != msg2->SerializeAsString() ) {
    return false;
  }
  return true;
}

bool isEqual( const google::protobuf::Message& msg1, const google::protobuf::Message* msg2 )
{
  return isEqual( &msg1, msg2 );
}

bool isEqual( const google::protobuf::Message* msg1, const google::protobuf::Message& msg2 )
{
  return isEqual( msg1, &msg2 );
}

bool protomsg2arr( const google::protobuf::Message& msg, QByteArray* arr )
{
  if ( nullptr == arr ) {
    error_log << QObject::tr("Нулевой указатель");
    return false;
  }
  std::string str = msg.SerializeAsString();
  QByteArray a( str.data(), str.size() );
  *arr = a;
  return true;
}

bool arr2protomsg( const QByteArray& arr, google::protobuf::Message* msg )
{
  if ( nullptr == msg ) {
    error_log << QObject::tr("Нулевой указатель");
    return false;
  }
  std::string str( arr.constData(), arr.size() );
  return msg->ParsePartialFromString(str);
}

void saveSize( QWidget* wgt )
{
  createMapSettingsDir();
  QFileInfo fi(kMapColorSettingsFilename);
  QDir dir(fi.path());
  QString classname( wgt->metaObject()->className() );
  QFile file( dir.absoluteFilePath(classname.replace("::","_")) );
  if ( false == file.open( QIODevice::WriteOnly | QIODevice::Truncate ) ) {
    error_log << QObject::tr("Не удалось сохранить размер окна в файл %1").arg( file.fileName() );
    return;
  }
  QDataStream stream(&file);
  stream << wgt->size();
  file.flush();
  file.close();
}

void loadSize( QWidget* wgt )
{
  QFileInfo fi(kMapColorSettingsFilename);
  QDir dir(fi.path());
  QString classname( wgt->metaObject()->className() );
  QFile file( dir.absoluteFilePath(classname.replace("::","_")) );
  if ( false == file.exists() ) {
    return;
  }
  if ( false == file.open( QIODevice::ReadOnly ) ) {
    error_log << QObject::tr("Не удалось прочитать размер окна в файле %1").arg( file.fileName() );
    return;
  }
  QSize sz;
  QDataStream stream( &file );
  stream >> sz;
  wgt->resize(sz);
}

bool askUser( QWidget* wgt, const QString& message) {
  QMessageBox* msgBox = new QMessageBox(wgt);
  msgBox->setText(message);
  msgBox->setWindowTitle(QObject::tr("Внимание"));
  QAbstractButton* yesBtn = dynamic_cast<QAbstractButton*>(msgBox->addButton(QObject::tr("Принять"),QMessageBox::ButtonRole::YesRole));
  QAbstractButton* noBtn = dynamic_cast<QAbstractButton*>(msgBox->addButton(QObject::tr("Отмена"),QMessageBox::ButtonRole::NoRole));

  msgBox->exec();

  if ( msgBox->clickedButton() == yesBtn ){
    return true;
  }
  if ( msgBox->clickedButton() == noBtn ){
    return false;
  }
  error_log << QObject::tr("Получен неизвестный ответ от пользователя");
  return false;
}

meteo::sprinf::GmiTypes kGmiTypes()
{
  QFileInfo fi( QDir::homePath() + QString("/.meteo/map/gmitypes.cache") );
  meteo::sprinf::GmiTypes proto;
  if ( false == fi.exists() || false == TProtoText::fromFile( fi.absoluteFilePath(), &proto ) ) {
    if ( false == createMapSettingsDir() ) {
      warning_log << QObject::tr("Не удалось создать директорию для настроек %1").arg( fi.absolutePath() );
    }
    rpc::Channel* ctrl = serviceChannel( settings::proto::kSprinf );
    if ( nullptr == ctrl ) {
      return proto;
    }
    meteo::sprinf::TypesRequest req;
    meteo::sprinf::GmiTypes* res = ctrl->remoteCall( &meteo::sprinf::SprinfService::GetGmiTypes, req, 50000 );
    delete ctrl; ctrl = nullptr;
    if ( nullptr == res ) {
      return proto;
    }
    proto.CopyFrom( *res );
    delete res; res = nullptr;
    TProtoText::toFile( proto, fi.absoluteFilePath() );
  }
  return proto;
}

QMap<int, QStringList> kStandardLevels()
{
  QMap< int, QStringList > levels;
  QFileInfo fi( QDir::homePath() + QString("/.meteo/map/standardlevels.cache") );
  meteo::sprinf::LevelTypes proto;
  if ( false == fi.exists() || false == TProtoText::fromFile( fi.absoluteFilePath(), &proto ) ) {
    if ( false == createMapSettingsDir() ) {
      warning_log << QObject::tr("Не удалось создать директорию для настроек %1").arg( fi.absolutePath() );
    }
    rpc::Channel* ctrl = serviceChannel( settings::proto::kSprinf );
    if ( nullptr == ctrl ) {
      return levels;
    }
    meteo::sprinf::TypesRequest req;
    meteo::sprinf::LevelTypes* res = ctrl->remoteCall(&meteo::sprinf::SprinfService::GetLevelTypes, req, 50000 );
    delete ctrl; ctrl = nullptr;
    if ( nullptr == res ) {
      return levels;
    }
    proto.CopyFrom( *res );
    delete res; res = nullptr;
    TProtoText::toFile( proto, fi.absoluteFilePath() );
  }
  for (int i = 0, sz = proto.level_size(); i < sz; ++i ) {
    QStringList values;
    for( int j = 0; j < proto.level(i).value_size(); ++j){
      QString value = QString::number(proto.level(i).value(j));
      values.append(value);
    }
    levels.insert(static_cast<int>(proto.level(i).type()), values);
  }
  return levels;
}

QString telegramsPath()
{ return MnCommon::varPath() + "/telegrams"; }

bool readStationCacheFromMemory( const sprinf::MultiStatementRequest& req, sprinf::Stations* stations )
{
  QMutexLocker lock(&stationmutex_);
  std::string str;
  req.SerializePartialToString(&str);
  QByteArray arr( str.data(), str.size() );
  if ( true == stationcache_.contains(arr) ) {
    stations->CopyFrom( stationcache_[arr] );
    return true;
  }
  return false;
}

bool setStationCacheToMemory( const sprinf::MultiStatementRequest& req, const sprinf::Stations& stations )
{
  QMutexLocker lock(&stationmutex_);
  std::string str;
  req.SerializePartialToString(&str);
  QByteArray arr( str.data(), str.size() );
  if ( false == stationcache_.contains(arr) ) {
    stationcache_.insert( arr, stations );
    return true;
  }
  warning_log << QObject::tr("Попытка повторно добавить станции в кеш. Фильтра станций =") << req.Utf8DebugString();
  return false;
}

bool loadStations(const sprinf::MultiStatementRequest &req, meteo::sprinf::Stations* stations)
{
  bool stations_loaded = readStationCacheFromMemory( req, stations );
  if ( true == stations_loaded ) {
    debug_log << "[ ok ] READ CACHE - sprinf.service::GetStations()";
  }
  else {
    meteo::rpc::Channel* control = meteo::global::serviceChannel(meteo::settings::proto::kSprinf);
    meteo::sprinf::Stations* resp = nullptr;
    if ( nullptr != control ) {
      resp = control->remoteCall(&meteo::sprinf::SprinfService::GetStations, req, 20000);
      delete control;
      control = nullptr;
    }

    if ( nullptr == resp ) {
      return false;
    }

    if ( resp->result() == false ) {
      delete resp;
      return false;
    }

    stations->CopyFrom(*resp);
    delete resp;
    setStationCacheToMemory( req, *stations );
  }

  return true;
}
/*
bool loadStations(const meteo::sprinf::MultiStatementRequest& req, sprinf::Stations* result)
{
  pbtools::PbCache cache("sprinf.service", "getstations", 604800);
  cache.setCachePath(QDir::homePath() + "/.meteo/pb_cache");
  if (true == cache.read(req, result) ) {
    return true;
  }

  meteo::rpc::Channel* control = meteo::global::serviceChannel(meteo::settings::proto::kSprinf);
  meteo::sprinf::Stations* resp = nullptr;
  if ( nullptr != control ) {
    resp = control->remoteCall(&meteo::sprinf::SprinfService::GetStations, req, 20000);
    delete control;
    control = nullptr;
  }

  if ( nullptr == resp ) {
    return false;
  }
  if ( resp->result() == false ) {
    delete resp;
    return false;
  }

  result->CopyFrom(*resp);
  delete resp;

  if ( false == cache.write(req,*result) ){
    warning_log << QObject::tr("При записи данных в кэш произошла ошибка.");
  }

  return true;
}*/

bool loadStationTypes(sprinf::StationTypes *types, const sprinf::MultiStatementRequest &req)
{
  if ( nullptr == types ) { return false; }

  meteo::sprinf::TypesRequest request;
  for ( int i=0,isz=req.type_size(); i<isz; ++i ) {
    request.add_type(req.type(i));
  }

  pbtools::PbCache cache("sprinf.service", "getstationtypes");
  cache.setCachePath(QDir::homePath() + "/.meteo/pb_cache");
  if (/* cacheOn_ && */cache.read(request, types) ) {
//    debug_log << "[ ok ] READ CACHE - sprinf.service::GetStationTypes()";
    return true;
  }

  meteo::rpc::Channel* ch = meteo::global::serviceChannel(meteo::settings::proto::kSprinf);
  if ( nullptr == ch ) {
    error_log << QObject::tr("Не удалось установить соединение с сервисом справки");
    return false;
  }
  meteo::sprinf::StationTypes* resp = ch->remoteCall(&meteo::sprinf::SprinfService::GetStationTypes, request, 20000);
  ch->disconnect();
  delete ch;

  if ( nullptr == resp ) { return false; }

  types->CopyFrom(*resp);
  delete resp;

  bool r = cache.write(request,*types);
  Q_UNUSED(r);

  return true;
}

bool loadStationByCoord(const meteo::GeoPoint& point, sprinf::Station* station)
{
  static QMap<meteo::GeoPoint,sprinf::Station> cache;

  if ( nullptr == station ){
    return false;
  }

  if ( true == cache.contains(point) ){
    auto result = cache[point];
    station->CopyFrom(result);
    return true;
  }

  auto channel = std::unique_ptr<rpc::Channel>(serviceChannel(settings::proto::ServiceCode::kSprinf));
  if ( nullptr == channel ){
    error_log << meteo::msglog::kServiceConnectFailed.arg(serviceTitle(settings::proto::ServiceCode::kSprinf))
                 .arg(serviceAddress(settings::proto::ServiceCode::kSprinf));
    return false;
  }

  meteo::sprinf::CircleRegionRequest req;
  req.mutable_circle()->set_radius_meter(25000); //Загружаем станции в радиусе 25км, просто есть шанс того, что следующий запрос будет по станции неподалеку
  req.mutable_circle()->mutable_center()->set_lat_radian(point.lat());
  req.mutable_circle()->mutable_center()->set_lon_radian(point.lon());

  auto resp = std::unique_ptr<meteo::sprinf::StationsWithDistance>(channel->remoteCall(&meteo::sprinf::SprinfService::GetStationsByPosition, req, 30000));

  if ( nullptr == resp || false == resp->result() ) {
    error_log << msglog::kServiceRequestFailedErr;
    return false;
  }

  for ( auto station: resp->station() ){
    auto posProto = station.main().position();
    meteo::GeoPointPb point;
    point.set_lat_radian(posProto.lat_radian());
    point.set_lon_radian(posProto.lon_radian());
    point.set_height_meters(posProto.height_meters());
    meteo::GeoPoint stationPos = meteo::pbgeopoint2geopoint(point);
    cache.insert(stationPos,station.main());
  }

  if ( true == cache.contains(point) ){
    auto result = cache[point];
    station->CopyFrom(result);
    return true;
  }
  return false;
}

bool json2meteodata(const DbiEntry *json, TMeteoData *md)
{
  QTime ttt; ttt.elapsed();
  auto add_param = []( Document* doc, TMeteoData* md, QList<int64_t>* tlglist ) {
    QString descr =  doc->valueString("descrname");
    if ( 0 == descr.size() ) {
      return;
    }
    double value = doc->valueDouble("value");
    int qual = doc->valueInt32("quality");
    QString code = doc->valueString("code");
    TMeteoParam mp( code, value, static_cast<control::QualityControl>(qual) );
    if ( true == doc->hasField("tlgid") ) {
      int64_t tlgid = doc->valueInt64("tlgid");
      if ( false == tlglist->contains(tlgid) ) {
        tlglist->append(tlgid);
      }
    }
    md->add( descr, mp );
  };
  QString station = json->valueString("station");
  if (true == json->hasField("station_info.name_ru")){
    QString station_name = json->valueString("station_info.name_ru");
    md->set(TMeteoDescriptor::instance()->descriptor("station"),
        TMeteoParam(station_name, 0, control::RIGHT));
  }
  int station_type = json->valueInt32("station_type");
  md->setStation(station);
  md->set(TMeteoDescriptor::instance()->descriptor("station_type"), TMeteoParam(QString::number(station_type), station_type, control::RIGHT));
  GeoPoint coords = json->valueGeo("location");
  int level_type = json->valueInt32("level_type");
  md->set(TMeteoDescriptor::instance()->descriptor("level_type"),
    TMeteoParam(QString::number(level_type), level_type, control::RIGHT));
  int dataType = json->valueInt32("data_type");
  md->set(TMeteoDescriptor::instance()->descriptor("category"),
         TMeteoParam(QString::number(dataType), dataType, control::RIGHT));
  if ( true == json->hasField("oid") ) {
    QString iddb = json->valueOid("oid").simplified();
    TMeteoParam param(iddb, -1, control::RIGHT);
    md->add("text", param);
  }

  md->setCoord( coords.latDeg(), coords.lonDeg(), coords.alt() );
  md->setDateTime( json->valueDt("dt") );
  bool ok = false;
  Array obs = json->valueArray("param", &ok);
  if ( false == ok ) {
    error_log << json->jsonExtendedString();
  }
  while ( true == obs.next() ) {
    Document param;
    if ( false == obs.valueDocument(&param) ) {
      continue;
    }
    QList<int64_t> tlglist;
    add_param( &param, md, &tlglist );
    for ( auto t : tlglist ) {
      TMeteoParam param( QString::number(t), -1, control::RIGHT);
      md->add("ii_tlg", param);
    }
  }
  return true;
}

bool json2meteoproto(const DbiEntry *json, surf::MeteoDataProto *md, bool as_tree )
{
//  static const QStringList clouds = QStringList() << "C" << "Cn" << "CH" << "CL" << "CM" << "h" << "N" << "Nh";
  QTime ttt; ttt.elapsed();
  auto add_param = []( Document* doc,
                       surf::MeteoDataProto* proto,
                       QList<int64_t>* tlglist,
                       QMap<QString, surf::MeteoDataProto*>* childs,
                       bool as_tree ) {
    QString descr =  doc->valueString("descrname");
    if ( 0 == descr.size() ) {
      return;
    }
    double value = doc->valueDouble("value");
    int qual = doc->valueInt32("quality");
    QString code = doc->valueString("code");
    QString dt_beg = doc->valueDt("dt_beg").toString(Qt::ISODate);
    QString dt_end = doc->valueDt("dt_end").toString(Qt::ISODate);
    QString uuid = doc->valueString("uuid");

    surf::MeteoParamProto* param = nullptr;//proto->add_param();TODO
    surf::MeteoDataProto* child = nullptr;
    
    if ( true == as_tree && 0 < uuid.length() ) {
      if ( true == childs->contains(uuid) ) {
        child = (*childs)[uuid];
      }
      else {
        child = proto->add_child();
        childs->insert( uuid, child );
      }
      param = child->add_param();
    }
    else {
      param = proto->add_param();
    }

    param->set_code( code.toStdString() );
    param->set_value( value );
    param->set_quality( qual );
    param->set_descrname( descr.toStdString() );
    if ( true == doc->hasField("tlgid") ) {
      int64_t tlgid = doc->valueInt64("tlgid");
      if ( false == tlglist->contains(tlgid) ) {
        tlglist->append(tlgid);
      }
    }
    if (( proto->dt() != dt_beg.toStdString() )
         || ( proto->dt() != dt_end.toStdString() )) {
      param->set_dt_beg( dt_beg.toStdString() );
      param->set_dt_end( dt_end.toStdString() );
    }
    if ( true == doc->hasField("uuid") ) {
      param->set_uuid( doc->valueString("uuid").toStdString() );
    }
    if ( true == doc->hasField("cor_number") ) {
      int cornumber = doc->valueInt32("cor_number");
      if ( false == proto->has_cor_number() || proto->cor_number() < cornumber ) {
        proto->set_cor_number(cornumber);
      }
    }
  };
  QString station = json->valueString("station");
  int station_type = json->valueInt32("station_type");
  md->mutable_station_info()->set_cccc(station.toStdString());
  md->mutable_station_info()->set_station(station.toStdString());
  md->mutable_station_info()->set_station_type(station_type);
  if (true == json->hasField("station_info")) {
    const Document& info = json->valueDocument("station_info");
    if (true == info.hasField("country_ru")){   //MONGODB
      QString countryRu = info.valueString("country_ru");
      md->mutable_station_info()->set_country_rus(countryRu.toStdString());
    }
    if (true == info.hasField("country_en")){
      QString countryEn = info.valueString("country_en");
      md->mutable_station_info()->set_country_eng(countryEn.toStdString());
    }
    if (true == info.hasField("name_ru")){
      QString nameRu = info.valueString("name_ru");
      md->mutable_station_info()->set_name_rus(nameRu.toStdString());
    }
    if (true == info.hasField("name")){
      QString nameEng = info.valueString("name");
      md->mutable_station_info()->set_name_eng(nameEng.toStdString());
    }
    if (true == info.hasField("country")){
      double countryCode = info.valueDouble("country");
      md->mutable_station_info()->set_country_code(countryCode);
    }
    if ( true == info.hasField("ru") ) {     //POSTGRES
      QString nameRu = info.valueString("ru");
      md->mutable_station_info()->set_name_rus(nameRu.toStdString());
    }
    if (true == info.hasField("international")){
      QString nameEng = info.valueString("international");
      md->mutable_station_info()->set_name_eng(nameEng.toStdString());
    }
    if ( false == md->station_info().has_name_rus()
      &&  false == md->station_info().has_name_eng() ) {
      md->mutable_station_info()->set_name_rus( station.toStdString() );
    }
  }
  md->set_dt( json->valueDt("dt").toString(Qt::ISODate).toStdString() );
  GeoPoint coords = json->valueGeo("location");
  md->mutable_gp()->set_la( coords.lat() );
  md->mutable_gp()->set_lo( coords.lon() );

  bool ok = false;
  Array obs = json->valueArray("param", &ok);
  if ( false == ok ) {
    error_log << json->jsonExtendedString();
  }
  QList<int64_t> tlglist;
  QMap<QString, surf::MeteoDataProto*> childs;
  while ( true == obs.next() ) {
    Document param;
    if ( false == obs.valueDocument(&param) ) {
      continue;
    }
    add_param( &param, md, &tlglist, &childs, as_tree );
  }
  for ( auto id : tlglist ) {
    md->add_tlgid(id);
  }
//  removeComplexDuplication(md); TODO не могу вспомнить, для какого случая это сделано. Поэтому, пусть не вызывается
  return true;
}

surf::MeteoDataProto meteodata2proto( const TMeteoData& meteodata ) {
  auto md = TMeteoDescriptor::instance();
  surf::MeteoDataProto proto;
  proto.mutable_station_info()->set_station( md->stationIdentificator(meteodata).toStdString() );
  proto.mutable_station_info()->set_name_eng( md->stationName(meteodata).toStdString() );
  proto.set_dt( md->dateTime(meteodata).toString(Qt::ISODate).toStdString() );

  int lvltype;
  float levelLo;
  float levelHi;
  md->findVertic( meteodata, &lvltype, &levelLo, &levelHi);
  proto.set_level(levelLo);
  proto.set_level_type(lvltype);
  auto params = meteodata.allByNames();
  for ( auto it = params.constBegin(), end = params.constEnd(); it != end; ++it ) {
    auto name = it.key();
    auto param = it.value();
    auto protoparam = proto.add_param();
    protoparam->set_descrname( name.toStdString() );
    protoparam->set_code( param.code().toStdString() );
    protoparam->set_value( param.value() );
    protoparam->set_quality( param.quality() );
  }
  auto childs = meteodata.childs();
  for ( auto ch : childs ) {
    auto chproto = proto.add_child();
    chproto->CopyFrom( meteodata2proto(*ch) );
  }
  return proto;
}

void removeComplexDuplication( surf::MeteoDataProto* md )
{
  typedef QMap< std::string, QMap< std::string,  surf::MeteoParamProto> > compl_data;
  compl_data group_list;
  for ( auto param : md->param() ) {
    if ( false == param.has_uuid() ) {
      continue;
    }
    group_list[param.uuid()].insert( param.descrname(),  param );
  }
  auto isEqual = []( compl_data::iterator it, compl_data::iterator nextit ) {
    auto left = it.value();
    auto right = nextit.value();
    for ( auto leftit = left.begin(); leftit != left.end();  ++leftit ) {
      if ( false == right.contains( leftit.key() ) ) {
        return false;
      }
      if ( false == MnMath::isEqual( leftit.value().value(), right[ leftit.key() ].value() ) ) {
        return false;
      }
    }
    return true;
  };
  for ( auto it = group_list.begin(), end = group_list.end(); it != end; ++it ) {
    for ( auto nextit = it + 1; nextit != end; ) {
      if ( true == isEqual( it, nextit ) ) {
        auto key = it.key();
        auto erasekey = nextit.key();
        for ( auto protoit = md->mutable_param()->begin(); protoit != md->mutable_param()->end(); ++protoit ) {
          if ( (*protoit).uuid() == erasekey ) {
            md->mutable_param()->erase(protoit);
            protoit = md->mutable_param()->begin();
          }
        }
        nextit = group_list.erase(nextit);
      }
      else {
        ++nextit;
      }
    }
  }
}

void loadBufrTables( const QStringList& codes )
{
  return Global::instance()->loadBufrTables(codes);
}


bool hasBufrTable( const QString& code )
{
  return Global::instance()->hasBufrTable(code);
}

QMap< int, QString > bufrTable( const QString& code )
{
  return Global::instance()->bufrTable(code);
}

QString bufrValueDescription( const QString& code, int value )
{
  return Global::instance()->bufrValueDescription( code, value );
}

float pressureOnStationLevel(float h, float P, float T)
{
  float pressure = -9999.0;
  if ( T == -9999.0 ) { // Упрощённая формула без учёта температуры
     pressure = P * std::pow(0.87, h);
     return pressure;
  }
  else {
    float M = 0.029; // молярная масса сухого воздуха
    float g = 9.81; //ускорение свободного падения
    float R = 8.31; //универсальная газовая постоянная
    T +=273; //Перевод Цельсия в Кельвины
    pressure = P * std::exp(-((M*g*h)/(R*T)));
  }
  return pressure;
}

float pressureOnOceanLevel(float h, float P, float T)
{
  float pressure = -9999.0;
  if ( T == -9999.0 ) { // Упрощённая формула без учёта температуры
     pressure = P / std::pow(0.87, h);
     return pressure;
  }
  else {
    float M = 0.029; // молярная масса сухого воздуха
    float g = 9.81; //ускорение свободного падения
    float R = 8.31; //универсальная газовая постоянная
    T +=273; //Перевод Цельсия в Кельвины
    pressure = P / std::exp(-((M*g*h)/(R*T)));
  }
  return pressure;
}

QMap< QString, QString > meteoparamProps( descr_t descr )
{
  auto md = TMeteoDescriptor::instance();
  QString name = md->name(descr);

  return global::meteoparamPropsByName(name);
}

QMap< QString, QString > meteoparamPropsByName( const QString& descrname )
{
  QMap< QString, QString > props;
  auto md = TMeteoDescriptor::instance();
  props["name"] = descrname;
  props["descriptor"] = QString::number(md->descriptor(descrname));
  meteodescr::Property p;
  if ( true == md->property( descrname, &p ) ) {
    props["units"] = p.units;
    props["unitsRu"] = p.unitsRu;
    props["description"] = p.description;
  }

  return props;
}

void setLogHandler()
{
  TLog::setMsgHandler(logoutToDb);
}

}

internal::Global::Global()
{
}

internal::Global::~Global()
{
}

ConnectProp internal::Global::dbConf(const QString& connectionName) const
{
  settings::DbConnection c = ::meteo::gSettings()->dbConn(connectionName);
  ConnectProp cc(QString::fromStdString(c.host()),
                     QString::fromStdString(c.name()),
                     QString::fromStdString(c.login()),
                     QString::fromStdString(c.password()),
                     c.port(),
                     QString::fromStdString(c.userdb()));
  cc.setDriver(c.dbdriver());
  return cc;
}

ConnectProp internal::Global::mongodbConfForecast() const
{ return dbConf("forecast"); }

ConnectProp internal::Global::dbConfDjango() const
{ return dbConf(meteo::global::dbnames::dbDjango); }

ConnectProp internal::Global::mongodbConfTelegrams() const
{ return dbConf("telegramsdb"); }

ConnectProp internal::Global::mongodbConfMeteo() const
{ return dbConf("meteodb"); }

ConnectProp internal::Global::mongodbConfInter() const
{ return dbConf("interdb"); }

ConnectProp internal::Global::mongodbConfDocument() const
{ return dbConf("documentdb"); }

ConnectProp internal::Global::mongodbConfSprinf() const
{ return dbConf("sprinfdb"); } ///TODO: изменить, когда у спринфа появится своя база

ConnectProp internal::Global::mongodbConfClimat() const
{ return dbConf("climatdb"); }

ConnectProp internal::Global::mongodbConfJournal() const
{
  return dbConf("journaldb");
}

ConnectProp internal::Global::mongodbConfObanal() const
{
  return dbConf("meteodb");
}

ConnectProp internal::Global::mongodbConfUser() const
{
  return dbConf("usersdb");
}

Dbi* internal::Global::dbForecast()
{
  ConnectProp p = mongodbConfForecast();
  Dbi* nosql = createDatabase(p);
  if (nullptr == nosql || false == nosql->connect() ) {
    error_log << QObject::tr("Не удалось подключиться к БД = '%1'")
      .arg( p.name() );
    delete nosql;
    nosql = nullptr;
  }
  return nosql;
}

Dbi* internal::Global::dbTelegram()
{
  ConnectProp p = mongodbConfTelegrams();
  Dbi* nosql = createDatabase(p);
  if (nullptr == nosql || false == nosql->connect() ) {
    error_log << QObject::tr("Не удалось подключиться к БД = '%1'")
      .arg( p.name() );
    delete nosql;
    nosql = nullptr;
  }
  return nosql;
}

Dbi* internal::Global::dbMeteo()
{
  ConnectProp p = mongodbConfMeteo();
  Dbi* nosql = createDatabase(p);

  if (nullptr == nosql || false == nosql->connect() ) {
    error_log << QObject::tr("Не удалось подключиться к БД = '%1'")
      .arg( p.name() );
    delete nosql;
    nosql = nullptr;
  }
  return nosql;
}

Dbi* internal::Global::dbJournal()
{
  ConnectProp p = mongodbConfJournal();
  Dbi* nosql = createDatabase(p);

  if (nullptr == nosql || false == nosql->connect() ) {
    error_log << QObject::tr("Не удалось подключиться к БД = '%1'")
      .arg( p.name() );
    delete nosql;
    nosql = nullptr;
  }
  return nosql;
}

Dbi* internal::Global::dbObanal()
{
  ConnectProp p = mongodbConfMeteo();
  Dbi* nosql = createDatabase(p);
  if (nullptr == nosql || false == nosql->connect() ) {
    error_log << QObject::tr("Не удалось подключиться к БД = '%1'")
      .arg( p.name() );
    delete nosql;
    nosql = nullptr;
  }
  return nosql;
}

Dbi* internal::Global::dbInter()
{
  ConnectProp p = mongodbConfInter();
  Dbi* nosql = createDatabase(p);
  if (nullptr == nosql || false == nosql->connect() ) {
    error_log << QObject::tr("Не удалось подключиться к БД = '%1'")
      .arg( p.name() );
    delete nosql;
    nosql = nullptr;
  }
  return nosql;
}

Dbi* internal::Global::dbUsers()
{
  ConnectProp p = mongodbConfUser();
  Dbi* nosql = createDatabase(p);
  if (nullptr == nosql || false == nosql->connect() ) {
    error_log << QObject::tr("Не удалось подключиться к БД = '%1'")
      .arg( p.name() );
    delete nosql;
    nosql = nullptr;
  }
  return nosql;
}

Dbi* internal::Global::dbDocument()
{
  ConnectProp p = mongodbConfDocument();
  Dbi* nosql = createDatabase(p);

  if (nullptr == nosql || false == nosql->connect() ) {
    error_log << QObject::tr("Не удалось подключиться к БД = '%1'")
      .arg( p.name() );
    delete nosql;
    nosql = nullptr;
  }
  return nosql;
}

Dbi* internal::Global::dbSprinf()
{
  ConnectProp p = mongodbConfSprinf();

  Dbi* nosql = createDatabase(p);
  if (nullptr == nosql || false == nosql->connect() ) {
    error_log << QObject::tr("Не удалось подключиться к БД = '%1'")
      .arg( p.name() );
    delete nosql;
    nosql = nullptr;
  }
  return nosql;
}

Dbi* internal::Global::db(const ConnectProp& prop)
{
  Dbi* db = createDatabase(prop);

  if (nullptr == db || false == db->connect() ){
    error_log << meteo::msglog::kDbConnectFailed.arg(prop.name());
    delete db;
    db = nullptr;
  }

  return db;
}

Dbi* internal::Global::dbClimat()
{
  ConnectProp p = mongodbConfClimat();
  Dbi* nosql = createDatabase(p);

  if (nullptr == nosql || false == nosql->connect() ) {
    error_log << QObject::tr("Не удалось подключиться к БД = '%1'")
      .arg( p.name() );
    delete nosql;
    nosql = nullptr;
  }
  return nosql;
}


Psql* internal::Global::dbDjango()
{ return reinterpret_cast<Psql*>(dbConn_.get(kDbDjango, dbConfDjango())); }

DbiQuery* internal::Global::dbqueryByName( const ConnectProp& params, const QString& queryName ) const
{
  auto queries = ::meteo::global::kSqlQueries;
  switch ( params.driver() ) {
    case settings::kMongo:
      queries = ::meteo::global::kMongoQueriesNew;
      break;
    case settings::kPsql:
    default:
      break;
  }
  return dbquery( params, queries[queryName] );
}

DbiQuery* internal::Global::dbquery( const ConnectProp& params, const QString q ) const
{
  DbiQuery* query = nullptr;
  switch ( params.driver() ) {
    case settings::kMongo:
      query = new NosqlQuery();
      break;
    case settings::kPsql:
    default:
      query = new PsqlQuery();
      break;
  }
  if ( nullptr != query && false == q.isEmpty() ) {
    query->setQuery(q);
  }
  return query;
}

Dbi* internal::Global::createDatabase( const ConnectProp& params ) const
{
  switch ( params.driver() ) {
    case settings::kMongo:
      return nullptr;
      break;
    case settings::kPsql:
    default:
      return new Psql(params);
      break;
  }
}

settings::Location internal::Global::punkt() const
{ return gSettings()->location(); }

QString internal::Global::currentUserLogin() const
{

  return meteo::TUserSettings::instance()->getCurrentUserLogin();
}

QString internal::Global::currentUserShortName() const {
  return meteo::TUserSettings::instance()->getCurrentUserShortName();
}


QString internal::Global::currentUserName() const
{
  return meteo::TUserSettings::instance()->getCurrentUserName();
}

QString internal::Global::rank(int code) const
{
  return meteo::TUserSettings::instance()->getRank(code);
}

QString internal::Global::currentUserRank() const
{
  return meteo::TUserSettings::instance()->getCurrentUserRank();
}

QString internal::Global::currentUserSurname() const
{
  return meteo::TUserSettings::instance()->getCurrentUserSurname();
}

QString internal::Global::currentUserDepartment() const
{
  return meteo::TUserSettings::instance()->getCurrentUserDepartment();
}

QString internal::Global::wmoId() const
{ return ::meteo::gSettings()->wmoId(); }

QString internal::Global::hmsId() const
{ return ::meteo::gSettings()->hmsId(); }

QString internal::Global::serviceTitle(settings::proto::ServiceCode code, bool* ok) const
{ return QString::fromUtf8(service(code, ok).title().data()); }

QString internal::Global::serviceAddress(settings::proto::ServiceCode code, bool* ok) const
{ return ::meteo::gSettings()->serviceAddress(code,ok); }

rpc::Channel* internal::Global::serviceChannel(settings::proto::ServiceCode code) const
{
  bool ok = false;
  QString addr = serviceAddress(code,&ok);
  if ( false == ok ) {
    error_log << QObject::tr("Не удалось получить адрес сервиса '%1'").arg(serviceTitle(code));
    return nullptr;
  }
  rpc::Channel* ch = rpc::Channel::connect(addr);
  if ( nullptr == ch ) {
    error_log << QObject::tr("Не удалось установить соединение со службой '%1' по адресу %2")
                 .arg(serviceTitle(code))
                 .arg(addr);
  }
  return ch;
}

rpc::Channel* internal::Global::serviceChannel(const QString& addr) const
{

  rpc::Channel* ch = rpc::Channel::connect(addr);
  if ( nullptr == ch ) {
    error_log << QObject::tr("Не удалось установить соединение со службой по адресу %1")
                 .arg(addr);
  }
  return ch;
}

int internal::Global::serviceTimeout(settings::proto::ServiceCode code) const
{ return ::meteo::gSettings()->serviceTimeout(code); }

settings::proto::Service internal::Global::service(settings::proto::ServiceCode code, bool* ok) const
{ return ::meteo::gSettings()->service(code,ok); }

void internal::Global::addGeoLoader( const QString& loader )
{
  if ( true == geoloaders_.contains(loader) ) {
    return;
  }
  geoloaders_.append(loader);
}

void internal::Global::loadBufrTables( const QStringList& codes )
{
  QStringList locodes = codes;
  for ( auto it = bufrtables_.begin(), end = bufrtables_.end(); it != end; ++it ) {
    QString code = it.key();
    locodes.removeAll(code);
  }
  if ( 0 == locodes.size() ) {
    return;
  }
  meteo::rpc::Channel* ch = meteo::global::serviceChannel( meteo::settings::proto::kSprinf );
  if ( nullptr == ch ) {
    error_log << QObject::tr("Не удалось подключиться к сервису справки");
    return;
  }
  sprinf::BufrParametersTableRequest request;
  for ( auto c : codes ) {
    request.add_table_code( c.toStdString() );
  }

  meteo::sprinf::BufrParametersTables* resp = ch->remoteCall(&meteo::sprinf::SprinfService::GetBufrParametersTables, request, 10000);
  delete ch;
  if ( nullptr == resp ) {
    error_log << QObject::tr("Ответ с описаниями значений параметров от сервиса справки не получен");
    return;
  }

  if ( false == resp->result() ) {
    if ( 0 != resp->error_message().size() ) {
      warning_log << QObject::tr("Ответ от сервиса справки с описаниями значений параметров помечен как ошибочный. Описание ошибки = %1")
        .arg( QString::fromStdString( resp->error_message() ) );
    }
    else {
      warning_log << QObject::tr("Ответ от сервиса справки с описаниями значений параметров помечен как ошибочный, но диагностическое сообщение отсутствует");
    }
  }
  for ( auto t : resp->table() ) {
    QString code = QString::fromStdString( t.table_code() );
    QMap< int, QString >& params = bufrtables_[code];
    for ( auto p : t.parameter() ) {
      params.insert( p.code(), QString::fromStdString( p.name() ) );
    }
  }
  delete resp;
}

bool internal::Global::hasBufrTable( const QString& code ) const
{
  return bufrtables_.contains(code);
}

QMap< int, QString > internal::Global::bufrTable( const QString& code ) const
{
  QMap< int, QString > table;
  table = bufrtables_[code];
  return table;
}

QString internal::Global::bufrValueDescription( const QString& code, int value ) const
{
  QString locode = code;
  while ( 5 > locode.length() ) {
    locode = "0" + locode;
  }
  if ( false == bufrtables_.contains(locode) ) {
    return QString();
  }
  return bufrtables_[locode].value( value, "" );
}


}

namespace meteo {

bool dbusSendNotification(const QString &title, const QString &message, const QString &icon, int timeout, const QString &buttonName, const QString &buttonText){
  QStringList button;
  button << buttonName << buttonText;
  return dbusSendNotification(title, message, icon, timeout, button);
}

bool dbusSendNotification(const QString &title, const QString &message, const QString &icon, int timeout, const QHash<QString, QString> &buttons){
  QStringList allButtons;
  for (const QString& key : buttons.keys()){
    const QString &value = buttons[key];
    allButtons << key << value;
  }
  return dbusSendNotification(title, message, icon, timeout, allButtons);
}

bool dbusSendNotification(const QString &title, const QString &message, const QString &icon, int timeout, const QStringList &buttons) {

  QDBusInterface soobw("org.freedesktop.Notifications", "/org/freedesktop/Notifications",
                       "org.freedesktop.Notifications", QDBusConnection::sessionBus());
  QVariantList args;
  args << QString(MnCommon::applicationTextName());
  args << QVariant(QVariant::UInt);
  args << QVariant(icon);
  args << QString(title);
  args << QString(message);
  args << buttons;

  args << (QVariantMap());
  args << timeout;
  soobw.callWithArgumentList(QDBus::AutoDetect, "Notify", args);
  return true;
}

void dbusConnect(QObject *receiver, const char *slot){
  QDBusConnection conn = QDBusConnection::sessionBus();;
  conn.connect( "org.freedesktop.Notifications", "/org/freedesktop/Notifications",
                "org.freedesktop.Notifications", "ActionInvoked", receiver, slot);
}

void dbusDisconnect(QObject *receiver, const char* slot){{
    QDBusConnection conn = QDBusConnection::sessionBus();;
    conn.disconnect( "org.freedesktop.Notifications", "/org/freedesktop/Notifications",
                  "org.freedesktop.Notifications", "ActionInvoked", receiver, slot);
  }


}

}



