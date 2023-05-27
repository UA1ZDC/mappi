#ifndef METEO_COMMONS_TLGPOOL_TELEGRAMSPOOL_H
#define METEO_COMMONS_TLGPOOL_TELEGRAMSPOOL_H

#include <QList>
#include <QMutex>


namespace meteo {
namespace msg {
class Telegram;
} // msg

template <typename T>
class SafeTelegramPool
{
private:
  QMutex mutex_;
  QList<T> pool_;

public:
  void addTelegram(const T& tlg)
  {
    QMutexLocker locker(&mutex_);
    pool_.append(tlg);
  }

  QList<T> getTelegrams()
  {
    QMutexLocker locker(&mutex_);
    QList<T> result;
    qSwap(pool_, result);
    return result;
  }

  bool isEmpty()
  {
    return pool_.isEmpty();
  }  

  //очередь должна быть не пуста, проверить isEmpty()
  // const T& first()
  // {
  //   return pool_.first();
  // }

  T takeFirst()
  {
    QMutexLocker locker(&mutex_);
    if (!pool_.isEmpty()) {
      return pool_.takeFirst();
    }

    return T();
  }

  void removeFirst()
  {
    QMutexLocker locker(&mutex_);
    if (!pool_.isEmpty()) {
      pool_.removeFirst();
    }
  }

  int size()
  {
    return pool_.size();
  }

};

typedef SafeTelegramPool<msg::Telegram> TelegramsPool;

} // meteo

#endif 
