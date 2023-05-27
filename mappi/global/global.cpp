#include "global.h"


//#include <commons/textproto/pbtools.h>
//#include <commons/container/thash.h>

// #include <sql/nspgbase/ns_pgbase.h>
// #include <sql/nspgbase/tsqlquery.h>

//#include <commons/funcs/selfaddress.h>

//#include <meteo/commons/ui/map/loader.h>

#include <mappi/settings/mappisettings.h>
// #include <meteo/ukaz/settings/tadminsettings.h>

template<> ::meteo::internal::MappiGlobal* meteo::MappiGlobal::_instance = nullptr;

namespace meteo {

namespace global {

Dbi* dbMappi() { return ::meteo::MappiGlobal::instance()->dbMappi(); }
ConnectProp dbConfMappi() { return ::meteo::MappiGlobal::instance()->dbConfMappi(); }


// proto::Location ukazpunkt()
// {
//   return ukaz::Settings::instance()->locSettings();
// }
QString findWeatherFile(const QString& tleDir, const QDateTime& timeCompare )
{
  QRegExp regexp("weather_[0-9]{8,8}.txt");
  QDir varDir(tleDir);
  QStringList fileList=varDir.entryList();
  QString weather;
  qint64 minsecs = timeCompare.toMSecsSinceEpoch();
  QDateTime timeWeather;
  timeWeather.setMSecsSinceEpoch(0);
  QString buftime;
  foreach (QString file, fileList) {
    if ( regexp.exactMatch(file) ) {
      buftime.clear();
      for (int i = 8; i<12;i++) {
        buftime.append(file[i]);
      }
      timeWeather = timeWeather.addYears((buftime.toInt()-timeWeather.date().year()));
      buftime.clear();
      for (int i = 12; i<14;i++) {
        buftime.append(file[i]);
      }
      timeWeather = timeWeather.addMonths(buftime.toInt()-timeWeather.date().month());
      buftime.clear();
      for (int i = 14; i<16;i++) {
        buftime.append(file[i]);
      }
      timeWeather = timeWeather.addDays(buftime.toInt() - timeWeather.date().day());
      if (minsecs >= std::abs(timeWeather.secsTo(timeCompare))) {
        minsecs = std::abs(timeWeather.secsTo(timeCompare));
        weather = file;
      }
    }
  }
  if (weather.isEmpty()) {
    regexp.setPattern("weather.txt");
    foreach (QString file, fileList) {
      if ( regexp.exactMatch(file) ) {
        weather = file;
        break;
      }
    }
  }
  return tleDir + weather;
}

}


Dbi* internal::MappiGlobal::dbMappi()
{
  ConnectProp p = dbConfMappi();
  Dbi* nosql = createDatabase(p);

  if ( false == nosql->connect() ) {
    error_log << QObject::tr("Не удалось подключиться к БД = '%1'")
                 .arg( p.name() );
    delete nosql;
    nosql = nullptr;
  }
  return nosql;
}


ConnectProp internal::MappiGlobal::dbConfMappi() const
{ return dbConf("mappidb"); }



}
// const QHash< QString, QString > kModuleNames = THash< QString, QString >()
//   << qMakePair( QString("vizdoc"), QObject::tr( "ПК визуализации и документирования" ))
//   << qMakePair( QString("recgmi"), QObject::tr( "ПК приема (передачи) ГМИ" ))
//   << qMakePair( QString("sortgmi"), QObject::tr( "ПК сортировки ГМИ" ))
//   << qMakePair( QString("contrdiag"), QObject::tr( "ПК самоконтроля и диагностики" ))
//   << qMakePair( QString("settings"), QObject::tr( "ПК настройки" ))
//   << qMakePair( QString("codecontrol"), QObject::tr( "ПК раскодирования и контроля ГМИ" ))
//   << qMakePair( QString("obanal"), QObject::tr( "ПК объективного анализа" ))
//   << qMakePair( QString("prepare"), QObject::tr( "ПК подготовки исходных данных" ))
//   << qMakePair( QString("dbtask"), QObject::tr( "ПК обслуживания базы данных" ))
//   << qMakePair( QString("dbconn"), QObject::tr( "ПК взаимодействия с базой данных" ))
//   << qMakePair( QString("applied"), QObject::tr( "ПК прикладных задач" ))
//   << qMakePair( QString("vko"), QObject::tr( "ПК автоматизированного составления военно-климатических описаний" ))
//   << qMakePair( QString("rpc"), QObject::tr( "Удаленный вызов процедур" ))
//   << qMakePair( QString("journal"), QObject::tr( "ПК журналирования" ))
//   << qMakePair( QString("wrb"), QObject::tr( "ПК прикладных задач" ))
//   << qMakePair( QString("puansoneditor"), QObject::tr( "ПК визуализации и документирования" ))
//   << qMakePair( QString("global"), QObject::tr( "ПК журналирования" ))
//   << qMakePair( QString("forecast"), QObject::tr( "ПК прогнозирования" ))
//   << qMakePair( QString("prototext"), QObject::tr( "ПК настройки" ))
//   << qMakePair( QString("oceandiag"), QObject::tr( "ПК визуализации и документирования" ))
//   << qMakePair( QString("procview"), QObject::tr( "ПК самоконтроля и диагностики" ))
//   << qMakePair( QString("monitor"), QObject::tr( "ПК самоконтроля и диагностики" ))
//   << qMakePair( QString("procident"), QObject::tr( "ПК визуализации и документирования" ))
//   << qMakePair( QString("aerodiag"), QObject::tr( "ПК прикладных задач" ))
//   << qMakePair( QString("customui"), QObject::tr( "ПК визуализации и документирования" ))
//   << qMakePair( QString("forecastwidget"), QObject::tr( "ПК прогнозирования" ))
//   << qMakePair( QString("decodeviewer"), QObject::tr( "ПК визуализации и документирования" ))
//   << qMakePair( QString("nabludenia"), QObject::tr( "ПК визуализации и документирования" ))
//   << qMakePair( QString("tlgeditor"), QObject::tr( "ПК визуализации и документирования" ))
//   << qMakePair( QString("settings"), QObject::tr( "ПК настройки" ))
//   << qMakePair( QString("sprinf"), QObject::tr( "ПК взаимодействия с базой данных" ))
//   << qMakePair( QString("map"), QObject::tr( "ПК визуализации и документирования" ))
//   << qMakePair( QString("cron"), QObject::tr( "ПК самоконтроля и диагностики" ))
//   << qMakePair( QString("planner"), QObject::tr( "ПК журналирования" ))
//   << qMakePair( QString("verticalcut"), QObject::tr( "ПК прикладных задач" ))
//   << qMakePair( QString("msgviewer"), QObject::tr( "ПК визуализации и документирования" ))
//   << qMakePair( QString("radar"), QObject::tr( "ПК визуализации и документирования" ));

