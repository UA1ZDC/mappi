#pragma once

#include <qobject.h>


namespace mappi {

namespace fsm {

/**
 * Примитива - событие, которое обрабатывает машина состояний.
 * Основное назначение - идентификация события, не предназначена для передачи данных.
 */
class Primitive
{
  Q_GADGET;
public :
  enum id_t {
    TIMEOUT = 0,        // событие таймера
    SERVICE_MODE_ON,    // переход в сервисный режим
    SERVICE_MODE_OFF,   // выход из сервисного режима
    ACCEPT_SESSION,     // принять сеанс
    WARMING_UP,         // выполнить подготовку к приёму
    WARMING_COMPLETED,  // подготовка завершена, начало сеанса
    WAIT_IND,           // предварительное ожидание
    ALERT_IND,          // начало сеанса пропущено, быстрый старт
    FAIL_IND,           // произошла ошибка при приёме сеанса
    ACTIVE_SESSION,     // запрос активного сеанса

    ANTENNA_SET_POSITION,
    ANTENNA_STOP,
    ANTENNA_MONKEY_TEST,
    ANTENNA_TRAVEL_MODE
  };
  Q_ENUM(id_t)

  typedef QList<id_t> queue_t;

private :
  Primitive() = delete;
  Primitive(const Primitive& other) = delete;
};

}

}
