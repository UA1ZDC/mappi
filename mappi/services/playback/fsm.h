#pragma once

#include "context.h"
#include <mappi/fsm/statemachine.h>

#define STATE_ERROR(text) \
  ctx->setLastError(text); \
  error_log << text \


namespace mappi {

namespace playback {

using namespace fsm;
using namespace schedule;


class ReceptionState :
  public State
{
public :
  ReceptionState(Context* ctx, StateMachine* fsm);
  virtual ~ReceptionState();

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
