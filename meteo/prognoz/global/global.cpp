#include "global.h"

#include <qhostinfo.h>

#include <meteo/commons/proto/msgsettings.pb.h>

#include <meteo/prognoz/settings/prognozsettings.h>

namespace meteo {

ConnectProp PrognozGlobal::dbConfObanal()
{
  settings::DbConnection c = prognoz::Settings::instance()->dbConn("obanal");
  return ConnectProp(QString::fromStdString(c.host()),
                     QString::fromStdString(c.name()),
                     QString::fromStdString(c.login()),
                     "",
                     QString::number(c.port()));
}

ConnectProp PrognozGlobal::dbConfTelegram()
{
  settings::DbConnection c = prognoz::Settings::instance()->dbConn("telegram");
  return ConnectProp(QString::fromStdString(c.host()),
                     QString::fromStdString(c.name()),
                     QString::fromStdString(c.login()),
                     "",
                     QString::number(c.port()));
}

ConnectProp PrognozGlobal::dbConfMeteo()
{
  settings::DbConnection c = prognoz::Settings::instance()->dbConn("meteo");
  return ConnectProp(QString::fromStdString(c.host()),
                     QString::fromStdString(c.name()),
                     QString::fromStdString(c.login()),
                     "",
                     QString::number(c.port()));
}

ConnectProp PrognozGlobal::mongodbConfTelegrams() const
{
  settings::DbConnection c = prognoz::Settings::instance()->dbConn("telegramsdb");
  return ConnectProp(QString::fromStdString(c.host()),
                     QString::fromStdString(c.name()),
                     QString::fromStdString(c.login()),
                     "",
                     QString::number(c.port()));
}

ConnectProp PrognozGlobal::mongodbConfMeteo() const
{
  settings::DbConnection c = prognoz::Settings::instance()->dbConn("meteodb");
  return ConnectProp(QString::fromStdString(c.host()),
                     QString::fromStdString(c.name()),
                     QString::fromStdString(c.login()),
                     "",
                     QString::number(c.port()));
}

ConnectProp PrognozGlobal::mongodbConfSprinf() const
{
  settings::DbConnection c = prognoz::Settings::instance()->dbConn("sprinfdb");
  return ConnectProp(QString::fromStdString(c.host()),
                     QString::fromStdString(c.name()),
                     QString::fromStdString(c.login()),
                     "",
                     QString::number(c.port()));
}

ConnectProp PrognozGlobal::mongodbConfObanal() const
{
  settings::DbConnection c = prognoz::Settings::instance()->dbConn("obanaldb");
  return ConnectProp(QString::fromStdString(c.host()),
                     QString::fromStdString(c.name()),
                     QString::fromStdString(c.login()),
                     "",
                     QString::number(c.port()));
}

int PrognozGlobal::serviceTimeout(settings::proto::ServiceCode code) const
{ return prognoz::Settings::instance()->serviceTimeout(code); }

settings::proto::Service PrognozGlobal::service(settings::proto::ServiceCode code, bool* ok) const
{ return prognoz::Settings::instance()->service( code, ok ); }

rpc::Address PrognozGlobal::serviceAddress(settings::proto::ServiceCode code, bool* ok) const
{ return prognoz::Settings::instance()->serviceAddress( code, ok ); }

rpc::Channel*PrognozGlobal::serviceChannel(settings::proto::ServiceCode code) const
{
  QString serviceName = QString::fromStdString(settings::proto::ServiceCode_Name(code)).remove(0,1).toLower();

  bool ok = false;
  rpc::Address addr = serviceAddress( code, &ok );
  if ( false == ok ) {
    error_log << QObject::tr("Не удалось получить адрес сервиса %1")
                 .arg(serviceName);
    return 0;
  }
  rpc::Channel* ch = rpc::Channel::connect(addr);
  if ( 0 == ch ) {
    error_log << QObject::tr("Не удалось установить соединение со службой %1 по адресу %2")
                 .arg( serviceName )
                 .arg( addr.asString() );
  }
  return ch;
}

rpc::TController* PrognozGlobal::serviceController(settings::proto::ServiceCode code) const
{
  QString serviceName = QString::fromStdString(settings::proto::ServiceCode_Name(code)).remove(0,1).toLower();

  bool ok = false;
  rpc::Address addr = serviceAddress( code, &ok );
  if ( false == ok ) {
    error_log << QObject::tr("Не удалось получить адрес сервиса %1")
                 .arg(serviceName);
    return 0;
  }
  return ::meteo::global::serviceController(addr);
}

} // meteo

