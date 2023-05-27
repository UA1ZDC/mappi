#pragma once

#include "fsm.h"
#include <mappi/device/antenna/direct.h>


namespace mappi {

namespace antenna {

/** */
class WarmingState :
  public AntennaState
{
public :
  WarmingState(Context* ctx, StateMachine* fsm);
  virtual ~WarmingState();

  virtual State::id_t id() const { return State::WARMING; }
  virtual State::id_t branch(Primitive::id_t prim, Primitive::queue_t* input);

private :
   bool executeCheckList(Context* ctx);
};

}

}
