#pragma once

#include "fsm.h"


namespace mappi {

namespace playback {

/**
 * Подготовка (прогрев) - временной период в течении которого должно быть настроено все оборудование для приёма сеанса.
 * Сейчас это вырожденное состояние отвечает только за уведомление сервисов, т.к. настройку выполняют сами сервисы приёма.
 * Приём сеанса игнорируется (без уведомления сервисов приёма), когда продолжительность сеанса оказывается меньше времени подготовки.
 */
class WarmingState :
  public ReceptionState
{
public :
  explicit WarmingState(Context* ctx, StateMachine* fsm);
  virtual ~WarmingState();

  virtual State::id_t id() const { return State::WARMING; }
  virtual State::id_t branch(Primitive::id_t prim, Primitive::queue_t* input);

private :
  void notifyAll();
  void notifyReceiver(const conf::SessionBundle& req);
  void notifyAntenna(const conf::SessionBundle& req);
};

}

}
