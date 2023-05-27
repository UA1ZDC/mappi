#ifndef COMMONS_CONTAINER_BLOCKINGQUEUE_H
#define COMMONS_CONTAINER_BLOCKINGQUEUE_H

#include <qlist.h>
#include <qmutex.h>
#include <qsemaphore.h>

//! Очередь с блокировкой получения элементов.
template<class T> class BlockingQueue
{
public:
  void append(const T& value);

  T takeFirst();
  void removeFirst();

  int size() const;

private:
  mutable QMutex mutex_;
  QSemaphore sem_;
  QList<T> list_;
};

template<class T> void BlockingQueue<T>::append(const T& value)
{
  mutex_.lock();
  list_.append( value );
  mutex_.unlock();
  sem_.release();
}

template<class T> T BlockingQueue<T>::takeFirst()
{
  sem_.acquire();
  return list_.takeFirst();
}

template<class T> void BlockingQueue<T>::removeFirst()
{
  mutex_.lock();
  if ( !list_.isEmpty() ) {
    sem_.acquire();
    list_.removeFirst();
  }
  mutex_.unlock();
}

template<class T> int BlockingQueue<T>::size() const
{
  QMutexLocker locker( &mutex_ );
  return list_.size();
}

#endif // COMMONS_CONTAINER_BLOCKINGQUEUE_H
