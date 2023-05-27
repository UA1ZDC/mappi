#include "fsm.h"
#include "findstate.h"
#include "prewaitstate.h"
#include "warmingstate.h"
#include "activestate.h"
#include "servicestate.h"


namespace mappi {

namespace playback {

// class ReceptionState
ReceptionState::ReceptionState(Context* ctx, StateMachine* fsm) :
   State(fsm)
{
  this->ctx = ctx;
}

ReceptionState::~ReceptionState()
{
  ctx = nullptr;
}


// class FSM
FSM::FSM(Context* ctx, QObject* parent /*=*/) :
  StateMachine(parent)
{
  registerState(new FindState(ctx, this), true);        // начальное состояние
  registerState(new PreWaitState(ctx, this));
  registerState(new WarmingState(ctx, this));
  registerState(new ActiveState(ctx, this));
  registerState(new ServiceState(ctx, this));
}

FSM::~FSM()
{
}

}

}
