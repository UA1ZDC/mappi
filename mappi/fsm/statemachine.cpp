#include "statemachine.h"
#include <QTimerEvent>


namespace mappi {

namespace fsm {

StateMachine::StateMachine(QObject* parent /*=*/) :
    QObject(parent),
  timerId_(-1),
  isSingleShot_(false)
{
}

StateMachine::~StateMachine()
{
}

void StateMachine::proceed(Primitive::id_t prim)
{
  Primitive::queue_t input;
  input.append(prim);

  while (!input.isEmpty()) {
    State::ptr_t state = currentState();
    Primitive::id_t item = input.takeFirst();

    debug_log << QObject::tr(" IN state: %1, prim: %2")
      .arg(STATE_NAME(state->id()))
      .arg(PRIM_NAME(item));

    stateId_ = state->branch(item, &input);

    debug_log << QObject::tr("OUT state: %1")
      .arg(STATE_NAME(currentState()->id()));
  }
}

State::ptr_t StateMachine::currentState()
{
  return statePool_[stateId_];
}

void StateMachine::syncTime()
{
  QThread::usleep((1000 - QTime::currentTime().msec()) * 1000); // microsecs
}

void StateMachine::startTimer_sec(int sec, bool isSingleShot /*=*/)
{
  startTimer_msec(sec * 1000, isSingleShot);
}

void StateMachine::startTimer_msec(int msec, bool isSingleShot /*=*/)
{
  if (timerId_ != -1)
    stopTimer();

  isSingleShot_ = isSingleShot;
  timerId_ = startTimer(msec, Qt::PreciseTimer);

  debug_log << QObject::tr("timer start (id: %1, secs: %2, type: %3)")
    .arg(timerId_)
    .arg(msec / 1000.0)
    .arg(isSingleShot_ ? "single" : "repeat");
}

void StateMachine::stopTimer()
{
  if (timerId_ == -1)
    return ;

  killTimer(timerId_);
  debug_log << QObject::tr("timer stop  (id: %1)").arg(timerId_);

  timerId_ = -1;
}

void StateMachine::timerEvent(QTimerEvent* event)
{
  if (event->timerId() == timerId_) {
    if (isSingleShot_)
      stopTimer();

    proceed(Primitive::TIMEOUT);
  }
}

void StateMachine::registerState(State* state, bool isInitial /*=*/)
{
  statePool_.insert(state->id(), State::ptr_t(state));
  if (isInitial)
    stateId_ = state->id();
}

}

}
