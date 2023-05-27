#include "catcher.h"


namespace mappi {

namespace antenna {

Catcher::Catcher(QObject* parent /*=*/) :
    Script(parent),
  try_(1)
{
}

Catcher::~Catcher()
{
}

bool Catcher::exec(Antenna* antenna)
{
  debug_log << QString("Catcher::exec");
  if (((1 < azimut.seq) && (1 < elevat.seq)) || (TRY_COUNT == try_)) {
    try_ = 1;

    return false;
  }

  debug_log << QString("Catcher::exec->STOP");
  antenna->stop();
  ++try_;

  return true;
}

}

}
