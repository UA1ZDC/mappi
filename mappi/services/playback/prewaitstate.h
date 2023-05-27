#pragma once

#include "fsm.h"


namespace mappi {

namespace playback {

/**
 * Предварительное ожидание начала сеанса, т.е. не aos, а aos - время упреждения.
 * В случае если сеанс уже начался, предварительное ожидание не выполняется.
 */
class PreWaitState :
  public ReceptionState
{
public :
  explicit PreWaitState(Context* ctx, StateMachine* fsm);
  virtual ~PreWaitState();

  virtual State::id_t id() const { return State::PREWAIT; }
  virtual State::id_t branch(Primitive::id_t prim, Primitive::queue_t* input);
};

}

}
