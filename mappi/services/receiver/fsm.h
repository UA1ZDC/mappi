#pragma once

#include "context.h"
#include <mappi/fsm/statemachine.h>

#define STATE_ERROR(text) \
  ctx->setLastError(text); \
  error_log << text \


namespace mappi {

namespace receiver {

using namespace fsm;
using namespace schedule;


class ReceiverState :
  public State
{
public :
  ReceiverState(Context* ctx, StateMachine* fsm);
  virtual ~ReceiverState();

protected :
  Context* ctx;
};


class FSM :
  public StateMachine
{
public :
  FSM(Context* ctx, QObject* parent = nullptr);
  virtual ~FSM();
};

}

}
