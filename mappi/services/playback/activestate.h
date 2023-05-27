#pragma once

#include "fsm.h"


namespace mappi {

namespace playback {

/**
 * Состояние ожидания завершения активного сеанса.
 * Необходимо выждать т.к. при запросе следующего сеанса вернется тот же самый сеанс.
 * Это условие касается и демонстрационного режима работы.
 */
class ActiveState :
  public ReceptionState
{
public :
  explicit ActiveState(Context* ctx, StateMachine* fsm);
  virtual ~ActiveState();

  virtual State::id_t id() const { return State::ACTIVE; }
  virtual State::id_t branch(Primitive::id_t prim, Primitive::queue_t* input);

private :
  void notify();
};

}

}
