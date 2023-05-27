#pragma once

#include "fsm.h"


namespace mappi {

namespace playback {

/**
 * Поиск сеанса для приёма, запрашивается у сервиса расписания.
 * В случае недоступности сервиса расписания, попытки повторяются с определенной периодичностью.
 * Если приём запущен в демонстрационном режиме - запрашивается следующий сеанс, иначе ближайший.
 */
class FindState :
  public ReceptionState
{
public :
  explicit FindState(Context* ctx, StateMachine* fsm);
  virtual ~FindState();

  virtual State::id_t id() const { return State::FIND; }
  virtual State::id_t branch(Primitive::id_t prim, Primitive::queue_t* input);

private :
  bool find();
};

}

}
