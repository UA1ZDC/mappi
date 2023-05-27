#pragma once

#include "fsm.h"


namespace mappi {

namespace receiver {

/** Состояние работы приёмника. */
class ActiveState :
  public ReceiverState
{
public :
  ActiveState(Context* ctx, StateMachine* fsm);
  virtual ~ActiveState();

  virtual State::id_t id() const { return State::ACTIVE; }
  virtual State::id_t branch(Primitive::id_t prim, Primitive::queue_t* input);

private :
  void saveToDb();
  void feedhornOutput(int n, bool turnOn);

private :
  QDateTime actualAos;
};

}

}
