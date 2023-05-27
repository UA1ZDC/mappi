#include "smallant.h"


namespace mappi {

namespace receive {

SmallAnt::SmallAnt(QObject* parent /*=*/) :
  SuzhetAnt(conf::kSmallAntenna, parent)
{
}

SmallAnt::~SmallAnt()
{
}

bool SmallAnt::isInit()
{
  uint stat = 0xffffffff;
  bool ok = status(&stat);
  if (!ok) {
    stat = 0xffffffff;
    var(ok);
  }
  debug_log << "status" << QString("0x%1 (%2)").arg(QString::number(stat, 16).right(2)).arg(QString::number(stat, 2));

  if (ok && (stat & (kNotInit))) {
    ok = false;
  }

  return ok;
}

bool SmallAnt::setPower(bool v)
{
  // trc;
  bool ok = PrblAnt::setPower(v);
  if (!ok) {
    return false;
  }

  if (!v) {
    return true;
  }

  usleep(1000000);

  uint stat = 0xffffffff;
  status(&stat);
  if ((stat & 0xff00) == 0) {
    debug_log << "status" << QString("0x%1 (%2)").arg(QString::number(stat, 16).right(2)).arg(QString::number(stat, 2));
    return true;
  }

  ok = regenerate();
  if (!ok) {
    return false;
  }

  int timeout = 3 * 60;//запрос каждую секунду в течении 3 минут
  while ((stat & 0xff00) !=0 && timeout > 0) {
    usleep(1000000);
    ok = status(&stat);
    if (!ok) {
      stat = 0xffffffff;
      var(ok);
    }
    debug_log << "status" << QString("0x%1 (%2)").arg(QString::number(stat, 16).right(2)).arg(QString::number(stat, 2));
    --timeout;
  }

  var(timeout);

  return true;
}

bool SmallAnt::status(uint32_t* stat)
{
  if (!sendCommand(0, kCmd_Status)) {
    return false;
  }

  QByteArray cmd = readStatus();
  if (cmd.size() < 3)
    return false;

  *stat = (uchar(cmd[0]) << 16) + (uchar(cmd[1]) << 8) + uchar(cmd[2]);

  return true;
}

bool SmallAnt::regenerate()
{
  return sendCommand(0, kCmd_Regen);
}

bool SmallAnt::setTravelPosition()
{
  return sendCommand(0, kCmd_SetTravel);
}

bool SmallAnt::setWorkPosition()
{
  return sendCommand(0, kCmd_SetWork);
}

}

}
