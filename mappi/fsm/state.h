#pragma once

#include "primitive.h"
#include <meteo/commons/global/global.h>
#include <qsharedpointer.h>
#include <qlist.h>
#include <qmap.h>


namespace mappi {

namespace fsm {

using namespace meteo;

class StateMachine;

/**
 * Состояние - составная часть машины.
 * Состояние определяет каким образом машина будет реагировать на входное событие.
 * В каждый конкретный момент времени машина может находиться только в одном состоянии.
 */
class State
{
  Q_GADGET;
public :
  enum id_t {
    IDLING = 0, // ожидание приёма, холостой ход
    WARMING,    // подготовка к приёму, "прогрев"
    ACTIVE,     // приём сеанса
    SKIPPING,   // пассивное ожидание завершения приёма
    SERVICE,    // сервисный режим
    FIND,       // поиск сеанса для приёма
    PREWAIT,    // предварительное ожидание начала приёма
    SYNC        // синхронизация
  };

  Q_ENUM(id_t)

  typedef QSharedPointer<State> ptr_t;
  typedef QMap<id_t, ptr_t> pool_t;

public :
  State(StateMachine* fsm);
  virtual ~State();

  virtual id_t id() const = 0;
  virtual id_t branch(Primitive::id_t prim, Primitive::queue_t* input) = 0;

protected :
  rpc::Channel* channel(settings::proto::ServiceCode code);

protected :
  StateMachine* fsm;
};

}

}
