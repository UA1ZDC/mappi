#include "connectiondict.h"

#include <cross-commons/debug/tlog.h>
#include <sql/psql/psql.h>

namespace meteo {

Dbi* ConnectionDict::get(int type, const ConnectProp& prop)
{
  if ( !contains_ts(type) ) {
    QWriteLocker locker(&lock_);
    insert(type, new Psql);
  }

  Dbi* db = value_ts(type);
  if ( !db->connect(prop) ) {
    error_log << QString::fromUtf8("Не удалось подключиться к БД.");
    debug_log << prop.print();
  }
  return db;
}

}
