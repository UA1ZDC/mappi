#pragma once

#include "fsm.h"


namespace mappi {

namespace antenna {

/**
 * Сервисный режим.
 * Прекращение приёма по расписанию.
 * Доступно ручное управление антенной:
 * - позиционирование;
 * - прогон;
 * - перевод в походное положение.
 */
class ServiceState :
  public AntennaState
{
public :
  ServiceState(Context* ctx, StateMachine* fsm);
  virtual ~ServiceState();

  virtual State::id_t id() const { return State::SERVICE; }
  virtual State::id_t branch(Primitive::id_t prim, Primitive::queue_t* input);
};

}

}
