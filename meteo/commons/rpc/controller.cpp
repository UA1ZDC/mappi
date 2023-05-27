#include "controller.h"

#include <quuid.h>

#include "channel.h"


namespace meteo {
namespace rpc {

Controller::Controller( Channel* ch )
  : channel_(ch),
  callid_( QUuid::createUuid().toByteArray() )
{
}

Controller::~Controller()
{
  channel_->controllers_.removeAll(this);
}

bool Controller::isServerSide() const
{
  return channel_->isServerSide();
}

}
}
