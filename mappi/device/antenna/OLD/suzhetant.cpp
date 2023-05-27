#include "suzhetant.h"

#define AZIMUT_ID_MASK 0x0
#define ELEVATION_ID_MASK 0x40

// для получения угла из делений и наоборот
// #define AZIMUT_MAX_ANGLE_SCALE     720.0
// #define AZIMUT_MAX_DIVISION_SCALE 8191.0
// #define ELEV_MAX_ANGLE_SCALE       360.0
// #define ELEV_MAX_DIVISION_SCALE   8191.0

#define NEED_POS_PRECISION 0.5 // градусов

#define TIMEOUT 5000 // мкс
#define TIMEOUT_ms 500 // мс


namespace mappi {

namespace receive {

SuzhetAnt::SuzhetAnt(conf::AntennaType type, QObject* parent /*=*/) :
  PrblAnt(type, parent)
{
}

SuzhetAnt::SuzhetAnt(QObject* parent /*=*/) :
  PrblAnt(conf::kSuzhetAntenna, parent)
{
}

SuzhetAnt::~SuzhetAnt()
{
}

//! Текущие углы
bool SuzhetAnt::currentAngles(float* azimut, float* eleval)
{
  if (azimut == nullptr || eleval == nullptr) {
    return false;
  }

  bool ok = false;
  float angle;
  for(auto idx = 0; idx < 15; ++idx) {
    ok = currentAzimut(&angle);
    if (ok) break;
    usleep(100000);
  }

  if (!ok) {
    error_log << QObject::tr("Ошибка получения текущего азимута антенны");
    //Q_EMIT(checked(ok));
    return false;
  }

  *azimut = angle;

  for(auto idx = 0; idx < 5; ++idx) {
    ok = currentElevat(&angle);
    if (ok) break;
    usleep(100000);
  }

  if (!ok) {
    error_log << QObject::tr("Ошибка получения текущего угла места антенны");
    return false;
  }

 *eleval = angle;

 return true;
}

//! Получить текущий азимут
bool SuzhetAnt::currentAzimut(float* az)
{
  bool ok = currentAngle(AZIMUT_ID_MASK, conf().az_angle_scale(), conf().az_division_scale(), az);
  if (ok) {
    *az -= zeroAzimut();
  }
  return ok;
}

//! Получить текущий угол места
bool SuzhetAnt::currentElevat(float* el)
{
  bool ok = currentAngle(ELEVATION_ID_MASK, conf().el_angle_scale(), conf().el_division_scale(), el);
  if (ok) {
    *el -= zeroElevat();
  }
  return ok;
}

//! Получить текущий угол места
/*!
  \param id        ID двигателя
  \param angScale  Максимальный угол на шкале
  \param divisionScale  Количество делений
  \param angle     Текущий угол

  \return
*/
bool SuzhetAnt::currentAngle(uint8_t id, float angScale, float divisionScale, float* angle)
{
  *angle = 0;
  if (!sendCommand(id, kCmd_GetAngle)) {
    return false;
  }

  int16_t division;
  if (!readPortAngle(&division)) {
    return false;
  }

  *angle = division * angScale / divisionScale;
  return true;
}

//! Установка положения антенны
/*!
  \param azimut Азимут
  \param elev Угол места
  \param wait true - подождать установления положения
  \return false - при ошибке установки положения или при выходе по таймауту
*/
bool SuzhetAnt::setPosition(float azimut, float elev, bool wait)
{
  float curAz, curEl;
  bool ok = setAzimut(azimut, &curAz);
  ok &= setElevat(elev, &curEl);

  if (!ok) return false;

  if (azimut < 0) { azimut += 360; }
  if (curAz  < 0) { curAz += 360; }
  if (curAz  > 360) { curAz -= 360; }

  int timeout = 20*2; //каждые 0,5 сек в течении 20 сек
  if (wait) {
    while (timeout-- > 0 && 
        (fabs(curAz - azimut) > NEED_POS_PRECISION || fabs(curEl - elev) > NEED_POS_PRECISION)) {
      usleep(500000);//500 мс
      currentAzimut(&curAz);
      currentElevat(&curEl);
      if (curAz  < 0) { curAz += 360; }
      if (curAz  > 360) { curAz -= 360; }
      debug_log << timeout << "set start az=" << curAz << azimut
                << "el=" << curEl << elev << fabs(curAz - azimut) << fabs(curEl - elev);
    }
  }

  debug_log << "set pos = " << (timeout > 0);

  return timeout > 0;
}

bool SuzhetAnt::setAzimut(float angle, float* curAzimut /*=*/)
{
  angle += zeroAzimut();
  if (angle < azMin() || angle > azMax()) {
    error_log << QObject::tr("Заданный угол %1 (поправка = %2) выходит за пределы возможного диапазона [%3, %4]")
      .arg(angle).arg(zeroAzimut()).arg(azMin()).arg(azMax());
    return false;
  }

  bool ok = setAngle(AZIMUT_ID_MASK, conf().az_angle_scale(), conf().az_division_scale(), angle, curAzimut);
  if (ok && 0 != curAzimut) {
    *curAzimut -= zeroAzimut();
  }

  return ok;
}

bool SuzhetAnt::setElevat(float angle, float* curEl /*=*/)
{
  angle += zeroElevat();
  if (angle < elMin() || angle > elMax()) {
    error_log << QObject::tr("Заданный угол %1 (поправка = %2) выходит за пределы возможного диапазона [%3, %4]")
      .arg(angle).arg(zeroElevat()).arg(elMin()).arg(elMax());
    return false;
  }

  bool ok = setAngle(ELEVATION_ID_MASK, conf().el_angle_scale(), conf().el_division_scale(), angle, curEl);
  if (ok && 0 != curEl) {
    *curEl -= zeroElevat();
  }

  return ok;
}

//! Установка угла
/*!
  \param id        ID двигателя
  \param angScale  Максимальный угол на шкале
  \param divisionScale  Количество делений
  \param angle     Угол, на который установить
  \param curAngle  Текущий угол
*/
bool SuzhetAnt::setAngle(uint8_t id, float angScale, float divisionScale, float angle, float* curAngle /*= */)
{
  int16_t division  = int16_t(round((angle * divisionScale) / angScale)) & 0x1FFF;
  debug_log << "SET" << division * angScale / divisionScale;

  if (!sendCommand(id, kCmd_SetAngle, division)) {
    return false;
  }
  int16_t curDiv;
  if (!readPortAngle(&curDiv)) {
    return false;
  }

  float current = curDiv * angScale / divisionScale;

  //  debug_log << "set angle=" << angle << "cur=" << current << "div=" << division << "curDiv=" << curDiv;
  if (curAngle != 0) {
    *curAngle = current;
  }

  return true;
}

bool SuzhetAnt::movePosition(float azimut, float elevat, float* curAzimut /*=*/, float* curEl /*=*/)
{
  if (! moveAzimut(azimut, curAzimut)) {
    return false;
  }
  if (! moveElevat(elevat, curEl)) {
    return false;
  }
  return true;
}

bool SuzhetAnt::moveAzimut(float angle, float* curAzimut /*=*/)
{
  angle += zeroAzimut();
  if (angle < azMin() || angle > azMax()) {
    error_log << QObject::tr("Заданный угол %1 (поправка = %2) выходит за пределы возможного диапазона [%3, %4]")
      .arg(angle).arg(zeroAzimut()).arg(azMin()).arg(azMax());
    return false;
  }

  bool ok = move(AZIMUT_ID_MASK, conf().az_angle_scale(), conf().az_division_scale(), angle, curAzimut);
  if (ok && 0 != curAzimut) {
    *curAzimut -= zeroAzimut();
  }

   return ok;
}

bool SuzhetAnt::moveElevat(float angle, float* curEl /*=*/)
{
  angle += zeroElevat();
  if (angle < elMin() || angle > elMax()) {
    error_log << QObject::tr("Заданный угол %1 (поправка = %2) выходит за пределы возможного диапазона [%3, %4]")
      .arg(angle).arg(zeroElevat()).arg(elMin()).arg(elMax());
    return false;
  }

  bool ok = move(ELEVATION_ID_MASK, conf().el_angle_scale(), conf().el_division_scale(), angle, curEl);
  if (ok && 0 != curEl) {
    *curEl -= zeroElevat();
  }

  return ok;
}

//! Двигаться с заданной скоростью
/*!
  \param id        ID двигателя
  \param angScale  Максимальный угол на шкале
  \param divisionScale  Количество делений
  \param angle     Угол, на который установить
  \param curAngle  Текущий угол
*/
bool SuzhetAnt::move(uint8_t id, float angScale, float divisionScale, float angle, float *curAngle /*=*/)
{
  int16_t division  = int16_t(round((angle * divisionScale) / angScale)) & 0x1FFF;

  debug_log << "MOVE" <<division * angScale / divisionScale;

  if (!sendCommand(id, kCmd_Move, division)) {
    return false;
  }

  int16_t curDiv;
  if (!readPortAngle(&curDiv)) {
    return false;
  }

  float current = curDiv * angScale / divisionScale;

  // debug_log << "move angle=" << angle << "cur=" << current << "div=" << division << "curDiv=" << curDiv;

  if (curAngle != 0) {
    *curAngle = current;
  }

  return true;
}

bool SuzhetAnt::stop(float *azimut, float *elevation)
{
  bool ok1 = stopAzimut(azimut);
  if (ok1) {
    *azimut -= zeroAzimut();
  }

  bool ok2 = stopElevat(elevation);
  if (ok2) {
    *elevation -= zeroElevat();
  }

  return ok1 && ok2;
}

bool SuzhetAnt::stopAzimut(float* angle)
{
  bool ok = stop(AZIMUT_ID_MASK, conf().az_angle_scale(), conf().az_division_scale(), angle);
  if (ok) {
    *angle -= zeroAzimut();
  }

  return ok;
}

bool SuzhetAnt::stopElevat(float* angle)
{
  bool ok = stop(ELEVATION_ID_MASK, conf().el_angle_scale(), conf().el_division_scale(), angle);
  if (ok) {
    *angle -= zeroElevat();
  }

  return ok;
}

//! Остановка поворота
/*!
  \param id        ID двигателя
  \param angScale  Максимальный угол на шкале
  \param divisionScale  Количество делений
  \param angle     Текущий угол
*/
bool SuzhetAnt::stop(uint8_t id, float angScale, float divisionScale, float* angle)
{
  if (!sendCommand(id, kCmd_Stop)) {
    return false;
  }

  int16_t curDiv;
  if (!readPortAngle(&curDiv)) {
    return false;
  }

  *angle = curDiv * angScale / divisionScale;
  // debug_log << "stop angle=" << *angle << "curDiv=" << curDiv;

  return true;
}

//! Отправка команды в порт
/*!
  \param id        ID двигатела
  \param command   Команда (см. define вверху)
  \param division  Угол (деление) при необходимости задать новое значение
*/
bool SuzhetAnt::sendCommand(uint8_t id, uint8_t command, int16_t division /*=*/)
{
  /*
  _port->flush();
  QByteArray ba;
  _port->readData(&ba, 100);
  //var(ba.size());
  for (int idx= 0; idx < ba.size(); idx++) {
    debug_log <<  QString("trash 0x%1 (%2)").arg(QString::number(ba[idx], 16).right(2)).arg(db(ba[idx]));
  }
  */

  // var(division);
  uint8_t angle_lo = division & 0x3F;
  uint8_t angle_hi = (division >> 6) & 0x7F;

  QByteArray cmd(3,'\0');
  cmd[0] = 0x80 | id | (angle_lo & 0x3F);
  cmd[1] = angle_hi;
  uint8_t crc = createCrc(cmd[0], cmd[1], command);
  cmd[2] = command | crc;

  // debug_log << "write cmd"
  //           << QString("0x%1 (%2)").arg(QString::number(cmd[0], 16).right(2)).arg(db(cmd[0]))
  //           << QString("0x%1 (%2)").arg(QString::number(cmd[1], 16).right(2)).arg(db(cmd[1]))
  //          << QString("0x%1 (%2)").arg(QString::number(cmd[2], 16).right(2)).arg(db(cmd[2]));

  bool ok = writeBytes(cmd);

  return ok;
}

//! Чтение значения угла из порта (в делениях угла)
bool SuzhetAnt::readPortAngle(int16_t* division)
{
  QByteArray cmd = readCommand();
  if (cmd.size() < 3) return false;

  *division  = (cmd[1] & 0x3F) << 6;
  *division |= cmd[0] & 0x3F;
  if(cmd[1] & 0x40) { // angle < 0
    *division |= 0xF000;
  }

  return true;
}

//! Чтение значения угла из порта (в делениях угла)
QByteArray SuzhetAnt::readCommand()
{
  QByteArray cmd(3, '\0');

  if (!readBytes(&cmd)) {
    return QByteArray();
  }

  // debug_log << "read cmd"
  //           << QString("0x%1 (%2)").arg(QString::number(cmd[0], 16).right(2)).arg(db(cmd[0]))
  //           << QString("0x%1 (%2)").arg(QString::number(cmd[1], 16).right(2)).arg(db(cmd[1]))
  //           << QString("0x%1 (%2)").arg(QString::number(cmd[2], 16).right(2)).arg(db(cmd[2]));

  uint8_t crc = createCrc(cmd[0], cmd[1], cmd[2]);
  if (crc != (cmd[2] & 0xF)) {
    error_log << QObject::tr("Ошибка контрольной суммы crc = %1, ожидаемая = %2").arg((cmd[2] & 0xF)).arg(crc);
    return QByteArray();
  }

  if (kAnsw_FailDriverEvent == (cmd[2] & kAnswMask)) {
    error_log << QObject::tr("Произошло событие драйвера привода. Следует проверить статус драйвера привода");
    //readStatus();
    return cmd;
  }

  if (kAnsw_FailMotorDriver == (cmd[2] & kAnswMask)) {
    error_log << QObject::tr("Неисправен датчик угла");
    return QByteArray();
  }

  if (kAnsw_FailAngle == (cmd[2] & kAnswMask)) {
    error_log << QObject::tr("Задан невыполнимый угол");
    return QByteArray();
  }

  return cmd;
}

//! Чтение значения угла из порта (в делениях угла)
QByteArray SuzhetAnt::readStatus()
{
  QByteArray cmd(3,'\0');

  if (!readBytes(&cmd)) {
    return QByteArray();
  }

  // debug_log << "read stat"
  //           << QString("0x%1 (%2)").arg(QString::number(cmd[0], 16).right(2)).arg(db(cmd[0]))
  //           << QString("0x%1 (%2)").arg(QString::number(cmd[1], 16).right(2)).arg(db(cmd[1]))
  //           << QString("0x%1 (%2)").arg(QString::number(cmd[2], 16).right(2)).arg(db(cmd[2]));

  uint8_t crc = createCrc(cmd[0], cmd[1], cmd[2]);
  if (crc != (cmd[2] & 0xF)) {
    error_log << QObject::tr("Ошибка контрольной суммы crc = %1, ожидаемая = %2").arg((cmd[2] & 0xF)).arg(crc);
    return QByteArray();
  }

  return cmd;
}

// Контрольная сумма, является дополнение до 0 к числу вычисляемому суммирование всех
// ниблов(полубайтов) посылки, с отбрасывание переноса за пределы нибла.
uint8_t SuzhetAnt::createCrc(uint8_t b1, uint8_t b2, uint8_t b3)
{
  uint8_t sum = ((b1 & 0x0F) + (b1 >> 4) + (b2 & 0x0F) + (b2 >> 4) + (b3 >> 4)) & 0x0F;

  return (0x10-sum) & 0xF;
}

}

}
