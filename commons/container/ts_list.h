#ifndef METEO_COMMONS_GLOBAL_TS_LIST_H
#define METEO_COMMONS_GLOBAL_TS_LIST_H

#include <qlist.h>
#include <qmutex.h>

//! Потокобезопасный список.
template<class T> class TSList
{
public:
  void append(const T& value);
  void append(const QList<T>& list);

  void prepend(const T& value);
  void prepend(const QList<T>& list);

  T at(int i) const;
  T first() const;
  T last() const;

  T takeFirst();
  T takeLast();

  void removeFirst();
  void removeLast();
  void removeAt(int idx);

  int size() const;
  bool empty() const;

  bool contains(const T& value) const;

private:
  mutable QMutex mutex_;
  QList<T> list_;
};

template <class T> bool TSList<T>::empty() const
{
  QMutexLocker locker(&mutex_);
  return list_.empty();
}


template<class T> void TSList<T>::append(const T& value)
{
  mutex_.lock();
  list_.append( value );
  mutex_.unlock();
}

template<class T> void TSList<T>::append(const QList<T>& list)
{
  mutex_.lock();
  for ( int i=list.size() - 1; i>=0; --i ) {
    list_.append(list.at(i));
  }
  mutex_.unlock();
}

template<class T> void TSList<T>::prepend(const T& value)
{
  mutex_.lock();
  list_.prepend( value );
  mutex_.unlock();
}

template<class T> void TSList<T>::prepend(const QList<T>& list)
{
  mutex_.lock();
  for ( int i=list.size() - 1; i>=0; --i ) {
    list_.prepend(list.at(i));
  }
  mutex_.unlock();
}

template<class T> T TSList<T>::at(int i) const
{
  QMutexLocker locker(&mutex_);
  return list_.at(i);
}

template<class T> T TSList<T>::first() const
{
  QMutexLocker locker(&mutex_);
  return list_.first();
}

template<class T> T TSList<T>::last() const
{
  QMutexLocker locker(&mutex_);
  return list_.last();
}

template<class T> T TSList<T>::takeFirst()
{
  QMutexLocker locker(&mutex_);
  return list_.takeFirst();
}

template<class T> T TSList<T>::takeLast()
{
  QMutexLocker locker(&mutex_);
  return list_.takeLast();
}

template<class T> void TSList<T>::removeFirst()
{
  mutex_.lock();
  if ( !list_.isEmpty() ) {
    list_.removeFirst();
  }
  mutex_.unlock();
}

template<class T> void TSList<T>::removeLast()
{
  mutex_.lock();
  if ( !list_.isEmpty() ) {
    list_.removeLast();
  }
  mutex_.unlock();
}

template<class T> void TSList<T>::removeAt(int idx)
{
  mutex_.lock();
  list_.removeAt(idx);
  mutex_.unlock();
}

template<class T> int TSList<T>::size() const
{
  QMutexLocker locker( &mutex_ );
  return list_.size();
}

template<class T> bool TSList<T>::contains(const T& value) const
{
  QMutexLocker locker( &mutex_ );
  return list_.contains(value);
}

#endif // METEO_COMMONS_GLOBAL_TS_LIST_H
