#pragma once

#include "context.h"
#include <mappi/fsm/statemachine.h>

#define STATE_ERROR(text) \
  ctx->setLastError(text); \
  error_log << text \


namespace mappi {

namespace antenna {

using namespace fsm;
using namespace schedule;


class AntennaState :
  public State
{
public :
  AntennaState(Context* ctx, StateMachine* fsm);
  virtual ~AntennaState();

protected :
  Context* ctx;
};


class FSM :
  public StateMachine
{
public :
  FSM(Context* ctx, QObject* parent = nullptr);
  virtual ~FSM();

private :
  Context* ctx_;
};

}

}
