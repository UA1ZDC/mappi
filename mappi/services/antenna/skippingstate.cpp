#include "skippingstate.h"


namespace mappi {

namespace antenna {

SkippingState::SkippingState(Context* ctx, StateMachine* fsm) :
  AntennaState(ctx, fsm)
{
}

SkippingState::~SkippingState()
{
}

State::id_t SkippingState::branch(Primitive::id_t prim, Primitive::queue_t* input)
{
  switch (prim) {
    case Primitive::FAIL_IND :
      info_log << QObject::tr("Приём сеанса будет пропущен");
      fsm->startTimer_msec(ctx->session.msecsToLos());
      break ;

    case Primitive::TIMEOUT :
      // уведомление о новом сеансе получено раньше завершения текущего
      if (!ctx->isReplay())
        input->append(Primitive::ACCEPT_SESSION);

      return State::IDLING;

    case Primitive::SERVICE_MODE_ON :
      fsm->stopTimer();
      return State::SERVICE;

    default :
      break ;
  }

  return id();
}

}

}
