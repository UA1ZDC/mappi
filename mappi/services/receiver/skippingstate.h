#pragma once

#include "fsm.h"


namespace mappi {

namespace receiver {

/**
 * Пропуск (ожидания завершения) активного сеанса.
 * Состояние введено целенаправленно, чтобы при запросе состояния (health check) сервиса можно было отличить
 * успешный приём (состояние ACTIVE) от проблем возникших при запуске приёмника.
 */
class SkippingState :
  public ReceiverState
{
public :
  SkippingState(Context* ctx, StateMachine* fsm);
  virtual ~SkippingState();

  virtual State::id_t id() const { return State::SKIPPING; }
  virtual State::id_t branch(Primitive::id_t prim, Primitive::queue_t* input);
};

}

}
