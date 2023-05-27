#include "idlingstate.h"


namespace mappi {

namespace antenna {

IdlingState::IdlingState(Context* ctx, StateMachine* fsm) :
  AntennaState(ctx, fsm)
{
}

IdlingState::~IdlingState()
{
}

State::id_t IdlingState::branch(Primitive::id_t prim, Primitive::queue_t* input)
{
  switch (prim) {
    case Primitive::ACCEPT_SESSION :
      if (ctx->sessionOriginal_prep.data().isDefault())
        break ;

      ctx->sessionOriginal = ctx->sessionOriginal_prep;
      ctx->session = ctx->session_prep;
      ctx->tleItem = ctx->tleItem_prep;

      info_log << QString("\n%1\n%2\n%3\n%4\n%5")
        .arg(ctx->sessionOriginal.data().toString() + (ctx->conf.demoMode ? "\toriginal" : ""))
        .arg((ctx->conf.demoMode ? ctx->session.data().toString() + "\tdemo" : ""))
        .arg(ctx->tleItem.title().c_str())
        .arg(ctx->tleItem.line_1().c_str())
        .arg(ctx->tleItem.line_2().c_str());

      input->append(Primitive::WARMING_UP);
      return State::WARMING;

    case Primitive::SERVICE_MODE_ON :
      return State::SERVICE;

    default :
      break ;
  }

  return id();
}

}

}
