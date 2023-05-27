#include "findstate.h"


namespace mappi {

namespace playback {

static const int TRY_AGAIN_TIMEOUT = 5;

FindState::FindState(Context* ctx, StateMachine* fsm) :
  ReceptionState(ctx, fsm)
{
}

FindState::~FindState()
{
}

State::id_t FindState::branch(Primitive::id_t prim, Primitive::queue_t* input)
{
  switch (prim) {
    case Primitive::TIMEOUT :
    case Primitive::SERVICE_MODE_OFF :
    case Primitive::ACCEPT_SESSION : {
      if (find() == false) {    // client stub
        STATE_ERROR(QObject::tr("Сеанс не найден, новая попытка"));
        fsm->startTimer_sec(TRY_AGAIN_TIMEOUT);
        break ;
      }

      info_log << QString("\n%1\n%2\n%3\n%4")
        .arg(ctx->session.data().toString())
        .arg(ctx->tleItem.title().c_str())
        .arg(ctx->tleItem.line_1().c_str())
        .arg(ctx->tleItem.line_2().c_str());

      input->append(Primitive::WAIT_IND);
      return State::PREWAIT;
    }

    case Primitive::SERVICE_MODE_ON :
      fsm->stopTimer();
      input->append(Primitive::SERVICE_MODE_ON);
      return State::SERVICE;

    default :
      break ;
  }

  return id();
}

bool FindState::find()
{
  QSharedPointer<rpc::Channel> ch(channel(settings::proto::kSchedule));
  if (ch == nullptr) {
    warning_log << QObject::tr("%1 недоступен")
      .arg(global::serviceTitle(settings::proto::kSchedule));
    return false;
  }

  conf::SessionResponse* sr;
  if (ctx->demoMode) {
    debug_log << "next session";
    conf::Session session;
    Session::toProto(ctx->session, &session);
    sr = ch->remoteCall(&conf::ScheduleService::NextSession, session, 10000, true);
  } else {
    debug_log << "near session";
    sr = ch->remoteCall(&conf::ScheduleService::NearSession, Dummy(), 10000, true);
  }

  QSharedPointer<conf::SessionResponse> resp(sr);
  if (resp.isNull()) {
    error_log << QObject::tr("Сервис вернул пустой ответ");
    return false;
  }

  if (resp->result() == false) {
    if (resp->has_comment())
      error_log << resp->comment().c_str();

    return false;
  }

  Session::fromProto(resp->session(), &ctx->session);
  ctx->tleItem = resp->tle();

  if (ctx->session.data().isDefault()) {
    // в demo-режиме расписание формируется по кругу
    if (ctx->demoMode) {
      ctx->timeOffset = 0;
      debug_log << "time offset:" << ctx->timeOffset;
    }

    return false;
  }

  return true;
}

}

}
