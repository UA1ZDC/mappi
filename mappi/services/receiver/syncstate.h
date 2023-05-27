#pragma once

#include "fsm.h"


namespace mappi {

namespace receiver {

/**
 * Cинхронизация - состояние в котором находится сервис только в момент запуска.
 * Состояние необходимо чтобы не быть привязанным к порядку запуска сервисов, например:
 * - receiver.service запущен ПЕРВЫМ по отношению к playback.service, уведомление об активном сеансе будет ПОЛУЧЕНО.
 * - receiver.service запущен ВТОРЫМ по отношению к playback.service, уведомление об активном сеансе будет ПРОПУЩЕНО.
 *   По этой причине НЕОБХОДИМО выполнить запрос текущего состояния playback.service.
 */
class SyncState :
  public ReceiverState
{
public :
  SyncState(Context* ctx, StateMachine* fsm);
  virtual ~SyncState();

  virtual State::id_t id() const { return State::SYNC; }
  virtual State::id_t branch(Primitive::id_t prim, Primitive::queue_t* input);

private :
  ServiceState sync();
};

}

}
