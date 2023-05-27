#pragma once

#include <cross-commons/debug/tlog.h>
#include <mappi/schedule/session.h>
#include <mappi/device/receiver/receiver.h>


namespace mappi {

namespace receiver {

/**
 * Контекст (контекст выполнения) - служебная информация, которая соответствует текущему запуску receiver.service:
 * Время жизни объекта - пока запущен receiver.service.
 *
 * Отличия сеансов:
 * - активный (предварительный) - сеанс получен от playback.service;
 * - исполнительный - сеанс ожидает приёма (т.е. будет принят даже при падении playback.service) или уже принимается.
 *
 * Сеансы с префиксом Original - сеанс из расписания.
 * Без префикса - коррекция aos и los для демонстрационного режима.
 * Если демонстрационный режим отключен сеансы равны.
 */
class Context
{
public :
  Context();
  ~Context();

  bool init();

  QDateTime now() const;
  bool isReplay() const;

  QString lastError();
  void setLastError(const QString& msg);
  bool hasError() const;

public :
  Receiver* receiver; // QObject
  Configuration conf;

  // активный (предварительный) сеанс
  schedule::Session sessionOriginal_prep;
  schedule::Session session_prep;
  conf::TleItem tleItem_prep;

  // исполнительный сеанс
  schedule::Session sessionOriginal;
  schedule::Session session;
  conf::TleItem tleItem;

  Satellite satellite;

private :
  QString error_;
};

}

}
