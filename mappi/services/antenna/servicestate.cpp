#include "servicestate.h"


namespace mappi {

namespace antenna {

ServiceState::ServiceState(Context* ctx, StateMachine* fsm) :
  AntennaState(ctx, fsm)
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
      ctx->direct->terminate();
      ctx->mTest->terminate();

      return State::IDLING;

    case Primitive::ACCEPT_SESSION :
      warning_log << QObject::tr("Cеанс(%1) игнорируется")
        .arg(ctx->sessionOriginal_prep.data().info());

      ctx->sessionOriginal_prep.data().setDefault();
      break ;

    case Primitive::ANTENNA_SET_POSITION :
      ctx->mTest->terminate();
      ctx->direct->run(ctx->antenna);

      break ;

    case Primitive::ANTENNA_STOP :
      ctx->direct->terminate();
      ctx->mTest->terminate();
      ctx->antenna->stop();

      break ;

    case Primitive::ANTENNA_MONKEY_TEST :
      ctx->direct->terminate();
      ctx->mTest->run(ctx->antenna);

      break ;

    // TODO нужны детали,
    // как перейти в походное положение и как вернуться обратно
    case Primitive::ANTENNA_TRAVEL_MODE :
      ctx->direct->terminate();
      ctx->mTest->terminate();
      ctx->antenna->travelMode();

      break ;

    default :
      break ;
  }

  return id();
}

}

}