// const QString kPriorityName[tlog::kPriority_ARRAYSIZE] = { "DD", "DD", "II", "WW", "EE", "CC" };

// void logoutToDb(tlog::Priority priority, const QString& facility, const QString& fileName, int line, const QString &m)
// {
//   fMsgHandler old = TLog::setMsgHandler(TLog::stdOut);

//   QString sender = kModuleNames.value(facility, facility);

//   TLog::stdOut( priority, sender, fileName, line, m );

//   settings::RegLevel lvl;

//   switch ( priority ) {
//     case tlog::kNone:
//     case tlog::kDebug:
//       lvl = ::meteo::ukaz::Settings::instance()->logSettings().debug();
//       break;
//     case tlog::kInfo:
//       lvl = ::meteo::ukaz::Settings::instance()->logSettings().info();
//       break;
//     case tlog::kWarning:
//       lvl = ::meteo::ukaz::Settings::instance()->logSettings().warning();
//       break;
//     case tlog::kError:
//       lvl = ::meteo::ukaz::Settings::instance()->logSettings().error();
//       break;
//     case tlog::kCritical:
//       lvl = ::meteo::ukaz::Settings::instance()->logSettings().critical();
//       break;
//   }
//   if ( lvl != settings::kNone  )  {
//     NS_PGBase* db = meteo::global::dbJournal();

//     QString username = ::meteo::gGlobalObj()->currentUserShortName();
//     if( username.isEmpty() ){
//       username = meteo::gGlobalObj()->currentUserLogin();
//     }
//     if( username.isEmpty() ){
//       username = "unknown";
//     }

//     QString msg = m;
//     msg.replace( '\'', "''" );

//     QString q = QString( "SELECT logmessage( '%1', '%2', '%3', '%4', '%5', '%6', %7)" )
//                     .arg( kPriorityName[priority] )
//                     .arg( sender )
//                     .arg( username )
//                     .arg( MnCommon::selfIpAddress() )
//                     .arg( QHostInfo::localHostName() )
//                     .arg( msg )
//                     .arg( 0 );

//     TSqlQuery query( q, db );

//     bool res = query.exec();
//     if ( false == res ) {
//       error_log << QObject::tr("Не удалось выполнить запрос %1").arg(q);
//     }
//   }
//   TLog::setMsgHandler(old);
// }

// }


// ConnectProp UkazGlobal::dbConfJournal()
// { return ukaz::Settings::instance()->dbConfJournal(); }

// ConnectProp UkazGlobal::dbConfTelegram()
// { return ukaz::Settings::instance()->dbConfTelegram(); }

// ConnectProp MappiGlobal::dbConfMeteo()
// { return meteo::mappi::TMeteoSettings::instance()->dbConfMeteo(); }

