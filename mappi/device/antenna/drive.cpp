#include "drive.h"
#include "print.h"
#include <QMetaEnum>


namespace mappi {

namespace antenna {

Drive Drive::makeAzimut()
{
  Drive drive;
  drive.self = 0;
  drive.dst = 359;

  return drive;
}

Drive Drive::makeElevat()
{
  Drive drive;
  drive.id = ELEVAT;
  drive.self = 0;
  drive.dst = -1;

  return drive;
}

Drive::Drive() :
  id(AZIMUT),
  state(Drive::UNKNOWN),
  self(0),
  dst(0),
  seq(0)
{
}

Drive::~Drive()
{
}

float Drive::precision() const
{
  return ::fabs(::fabs(self) - ::fabs(dst));
}

QString Drive::toString() const
{
  // return QString("%1(%2/%3/%4, state:%5, seq:%6)")
  return QString("%1(%2/%3, state:%4, seq:%5)")
    .arg(QMetaEnum::fromType<mappi::antenna::Drive::id_t>().valueToKey(id))
    .arg(self, (id == AZIMUT ? FW_AZIMUT : FW_ELEVAT), 'f', 2)
    .arg(dst,  (id == AZIMUT ? FW_AZIMUT : FW_ELEVAT), 'f', 2)
    // .arg(precision(), FW_ELEVAT, 'f', 2)
    .arg(QMetaEnum::fromType<mappi::antenna::Drive::state_t>().valueToKey(state))
    .arg(seq);
}

}

}
