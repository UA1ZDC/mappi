#pragma once

#include "fsm.h"


namespace mappi {

namespace playback {

/** Сервисный режим. */
class ServiceState :
  public ReceptionState
{
public :
  explicit ServiceState(Context* ctx, StateMachine* fsm);
  virtual ~ServiceState();

  virtual State::id_t id() const { return State::SERVICE; }
  virtual State::id_t branch(Primitive::id_t prim, Primitive::queue_t* input);

private :
  void notifyAll(bool flag);
  void notifyReceiver(const ServiceRequest& req);
  void notifyAntenna(const ServiceRequest& req);
};

}

}
