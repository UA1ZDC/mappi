#ifndef SQL_NSPGBASE_CONNECTIONDICT_H
#define SQL_NSPGBASE_CONNECTIONDICT_H

#include <qmap.h>
#include <qreadwritelock.h>

#include <sql/psql/tpgsettings.h>

namespace meteo {

class Psql;
class Dbi;

class ConnectionDict: public QMap<int,Psql*>
{
public:
  inline bool contains_ts(int key) const
  {
    QReadLocker locker(&lock_);
    return contains(key);
  }

  inline Psql* value_ts(int key) const
  {
    QReadLocker locker(&lock_);
    return value(key, 0);
  }

  Dbi* get(int type, const ConnectProp& prop);

private:
  mutable QReadWriteLock lock_;
};

}

#endif
