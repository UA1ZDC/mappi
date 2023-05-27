#include "activestate.h"
// #include <mappi/proto/reception.pb.h>


namespace mappi {

namespace antenna {

ActiveState::ActiveState(Context* ctx, StateMachine* fsm) :
  AntennaState(ctx, fsm)
{
}

ActiveState::~ActiveState()
{
}

State::id_t ActiveState::branch(Primitive::id_t prim, Primitive::queue_t* input)
{
  switch (prim) {
    case Primitive::ALERT_IND :
    case Primitive::WARMING_COMPLETED : {
      // TODO включение облучателя
      // ctx->antenna->feedhorn(true);

      fsm->syncTime();

      actualAos = ctx->now();
      if (ctx->conf.demoMode)
        actualAos = ctx->sessionOriginal.data().aos.addSecs(::abs(ctx->session.secsToAos(ctx->now())));

      Drive azimut;
      Drive elevat;
      Feedhorn feedhorn;
      ctx->antenna->snapshot(&azimut, &elevat, &feedhorn);
      info_log << feedhorn.toString();
      info_log << QObject::tr("current position: %1;%2")
        .arg(azimut.self)
        .arg(elevat.self);

      ctx->tracker->setTrack(ctx->track, actualAos);
      if (ctx->tracker->run(ctx->antenna)) {
        int msec = ctx->session.msecsToLos(ctx->now());
        fsm->startTimer_msec(msec);

        info_log << QObject::tr("antenna start: %1, duration(%2, %3)")
          .arg(SessionData::dateToString(actualAos))
          .arg(ctx->session.duration())
          .arg(msec / 1000.0);

        break ;
      }

      // если не получилось запустить антенну
      input->append(Primitive::FAIL_IND);
      return State::SKIPPING;
    }

    case Primitive::TIMEOUT :
      // TODO отключение облучателя
      // ctx->antenna->feedhorn(false);
      ctx->tracker->terminate();

      info_log << QObject::tr("antenna stop: %1")
        .arg(SessionData::dateToString(ctx->sessionOriginal.data().los));

      // уведомление о новом сеансе получено раньше завершения текущего
      if (!ctx->isReplay())
        input->append(Primitive::ACCEPT_SESSION);

      return State::IDLING;

    case Primitive::SERVICE_MODE_ON :
      // TODO отключение облучателя
      // ctx->antenna->feedhorn(false);
      fsm->stopTimer();
      ctx->tracker->terminate();
      info_log << QObject::tr("antenna terminate");

      return State::SERVICE;

    default :
      break ;
  }

  return id();
}

}

}
