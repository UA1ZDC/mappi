#include "syncstate.h"


namespace mappi {

namespace antenna {

SyncState::SyncState(Context* ctx, StateMachine* fsm) :
  AntennaState(ctx, fsm)
{
}

SyncState::~SyncState()
{
}

State::id_t SyncState::branch(Primitive::id_t prim, Primitive::queue_t* input)
{
  switch (prim) {
    case Primitive::ACTIVE_SESSION :
      switch (sync()) {
        case kServiceState :
          input->append(Primitive::SERVICE_MODE_ON);
          break ;

        case kWarmingState :
        case kActiveState :
          input->append(Primitive::ACCEPT_SESSION);
          break ;

        default :
          break ;
      }

      return State::IDLING;

    default :
      break ;
  }

  return id();
}

ServiceState SyncState::sync()
{
  QSharedPointer<rpc::Channel> ch(channel(settings::proto::kPlayback));
  if (ch == nullptr) {
    warning_log << QObject::tr("%1 недоступен")
      .arg(global::serviceTitle(settings::proto::kPlayback));
    return kPreWaitState;
  }

  QSharedPointer<conf::ReceptionResponse> resp(ch->remoteCall(&conf::ReceptionService::HealthCheck, Dummy(), 10000, true));
  if (resp.isNull()) {
    error_log << QObject::tr("Сервис вернул пустой ответ");
    return kPreWaitState;
  }

  debug_log << "reception state:" << STATE_NAME(resp->state());

  if (resp->result() == false) {
    if (resp->has_comment())
      error_log << resp->comment().c_str();

    return kPreWaitState;
  }

  switch (resp->state()) {
    case kWarmingState :
    case kActiveState :
      ctx->tleItem_prep = resp->tle();
      schedule::Session::fromProto(resp->session(), &ctx->sessionOriginal_prep);
      schedule::Session::fromProto(resp->session(), &ctx->session_prep);
      if (ctx->conf.demoMode) {
        ctx->session_prep.data().aos = schedule::SessionData::dateFromString(resp->demo_aos().c_str());
        ctx->session_prep.data().los = ctx->session_prep.data().aos.addSecs(ctx->sessionOriginal_prep.duration());
      }

    default :
      break ;
  }

  return resp->state();
}

}

}
