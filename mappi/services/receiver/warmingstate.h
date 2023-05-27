#pragma once

#include "fsm.h"


namespace mappi {

namespace receiver {

/**
 * Подготовка - настройка приёмника для приёма (вырожденное состояние, т.к. параметры устанавливаются в момент включения приёмника),
 * инициализирует файловое хранилище и спутник для активного сеанса.
 *
 * В случае ошибки при работе с файловым хранилищем сеанс будет пропущен.
 */
class WarmingState :
  public ReceiverState
{
public :
  WarmingState(Context* ctx, StateMachine* fsm);
  virtual ~WarmingState();

  virtual State::id_t id() const { return State::WARMING; }
  virtual State::id_t branch(Primitive::id_t prim, Primitive::queue_t* input);

private :
   bool executeCheckList(Context* ctx);
};

}

}