// ConnectProp MappiGlobal::dbConfMappi()
// { return meteo::mappi::TMeteoSettings::instance()->dbConfMappi(); }

// ConnectProp UkazGlobal::dbConfObanal()
// { return ukaz::Settings::instance()->dbConfObanal(); }

// ConnectProp UkazGlobal::dbConfForecast()
// { return ukaz::Settings::instance()->dbConfForecast(); }

// NS_PGBase* UkazGlobal::dbTelegram()
// { return dbConn_.get(ukaz::kDbPtkpp, ukaz::Settings::instance()->dbConfTelegram()); }

// NS_PGBase* MappiGlobal::dbMeteo()
// { return dbConn_.get(meteo::mappi::kDbMeteo, meteo::mappi::TMeteoSettings::instance()->dbConfMeteo()); }

// NS_PGBase* MappiGlobal::dbMappi()
// { return dbConn_.get(meteo::mappi::kDbMappi, meteo::mappi::TMeteoSettings::instance()->dbConfMappi()); }

// NS_PGBase* UkazGlobal::dbObanal()
// { return dbConn_.get(ukaz::kDbObanal, ukaz::Settings::instance()->dbConfObanal()); }

// NS_PGBase*UkazGlobal::dbJournal()
// { return dbConn_.get(ukaz::kDbJournal, ukaz::Settings::instance()->dbConfJournal()); }

// NS_PGBase*UkazGlobal::dbForecast()
// { return dbConn_.get(ukaz::kDbForecast, ukaz::Settings::instance()->dbConfForecast()); }

// bool UkazGlobal::connectDb()
// { return meteo::TAdminSettings::instance()->connectDb(); }

// bool UkazGlobal::init()
// { return meteo::TAdminSettings::instance()->init(); }

// QString UkazGlobal::currentUserName()
// { return meteo::TAdminSettings::instance()->getCurrentUserName(); }

// QString UkazGlobal::currentUserShortName()
// { return meteo::TAdminSettings::instance()->getCurrentUserShortName(); }

// QString UkazGlobal::currentRank()
// { return meteo::TAdminSettings::instance()->getCurrentUserRank(); }

// QString UkazGlobal::rank(int code)
// { return meteo::TAdminSettings::instance()->ranks().value(code); }

// QStringList UkazGlobal::geoLoaders()
// { return meteo::map::Loader::instance()->geoLoaders(); }

// rpc::Address MappiGlobal::serviceAddress(meteo::settings::proto::ServiceCode code, bool* ok)
// { return meteo::mappi::TMeteoSettings::instance()->serviceAddress( code, ok ); }

// rpc::Channel* MappiGlobal::serviceChannel(meteo::settings::proto::ServiceCode code)
// {
//   QString serviceName = pbtools::toQString(meteo::settings::proto::ServiceCode_Name(code)).remove(0,1).toLower();

//   bool ok = false;
//   rpc::Address addr = serviceAddress( code, &ok );
//   if ( false == ok ) {
//     error_log << QObject::tr("Не удалось получить адрес сервиса %1")
//                  .arg(serviceName);
//     return 0;
//   }
//   rpc::Channel* ch = rpc::Channel::connect(addr);
//   if ( 0 == ch ) {
//     error_log << QObject::tr("Не удалось установить соединение со службой %1 по адресу %2")
//                  .arg( serviceName )
//                  .arg( addr.asString() );
//   }
//   return ch;
// }

//   rpc::TController* MappiGlobal::serviceController(meteo::settings::proto::ServiceCode code)
// {
//   QString serviceName = pbtools::toQString(meteo::settings::proto::ServiceCode_Name(code)).remove(0,1).toLower();

//   bool ok = false;
//   rpc::Address addr = serviceAddress( code, &ok );
//   if ( false == ok ) {
//     error_log << QObject::tr("Не удалось получить адрес сервиса %1")
//                  .arg(serviceName);
//     return 0;
//   }
//   rpc::TController* ctrl = new rpc::TController;
//   if ( false == ctrl->initChannel(addr) ) {
//     error_log << QObject::tr("Не удалось установить соединение с сервисом %1 по адресу %2")
//                  .arg( serviceName )
//                  .arg( addr.asString() );
//     delete ctrl;
//     return 0;
//   }
//   return ctrl;
// }

// int MappiGlobal::serviceTimeout(meteo::settings::proto::ServiceCode code)
// { return meteo::mappi::TMeteoSettings::instance()->serviceTimeout(code); }

//   meteo::settings::proto::Service MappiGlobal::service(meteo::settings::proto::ServiceCode code, bool* ok)
//   { return meteo::mappi::TMeteoSettings::instance()->service( code, ok ); }

// ukaz::proto::Location UkazGlobal::ukazpunkt()
// { return ukaz::Settings::instance()->locSettings(); }

//} // mappi

