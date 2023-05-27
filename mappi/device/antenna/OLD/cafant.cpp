#include "cafant.h"

#define ERROR_AZIMUT_MOTOR_MASK  (1 << 15)
#define ERROR_ELEVAT_MOTOR_MASK  (1 << 14)
#define ERROR_ELEVAT_ENCODE_MASK (1 << 13)
#define ERROR_AZIMUT_ENCODE_MASK (1 << 12)
#define ERROR_ANTENNA_MASK (ERROR_AZIMUT_MOTOR_MASK &  \
                            ERROR_ELEVAT_MOTOR_MASK &  \
                            ERROR_ELEVAT_ENCODE_MASK & \
                            ERROR_AZIMUT_ENCODE_MASK)


namespace mappi {

namespace receive {

static const QByteArray kCmd_SetAngle = QByteArray("01");
static const QByteArray kCmd_Move = QByteArray("02");
static const QByteArray kCmd_Stop = QByteArray("03");
static const QByteArray kCmd_SetTravel = QByteArray("04");

static const QByteArray kStartPack = QByteArray("5555");
static const QByteArray kStopPack = QByteArray::fromHex("0D0A");
static const uint8_t kLengthPack = 30;


CafAnt::CafAnt(QObject* parent /*=*/) :
    PrblAnt(conf::kCafAntenna, parent)
{
  startAsyncRead(kStartPack, kStopPack, kLengthPack);
  connect(this, &PrblAnt::packetReady, this, &CafAnt::parseReadPack);
}

CafAnt::~CafAnt()
{
}

bool CafAnt::currentAzimut(float *az)
{
  if (!_isCoordOk) return false;
  *az = _azimut;
  return true;
}

bool CafAnt::currentElevat(float *el)
{
  if (!_isCoordOk) return false;
  *el = _elev;
  return true;
}

bool CafAnt::currentAngles(float *azimut, float *eleval)
{
  if (azimut == nullptr || eleval == nullptr) {
    return false;
  }

  if (!_isCoordOk) return false;
  *azimut = _azimut;
  *eleval = _elev;
  return true;
}

bool CafAnt::status(uint32_t* stat)
{
  if (!_isCoordOk) return false;
  *stat = _status;
  return true;
}


bool CafAnt::setAzimut(float angle, float* curAzimut /*=*/)
{
  bool ok = setPosition(angle, _elev, false);

  if (nullptr != curAzimut) {
    *curAzimut = _azimut;
  }

  return ok;
}

bool CafAnt::setElevat(float angle, float* curEl /*=*/)
{
  bool ok = setPosition(_elev, angle, false);

  if (nullptr != curEl) {
    *curEl = _elev;
  }

  return ok;
}

bool CafAnt::setPosition(float azimut, float elev, bool wait)
{
  Q_UNUSED(wait);

  azimut += zeroAzimut();
  if (azimut < azMin() || azimut > azMax()) {
    error_log << QObject::tr("Заданный угол %1 (поправка = %2) выходит за пределы рабочего диапазона [%3, %4]")
      .arg(azimut).arg(zeroAzimut()).arg(azMin()).arg(azMax());
    return false;
  }

  elev += zeroElevat();
  if (elev < elMin() || elev > elMax()) {
    error_log << QObject::tr("Заданный угол %1 (поправка = %2) выходит за пределы рабочего диапазона [%3, %4]")
      .arg(elev).arg(zeroElevat()).arg(elMin()).arg(elMax());
    return false;
  }

  int azRate = dsaMax();
  int elRate = dseMax();

  QByteArray rate;
  rate.append(QString::number(azRate*10, 'f', 0).rightJustified(3, '0'));
  rate.append(QString::number(elRate*10, 'f', 0).rightJustified(3, '0'));

  if (!sendCommand(kCmd_SetAngle, rate, angle2ascii(azimut), angle2ascii(elev))) {
    return false;
  }

  return true;
}

//TODO переход через 360 азимута?
//в режиме наведения
bool CafAnt::movePosition(float azimut, float elev, float* curAzimut /*=*/, float* curElev /*=*/)
{
  azimut += zeroAzimut();
  if (azimut < azMin() || azimut > azMax()) {
    error_log << QObject::tr("Заданный угол %1 (поправка = %2) выходит за пределы возможного диапазона [%3, %4]")
      .arg(azimut).arg(zeroAzimut()).arg(azMin()).arg(azMax());
    return false;
  }

  elev += zeroElevat();
  if (elev < elMin() || elev > elMax()) {
    error_log << QObject::tr("Заданный угол %1 (поправка = %2) выходит за пределы возможного диапазона [%3, %4]")
      .arg(elev).arg(zeroElevat()).arg(elMin()).arg(elMax());
    return false;
  }

  QByteArray time(6, '0'); //TODO
  if (!sendCommand(kCmd_Move, time, angle2ascii(azimut), angle2ascii(elev))) {
    return false;
  }

  if (nullptr != curAzimut) {
    *curAzimut = _azimut;
  }

  if (nullptr != curElev) {
    *curElev = _elev;
  }

  return true;
}

bool CafAnt::stop(float* azimut, float* elevation)
{
  bool ok =sendCommand(kCmd_Stop, QByteArray(6, '0'), QByteArray(6, '0'), QByteArray(6, '0'));

  *azimut = _azimut;
  *elevation = _elev;

  return ok;
}

bool CafAnt::setTravelPosition()
{
  if (!sendCommand(kCmd_SetTravel, QByteArray(6, '0'), QByteArray(6, '0'), QByteArray(6, '0'))) {
    return false;
  }
  return true;
}

bool CafAnt::sendCommand(const QByteArray& cmd,   const QByteArray& data1,
                         const QByteArray& data2, const QByteArray& data3)
{
  _currentPack += 1;
  if (_currentPack >= 100) {
    _currentPack = 1;
  }

  QByteArray pack = QByteArray::number(_currentPack).rightJustified(2, '0') + cmd + data1 + data2 + data3;
  QByteArray crc = createCrc(pack);

  pack = kStartPack + pack + crc + kStopPack;

  pack = pack.toUpper();
  var(pack);

  //var(pack.size());

  bool ok = writeBytes(pack);

  return ok;
}

//! преобразование угла в ascii формат
QByteArray CafAnt::angle2ascii(float angle)
{
  QByteArray ascii;
  if (angle >= 0) {
    ascii.append(0x2b);
  } else {
    ascii.append(0x2d);
  }

  QString str = QString::number(fabs(angle)*100, 'f', 0).rightJustified(5, '0');
  ascii.append(str);

  return ascii;
}

float CafAnt::ascii2angle(const QByteArray& ba)
{
  if (ba.size() < 6) return 0;

  float angle = ba.mid(1).toInt() / 100.;

  if (ba[0] == '-') {
    angle *= -1;
  }

  return angle;
}

//Контрольная сумма, является дополнением до 2 суммы всех байт с отбрасывание переноса
//CheckSum8 Xor
// QByteArray CafAnt::createCrc(const QByteArray& data)
// {
//   uint8_t lrc = data[0];

//   for (int idx = 1; idx < data.size(); idx++) {
//     lrc ^= data[idx];
//   }

//   QByteArray ba;
//   ba.append(QString::number(lrc));

//   // var(lrc);
//   // var(ba);
//   // var(ba.toHex());

//   return ba; //.toHex();
// }


//Контрольная сумма (LRC, CheckSum8 2s Complement)
QByteArray CafAnt::createCrc(const QByteArray& data)
{
  uint8_t lrc = data[0];

  //var(data);

  for (int idx = 1; idx < data.size(); idx++) {
    lrc += data[idx];
  }
  lrc = (256 - lrc) & 0xff;

  QByteArray ba;
  ba.append(QString::number(lrc, 16).rightJustified(2, '0'));

  // var(lrc);
  // var(ba);
  //var(ba.toHex());

  return ba;
}


//TODO обработка ошибки
void CafAnt::parseReadPack(const QByteArray& ba)
{
  QByteArray crc = createCrc(ba.mid(4, 22));
  if (crc.toUpper() != ba.mid(26, 2).toUpper()) {
    error_log << QObject::tr("Ошибка контрольной суммы пакета. Ожидаемая = %1, в пакете = %2")
      .arg(QString(crc)).arg(QString(ba.mid(26,2)));
  }

  bool ok;
  int curPack = ba.mid(4, 2).toInt(&ok);
  if (!ok) {
    _isCoordOk = false;
    return;
  }

  //TODO check curPack и _currentPack
  if (curPack != _currentPack) {
    error_log << QObject::tr("Номер отправленного и полученного пакета не совпадает %1, %2").arg(_currentPack).arg(curPack);
  }

  _status = ba.mid(6, 4).toInt(&ok, 16);
  if (_status & ERROR_ANTENNA_MASK) {
    error_log << QObject::tr("Ошибка антенны. Статус = %1").arg(QString::number(_status, 2).rightJustified(16, '0'));
    _isCoordOk = false;
    return;
  }

  _azimut = ascii2angle(ba.mid(10, 6)) - zeroAzimut();
  _elev   = ascii2angle(ba.mid(16, 6)) - zeroElevat();

  // debug_log << "stat" << QString::number(_status, 2).rightJustified(16, '0') << "az" << _azimut << "el" << _elev << "crc" << crc;

  _isCoordOk = true;
}

}

}
