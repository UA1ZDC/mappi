#pragma once

#include <mappi/schedule/schedule.h>
#include <cross-commons/debug/tlog.h>


namespace mappi {

namespace schedule {

class Context
{
public :
  bool confLoad();

  bool makeSchedule();
  void saveSchedule();

  QString lastError();
  void setLastError(const QString& msg);
  bool hasError() const;

public :
  Schedule schedule;
  Configuration conf;

private :
  QString error_;
};

}

}
