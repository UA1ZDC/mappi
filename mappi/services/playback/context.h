#pragma once

#include <cross-commons/debug/tlog.h>
#include <mappi/proto/reception.pb.h>
#include <mappi/schedule/session.h>


namespace mappi {

namespace playback {

/**
 * Контекст (контекст выполнения) - служебная информация, которая соответствует текущему запуску receiver.service:
 * Время жизни объекта - пока запущен playback.service.
 */
class Context
{
public :
  static const int WARMING_INTERVAL = 59;   // прогрев (время упреждения), сек.

public :
  Context();
  ~Context();

  bool confLoad();

  QDateTime now() const;
  QDateTime actualAos() const;

  QString lastError();
  void setLastError(const QString& msg);
  bool hasError() const;

public :
  bool demoMode;

  schedule::Session session;
  conf::TleItem tleItem;

  qint64 timeOffset;

private :
  QString error_;
};

}

}
