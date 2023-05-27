#pragma once

#include "fsm.h"


namespace mappi {

namespace antenna {

/** Состояние - сопровождение спутника. */
class ActiveState :
  public AntennaState
{
public :
  ActiveState(Context* ctx, StateMachine* fsm);
  virtual ~ActiveState();

  virtual State::id_t id() const { return State::ACTIVE; }
  virtual State::id_t branch(Primitive::id_t prim, Primitive::queue_t* input);

private :
  QDateTime actualAos;
};

}

}
