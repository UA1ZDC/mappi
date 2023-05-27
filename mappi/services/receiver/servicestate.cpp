#include "servicestate.h"


namespace mappi {

namespace receiver {

ServiceState::ServiceState(Context* ctx, StateMachine* fsm) :
  ReceiverState(ctx, fsm)
{
}

ServiceState::~ServiceState()
{
}

State::id_t ServiceState::branch(Primitive::id_t prim, Primitive::queue_t* input)
{
  Q_UNUSED(input)

  switch (prim) {
    case Primitive::SERVICE_MODE_OFF :
      return State::IDLING;

    case Primitive::ACCEPT_SESSION :
      warning_log << QObject::tr("Cеанс(%1) игнорируется")
        .arg(ctx->sessionOriginal_prep.data().info());

      ctx->sessionOriginal_prep.data().setDefault();
      break ;

    default :
      break ;
  }

  return id();
}

}

}
