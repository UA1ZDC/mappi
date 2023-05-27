#ifndef TMAP_H
#define TMAP_H

#pragma message("Header 'commons/container/tmap.h' is deprecated! Please use 'cross-commons/debug/tmap.h'")
#pragma message("This header file will be deleted on next update")

#include <qmap.h>
#include <qpair.h>

#include <cross-commons/includes/tcrossdefines.h>

#if TQT_VERSION == 3

template <class Key,class T> class TMap : public QMap<Key,T>
{
  public:
    inline TMap<Key,T>& operator<<( const QPair<Key,T>& t )
    { this->insert(t.first,t.second); return *this; }
};
#endif

#if TQT_VERSION == 4
template < class Key, class T > class TMap : public QMap< Key, T >
{
  public:
    inline TMap<Key,T>& operator<<( const QPair<Key,T>& t )
    { this->insert(t.first,t.second); return *this; }
};


#include <qreadwritelock.h>

template < class Key, class T >
class TSMap : public QMap< Key, T >
{
public:
  inline bool contains_ts(Key key) const
  {
    QReadLocker locker(&lock_);
    return QMap<Key,T>::contains(key);
  }

  inline T value_ts(Key key, T defaultValue = T()) const
  {
    QReadLocker locker(&lock_);
    return QMap<Key,T>::value(key, defaultValue);
  }

  inline void insert_ts(Key key, T value)
  {
    QWriteLocker locker(&lock_);
    QMap<Key,T>::insert(key, value);
  }

private:
  mutable QReadWriteLock lock_;
};

#endif

#endif // TMAP_H
