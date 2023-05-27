#pragma once

#include "state.h"
#include <QMetaEnum>

#define PRIM_NAME(v) (QMetaEnum::fromType<mappi::fsm::Primitive::id_t>().valueToKey(v))
#define STATE_NAME(v) (QMetaEnum::fromType<mappi::fsm::State::id_t>().valueToKey(v))
#define CURRENT_STATE(v) (v->currentState()->id())


namespace mappi {

namespace fsm {

/**
 * FSM - машина состояний, служит внешним интерфейсом для работы с набором состояний.
 * Все взаимодействие с FSM сводится к отправки примитив в метод proceed.
 * Текущее состояние машины определяет как будет обработана примитива.
 * Все данные которые необходимо сохранить между переходами, сохраняются в контексте машины состояний.
 *
 * !!! WARNING
 * FSM работает с таймером, поэтому НЕЛЬЗЯ работать с машиной состояний из разных потоков.
 */
class StateMachine :
  public QObject
{
  Q_OBJECT
public :
  explicit StateMachine(QObject* parent = nullptr);
  virtual ~StateMachine();

public :
  State::ptr_t currentState();

  void syncTime();
  void startTimer_sec(int sec, bool isSingleShot = true);
  void startTimer_msec(int msec, bool isSingleShot = true);
  void stopTimer();

public slots :
  virtual void proceed(Primitive::id_t prim);

protected :
  virtual void timerEvent(QTimerEvent* event);

  void registerState(State* state, bool isInitial = false);

private :
  int timerId_;
  bool isSingleShot_;
  State::id_t stateId_;
  State::pool_t statePool_;
};

}

}
