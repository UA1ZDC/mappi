#include "feedhorn.h"


namespace mappi {

namespace antenna {

Feedhorn::Feedhorn() :
  turnOn_(false)
{
}

Feedhorn::~Feedhorn()
{
}

QString Feedhorn::toString() const
{
  QString str;
  for (auto it : output_)
    str += QString::number(it);

  return QString("feedhorn(turn_on: %1, output(%2))")
    .arg(turnOn_)
    .arg(str);
}

}

}
