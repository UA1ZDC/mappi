#include "state.h"


namespace mappi {

namespace fsm {

State::State(StateMachine* fsm)
{
  this->fsm = fsm;
}

State::~State()
{
  fsm = nullptr;
}

rpc::Channel* State::channel(settings::proto::ServiceCode code)
{
  return global::serviceChannel(code);
}

}

}
