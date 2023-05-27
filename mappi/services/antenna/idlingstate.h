#pragma once

#include "fsm.h"


namespace mappi {

namespace antenna {

/** Состояние ожидания приёма (холостой ход), ожидание уведомление об активном сеансе. */
class IdlingState :
  public AntennaState
{
public :
  IdlingState(Context* ctx, StateMachine* fsm);
  virtual ~IdlingState();

  virtual State::id_t id() const { return State::IDLING; }
  virtual State::id_t branch(Primitive::id_t prim, Primitive::queue_t* input);
};

}

}