void prognoz::global::logoutToDb(tlog::Priority priority, const QString &facility, const QString &fileName, int line, const QString &m)
{
  static const QMap<QString, QString> kModuleNames = {
    { "vizdoc"        , QObject::tr("ПК визуализации и документирования"                              ) },
    { "recgmi"        , QObject::tr("ПК приема (передачи) ГМИ"                                        ) },
    { "sortgmi"       , QObject::tr("ПК сортировки ГМИ"                                               ) },
    { "contrdiag"     , QObject::tr("ПК самоконтроля и диагностики"                                   ) },
    { "settings"      , QObject::tr("ПК настройки"                                                    ) },
    { "codecontrol"   , QObject::tr("ПК раскодирования и контроля ГМИ"                                ) },
    { "obanal"        , QObject::tr("ПК объективного анализа"                                         ) },
    { "prepare"       , QObject::tr("ПК подготовки исходных данных"                                   ) },
    { "dbtask"        , QObject::tr("ПК обслуживания базы данных"                                     ) },
    { "dbconn"        , QObject::tr("ПК взаимодействия с базой данных"                                ) },
    { "applied"       , QObject::tr("ПК прикладных задач"                                             ) },
    { "vko"           , QObject::tr("ПК автоматизированного составления военно-климатических описаний") },
    { "rpc"           , QObject::tr("Удаленный вызов процедур"                                        ) },
    { "journal"       , QObject::tr("ПК журналирования"                                               ) },
    { "wrb"           , QObject::tr("ПК прикладных задач"                                             ) },
    { "puansoneditor" , QObject::tr("ПК визуализации и документирования"                              ) },
    { "global"        , QObject::tr("ПК журналирования"                                               ) },
    { "forecast"      , QObject::tr("ПК прогнозирования"                                              ) },
    { "prototext"     , QObject::tr("ПК настройки"                                                    ) },
    { "oceandiag"     , QObject::tr("ПК визуализации и документирования"                              ) },
    { "procview"      , QObject::tr("ПК самоконтроля и диагностики"                                   ) },
    { "monitor"       , QObject::tr("ПК самоконтроля и диагностики"                                   ) },
    { "procident"     , QObject::tr("ПК визуализации и документирования"                              ) },
    { "aerodiag"      , QObject::tr("ПК прикладных задач"                                             ) },
    { "customui"      , QObject::tr("ПК визуализации и документирования"                              ) },
    { "forecastwidget", QObject::tr("ПК прогнозирования"                                              ) },
    { "decodeviewer"  , QObject::tr("ПК визуализации и документирования"                              ) },
    { "nabludenia"    , QObject::tr("ПК визуализации и документирования"                              ) },
    { "tlgeditor"     , QObject::tr("ПК визуализации и документирования"                              ) },
    { "settings"      , QObject::tr("ПК настройки"                                                    ) },
    { "sprinf"        , QObject::tr("ПК взаимодействия с базой данных"                                ) },
    { "map"           , QObject::tr("ПК визуализации и документирования"                              ) },
    { "cron"          , QObject::tr("ПК самоконтроля и диагностики"                                   ) },
    { "planner"       , QObject::tr("ПК журналирования"                                               ) },
    { "verticalcut"   , QObject::tr("ПК прикладных задач"                                             ) },
    { "msgviewer"     , QObject::tr("ПК визуализации и документирования"                              ) },
    { "radar"         , QObject::tr("ПК визуализации и документирования"                              ) }
  };

  static const QMap<tlog::Priority, QString> kPriorityName = {
    { tlog::kNone,      "DD" },
    { tlog::kDebug,     "DD" },
    { tlog::kInfo,      "II" },
    { tlog::kWarning,   "WW" },
    { tlog::kError,     "EE" },
    { tlog::kCritical,  "CC" }
  };

  fMsgHandler old = TLog::setMsgHandler(meteo::global::stdOutWithTime);
  QString moduleName = kModuleNames[facility];

  QString sender = facility;

  if(false == moduleName.isEmpty()) {
    sender = moduleName;
  }
  meteo::global::stdOutWithTime(priority, sender, fileName, line, m);

  /*
  meteo::settings::RegLevel reg;
  switch(priority) {
  case tlog::kNone:
  case tlog::kDebug:
    reg = meteo::prognoz::Settings::instance()->msgSettings().debug();
    break;
  case tlog::kInfo:
    reg = meteo::prognoz::Settings::instance()->msgSettings().info();
    break;
  case tlog::kWarning:
    reg = meteo::prognoz::Settings::instance()->msgSettings().warning();
    break;
  case tlog::kError:
    reg = meteo::prognoz::Settings::instance()->msgSettings().error();
    break;
  case tlog::kCritical:
    reg = meteo::prognoz::Settings::instance()->msgSettings().critical();
    break;
  default:
    break;
  }

  if(reg != meteo::settings::kNone) {
    NS_PGBase* db = meteo::global::dbJournal();

    QString username = ::meteo::Global::instance()->currentUserShortName();
    if(username.isEmpty()) {
      username = meteo::Global::instance()->currentUserLogin();
    }
    if(username.isEmpty()) {
      username = "unknown";
    }

    QString msg = m;
    msg.replace('\'', "''");

    QString q = QString("SELECT logmessage( '%1', '%2', '%3', '%4', '%5', '%6', %7)")
                    .arg(kPriorityName[priority])
                    .arg(sender)
                    .arg(username)
                    .arg(MnCommon::selfIpAddress())
                    .arg(QHostInfo::localHostName())
                    .arg(msg)
                    .arg(0);

    TSqlQuery query(q, db);

    if(false == query.exec()) {
      error_log << QObject::tr("Не удалось выполнить запрос %1").arg(q);
    }
  }
  */
  TLog::setMsgHandler(old);
}
