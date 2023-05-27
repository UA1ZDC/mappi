#pragma once

#include "session.h"
#include "config.h"
#include <sat-commons/satellite/satellite.h>


namespace mappi {

namespace schedule {

class Schedule
{
public :
  Schedule();
  ~Schedule();

  // составить расписание, между сеансами могут быть конфликты
  bool make(const Configuration& conf, const QDateTime& dt = QDateTime::currentDateTimeUtc());

  // решение конфликтов, конфликты могут быть решены вручную пользователем
  void resolvConfl(const Configuration& conf, bool saveConflResolByUser = false);

  void clear();
  bool isEmpty() const;
  int size() const;

  int getIdx(const Session& session) const;

  Session& getSession(int idx);
  Session& getSession(const QString& satellite, const unsigned revol);

  Session& near(bool useProceed = true);
  Session& near(const QDateTime& dt, bool useProceed = true);
  Session& next();

  Session& first();
  Session& last();

  int percentCompleted() const;

  QString toString(bool isPretty = false) const;
  QString toNumberString() const;

  bool save(const QString& filePath) const;
  bool load(const QString& filePath);

private :
  int cmpConflict(const Session& lhs, const Session& rhs, const Configuration& conf) const;
  int timeWest(const Session& session, int step, const Configuration& conf) const;
  int direction(const Satellite& satellite, double aos) const;

private :
  static Session default_;
  Session::list_t data_;
};

}

}
