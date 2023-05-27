#pragma once

#include "sessiondata.h"
#include <qlist.h>


namespace mappi {

namespace schedule {

class Session
{
public :
  // стадия сеанса
  enum stage_t {
    COMPLETED = 0,      // пропущен, завершен
    WAITING,            // ожидание
    PROCEED,            // обработка, уже принимается
  };

  typedef QList<Session> list_t;

public :
  static void toProto(const Session& other, conf::Session* session);
  static void fromProto(const conf::Session& other, Session* session);

public :
  Session();
  Session(const SessionData& data);
  Session(const conf::Session& object);

  ~Session();

  SessionData& data() { return data_; }
  const SessionData& data() const { return data_; }

  // разница между заданным временем и началом сеанса в сек.
  int secsToAos(const QDateTime& dt = QDateTime::currentDateTimeUtc()) const;

  // разница между заданным временем и началом сеанса в мсек.
  int msecsToAos(const QDateTime& dt = QDateTime::currentDateTimeUtc()) const;

  // разница между заданным временем и концом сеанса в сек.
  int secsToLos(const QDateTime& dt = QDateTime::currentDateTimeUtc()) const;

  // разница между заданным временем и концом сеанса в мсек.
  int msecsToLos(const QDateTime& dt = QDateTime::currentDateTimeUtc()) const;

  // общая продолжительность сеанса (los - aos) в сек.
  int duration() const;

  // true сеанс не принимается
  bool isIgnored() const;

  stage_t stage(const QDateTime& dt = QDateTime::currentDateTimeUtc()) const;

public :
  bool operator ==(const Session& other) const;
  bool operator <(const Session& other) const;
  bool operator !=(const Session& other) const;

private :
  SessionData data_;
};

}

}
