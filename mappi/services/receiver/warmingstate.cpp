#include "warmingstate.h"
#include "filestorage.h"


namespace mappi {

namespace receiver {

WarmingState::WarmingState(Context* ctx, StateMachine* fsm) :
  ReceiverState(ctx, fsm)
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

      fsm->syncTime();
      int msec = ctx->session.msecsToAos();

      // с ожиданием
      if (1000 < msec) {
        fsm->startTimer_msec(msec);
        break ;
      }

      // без ожидания, начало сеанса пропущено
      input->append(Primitive::ALERT_IND);
      return State::ACTIVE;
    }

    case Primitive::TIMEOUT :
      input->append(Primitive::WARMING_COMPLETED);
      return State::ACTIVE;

    case Primitive::SERVICE_MODE_ON :
      fsm->stopTimer();
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

  // такое возможно:
  // - если править конфигурацию вручную;
  // - сервисы запущены на разных хостах и конфигурация не синхронизирована
  if (ctx->conf.hasParameters(name) == false) {
    STATE_ERROR(QObject::tr("Не заданы параметры приёма, спутник: %1").arg(name));
    return false;
  }

  FileStorage fs(ctx);
  return fs.create();
}

}

}
