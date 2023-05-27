#ifndef METEO_MAPPI_CLEANER_FUNCS_H
#define METEO_MAPPI_CLEANER_FUNCS_H

#include <qstring.h>

#include <cross-commons/debug/tlog.h>

#include <commons/textproto/pbtools.h>

#include <mappi/global/global.h>
#include <mappi/proto/cleaner.pb.h>
#include <mappi/settings/tmeteosettings.h>

namespace mappi {
namespace cleaner {

inline ConnectProp getDbConf(const QString& name)
{
//  if ( "db_ptkpp" == name ) {
//    return global::dbConfTelegram();
//  }
//  else if ( "db_meteo" == name ) {
//    return global::dbConfMeteo();
//  }
//  else if ( "db_meteo_loc" == name ) {
//    return global::dbConfMeteoLocal();
//  }
//  else if ( "db_obanal" == name ) {
//    return global::dbConfObanal();
//  }
//  else if ( "db_journal" == name ) {
//    return global::dbConfJournal();
//  }
//  else if ( "db_climat" == name ) {
//    return global::dbConfClimat();
//  }
//  else if ( "db_forecast" == name ) {
//    return global::dbConfForecast();
//  }
//  else if ( "db_documents" == name ) {
//    return global::dbConfObanal();
//  }
  if ( "db_mappi" == name ) {
    return meteo::global::dbConfMappi();
  }

  error_log << QObject::tr("Подключение '%1' не найдено.").arg(name);
  return ConnectProp();
}

inline QString sqlUnit(IntervalUnit unit)
{
  switch ( unit ) {
    case kDays:   return "days";
    case kMonths: return "month";
    case kYears:  return "years";
  }
  return QString();
}

inline QString prepareSql(QString sql, const QString& dbName)
{
  DbIntervalOption opt = ::meteo::mappi::TMeteoSettings::instance()->dbClearConf(dbName);

  QString v = QString::number(opt.value()) + " " + sqlUnit(opt.unit());
  sql.replace("$clear_interval$", v);

  return sql;
}

} // cleaner
} // meteo

#endif // METEO_MAPPI_CLEANER_FUNCS_H
