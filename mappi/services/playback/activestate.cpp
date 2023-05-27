#include "activestate.h"


namespace mappi {

namespace playback {

ActiveState::ActiveState(Context* ctx, StateMachine* fsm) :
  ReceptionState(ctx, fsm)
{
}

ActiveState::~ActiveState()
{
}

State::id_t ActiveState::branch(Primitive::id_t prim, Primitive::queue_t* input)
{
  switch (prim) {
    case Primitive::WARMING_COMPLETED : {
      // !!! начало приема
      int msec = ctx->session.msecsToLos(ctx->now());
      msec = (1000 < msec ? msec : 1000);
      fsm->startTimer_msec(msec);

      info_log << QObject::tr("reception start: %1, duration(%2, %3)")
        .arg(SessionData::dateToString(ctx->now()))
        .arg(ctx->session.duration())
        .arg(msec / 1000.0);
    }
    break ;

    case Primitive::TIMEOUT :
      info_log << QObject::tr("reception stop: %1")
        .arg(SessionData::dateToString(ctx->now()));

      notify(); // client stub

      input->append(Primitive::ACCEPT_SESSION);
      return State::FIND;

    case Primitive::SERVICE_MODE_ON :
      fsm->stopTimer();
      input->append(Primitive::SERVICE_MODE_ON);
      return State::SERVICE;

    default :
      break ;
  }

  return id();
}

void ActiveState::notify()
{
  conf::SessionBundle req;
  Session::toProto(ctx->session, req.mutable_session());
  *req.mutable_tle() = ctx->tleItem;
  if (ctx->demoMode)
    req.set_demo_aos(SessionData::dateToString(ctx->actualAos()).toStdString());

  QScopedPointer<rpc::Channel> ch(channel(settings::proto::kPretreatment));
  if (ch == nullptr) {
    warning_log << QObject::tr("%1 недоступна")
      .arg(global::serviceTitle(settings::proto::kPretreatment));
    return ;
  }

  QScopedPointer<conf::PretreatmentResponse> resp(ch->remoteCall(&conf::PretreatmentService::SessionCompleted, req, 10000, true));
  if (resp.isNull()) {
    error_log << QObject::tr("Сервис вернул пустой ответ");
    return ;
  }

  if (resp->result() == false) {
    if (resp->has_comment())
      error_log << resp->comment();

    return ;
  }

  info_log << QObject::tr("%1 оповещена")
    .arg(global::serviceTitle(settings::proto::kPretreatment));
}

}

}
