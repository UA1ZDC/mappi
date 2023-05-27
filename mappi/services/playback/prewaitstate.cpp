#include "prewaitstate.h"


namespace mappi {

namespace playback {

PreWaitState::PreWaitState(Context* ctx, StateMachine* fsm) :
  ReceptionState(ctx, fsm)
{
}

PreWaitState::~PreWaitState()
{
}

State::id_t PreWaitState::branch(Primitive::id_t prim, Primitive::queue_t* input)
{
  switch (prim) {
    case Primitive::WAIT_IND : {
      int sec = ctx->session.secsToAos(ctx->now()) - Context::WARMING_INTERVAL;
      if (ctx->demoMode) {
        if (0 <= sec)
          ctx->timeOffset += sec;

        debug_log << "time offset:" << ctx->timeOffset;
      } else {
        // c ожиданием
        if (0 < sec) {
          fsm->startTimer_sec(sec);
          break ;
        }

        info_log << QObject::tr("Без предварительного ожидания");
      }

      // без ожидания
    }

    case Primitive::TIMEOUT :
      // !!! начало ожидания
      input->append(Primitive::WARMING_UP);
      return State::WARMING;

    case Primitive::SERVICE_MODE_ON :
      fsm->stopTimer();
      input->append(Primitive::SERVICE_MODE_ON);
      return State::SERVICE;

    default :
      break ;
  }

  return id();
}

}

}
