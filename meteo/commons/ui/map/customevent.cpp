#include "customevent.h"

#include <google/protobuf/message.h>

const QEvent::Type meteo::map::CustomEvent::Custom = static_cast<QEvent::Type>(QEvent::registerEventType());

namespace meteo
{
namespace map
{
CustomEvent::CustomEvent(sub_t type) :
  QEvent(Custom),
  type_(type),
  message_(0)
{
}

CustomEvent::~CustomEvent()
{
  delete message_;
  message_ = 0;
}

CustomEvent::sub_t CustomEvent::customType() const
{
  return type_;
}

void CustomEvent::setOwnedMessage(google::protobuf::Message* message)
{
  delete message_;
  message_ = message;
}

google::protobuf::Message* CustomEvent::ownedMessage()
{
  return message_;
}

} // map
} // meteo
