#include "fsm.h"
#include "idlingstate.h"
#include "warmingstate.h"
#include "activestate.h"
#include "skippingstate.h"
#include "servicestate.h"
#include "syncstate.h"


namespace mappi {

namespace receiver {

// class ReceiverState
ReceiverState::ReceiverState(Context* ctx, StateMachine* fsm) :
   State(fsm)
{
  this->ctx = ctx;
}

ReceiverState::~ReceiverState()
{
  ctx = nullptr;
}


// class FSM
FSM::FSM(Context* ctx, QObject* parent /*=*/) :
  StateMachine(parent)
{
  registerState(new SyncState(ctx, this), true);        // начальное состояние
  registerState(new IdlingState(ctx, this));
  registerState(new WarmingState(ctx, this));
  registerState(new ActiveState(ctx, this));
  registerState(new SkippingState(ctx, this));
  registerState(new ServiceState(ctx, this));
}

FSM::~FSM()
{
}

}

}
