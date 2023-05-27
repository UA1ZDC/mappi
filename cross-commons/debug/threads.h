#ifndef CROSS_COMMONS_DEBUG_THREADS_H
#define CROSS_COMMONS_DEBUG_THREADS_H

#include <qthread.h>
#include <qmutex.h>

#include <cross-commons/debug/tmap.h>
#include <cross-commons/debug/tlog.h>

#define THREAD_NAME  (gThreadCounter()->name())

class ThreadCounter
{
public:
  ThreadCounter() : numb_(0) {}

  inline QString name()
  {
    QMutexLocker lock(&m_);

    quint64 id = quint64(QThread::currentThreadId());

    if ( !threads_.contains_ts(id) ) {
      threads_.insert_ts(id, QString::number(numb_++));
    }

    return threads_.value_ts(id);
  }

private:
  TSMap<quint64,QString> threads_;
  int numb_;
  QMutex m_;
};

ThreadCounter* gThreadCounter();

#endif // CROSS_COMMONS_DEBUG_THREADS_H
