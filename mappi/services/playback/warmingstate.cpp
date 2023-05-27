#include "warmingstate.h"


namespace mappi {

namespace playback {

WarmingState::WarmingState(Context* ctx, StateMachine* fsm) :
  ReceptionState(ctx, fsm)
{
}

WarmingState::~WarmingState()
{
}

State::id_t WarmingState::branch(Primitive::id_t prim, Primitive::queue_t* input)
{
  switch (prim) {
    case Primitive::WARMING_UP : {
      // если продолжительность сеанса больше времени затраченного на подготовку, уведомление сервисов приёма
      if (0 < (ctx->session.secsToLos(ctx->now()) - Context::WARMING_INTERVAL))
        notifyAll();    // client stub
      else
        warning_log << QObject::tr("Сеанс отклонен");

      int msec = ctx->session.msecsToAos(ctx->now());
      if (1000 < msec) {
        // с подготовкой
        fsm->startTimer_msec(msec);
        break ;
      }

      // без подготовки
    }

    // подготовка завершена, приём сеанса
    case Primitive::TIMEOUT :
      // !!! минутная готовность
      input->append(Primitive::WARMING_COMPLETED);
      return State::ACTIVE;

    case Primitive::SERVICE_MODE_ON :
      fsm->stopTimer();
      input->append(Primitive::SERVICE_MODE_ON);
      return State::SERVICE;

    default :
      break ;
  }

  return id();
}

void WarmingState::notifyAll()
{
  conf::SessionBundle req;
  Session::toProto(ctx->session, req.mutable_session());
  *req.mutable_tle() = ctx->tleItem;
  if (ctx->demoMode)
    req.set_demo_aos(SessionData::dateToString(ctx->actualAos()).toStdString());

  notifyReceiver(req);
  notifyAntenna(req);

  // info_log << QObject::tr("Сервисы приёма оповещены, warming");
}

void WarmingState::notifyReceiver(const conf::SessionBundle& req)
{
  QScopedPointer<rpc::Channel> ch(channel(settings::proto::kReceiver));
  if (ch == nullptr) {
    warning_log << QObject::tr("%1 недоступен")
      .arg(global::serviceTitle(settings::proto::kReceiver));
    return ;
  }

  QScopedPointer<conf::ReceiverResponse> resp(ch->remoteCall(&conf::ReceiverService::AcceptSession, req, 10000, true));
  if (resp.isNull()) {
    error_log << QObject::tr("Сервис вернул пустой ответ");
    return ;
  }

  debug_log << "receiver state:" << STATE_NAME(resp->state());

  if (resp->result() == false) {
    if (resp->has_comment())
      error_log << resp->comment();
  }
}

void WarmingState::notifyAntenna(const conf::SessionBundle& req)
{
  QScopedPointer<rpc::Channel> ch(channel(settings::proto::kAntenna));
  if (ch == nullptr) {
    warning_log << QObject::tr("%1 недоступен")
      .arg(global::serviceTitle(settings::proto::kAntenna));
    return ;
  }

  QScopedPointer<conf::AntennaResponse> resp(ch->remoteCall(&conf::AntennaService::AcceptSession, req, 10000, true));
  if (resp.isNull()) {
    error_log << QObject::tr("Сервис вернул пустой ответ");
    return ;
  }

  debug_log << "antenna state:" << STATE_NAME(resp->state());

  if (resp->result() == false) {
    if (resp->has_comment())
      error_log << resp->comment();
  }
}

}

}
