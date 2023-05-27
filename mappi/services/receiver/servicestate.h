#pragma once

#include "fsm.h"


namespace mappi {

namespace receiver {

/**
 * Сервисный режим.
 * Прекращение приёма по расписанию.
 */
class ServiceState :
  public ReceiverState
{
public :
  ServiceState(Context* ctx, StateMachine* fsm);
  virtual ~ServiceState();

  virtual State::id_t id() const { return State::SERVICE; }
  virtual State::id_t branch(Primitive::id_t prim, Primitive::queue_t* input);
};

}

}
