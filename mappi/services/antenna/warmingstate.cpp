#include "warmingstate.h"
#include <mappi/device/antenna/track.h>


namespace mappi {

namespace antenna {

WarmingState::WarmingState(Context* ctx, StateMachine* fsm) :
  AntennaState(ctx, fsm)
{
}

WarmingState::~WarmingState()
{
}

State::id_t WarmingState::branch(Primitive::id_t prim, Primitive::queue_t* input)
{
  switch (prim) {
    case Primitive::WARMING_UP : {
      if (executeCheckList(ctx) == false) {
        input->append(Primitive::FAIL_IND);
        return State::SKIPPING;
      }

      info_log << QObject::tr("Подготовка завершена успешно");

      // с ожиданием
      int msec = ctx->session.msecsToAos();
      if (1000 < msec) {
        const MnSat::TrackTopoPoint& point = ctx->track[0];
        ctx->direct->setPosition(point, true);
        ctx->direct->run(ctx->antenna);

        // WARNING чтобы успеть синхронизировать время и траекторию
        fsm->startTimer_msec(msec - 1000);

        break ;
      }

      // без ожидания, начало сеанса пропущено
      input->append(Primitive::ALERT_IND);
      return State::ACTIVE;
    }

    case Primitive::TIMEOUT :
      ctx->direct->terminate();
      input->append(Primitive::WARMING_COMPLETED);

      return State::ACTIVE;

    case Primitive::SERVICE_MODE_ON :
      fsm->stopTimer();
      ctx->direct->terminate();

      return State::SERVICE;

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

bool WarmingState::executeCheckList(Context* ctx)
{
  MnSat::STLEParams tleParam = {
    ctx->tleItem.title().c_str(),
    ctx->tleItem.line_1().c_str(),
    ctx->tleItem.line_2().c_str()
  };

  QString name = ctx->session.data().satellite;
  if (ctx->satellite.readTLE(tleParam) == false) {
    STATE_ERROR(QObject::tr("Недействительные tle-параметры, спутник: %1").arg(name));
    return false;
  }

  // расчет траектории
  bool isOk = ctx->satellite.trajectory(ctx->sessionOriginal.data().aos,
    ctx->sessionOriginal.data().los,
    ctx->conf.profile.timeSlot,
    ctx->conf.point,
    ctx->track
  );

  if (!isOk) {
    STATE_ERROR(QObject::tr("Ошибка расчета траектории"));
    return false;
  }

  debug_log << "original" << TrackUtils::info(ctx->track);

  TrackUtils::correction(ctx->track, MnMath::deg2rad(ctx->conf.azimutCorrect));
  debug_log << "after correction" << TrackUtils::info(ctx->track);

  TrackUtils::gap_removal(ctx->track);
  debug_log << "after gap_removal" << TrackUtils::info(ctx->track);

  isOk = TrackUtils::adaptation(ctx->track, ctx->conf.profile.azimut);
  if (!isOk) {
    STATE_ERROR(QObject::tr("Недопустимая траектория сопровождения"));
    return false;
  }
  debug_log << "after adaptation" << TrackUtils::info(ctx->track);

  return true;
}

}

}
