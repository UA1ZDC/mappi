#ifndef MAPPI_DEVICE_ANTENNA_SUZHETANT_H
#define MAPPI_DEVICE_ANTENNA_SUZHETANT_H

#include "prbl-ant.h"

/**
   Двигатели имеют ограничение по повороту на заданный угол
    согласно ТЗ:
    - минимальный угол для двигателя азимута -270
    - максимальный угол для двигателя азимута +270

    - минимальный угол для двигателя угла места -5//0
    - максимальный угол для двигателя угла места 103 //+179
**/

// #define MOTOR_AZIMUT_ANGLE_MIN -250
// #define MOTOR_AZIMUT_ANGLE_MAX 250
// #define MOTOR_ELEVAT_ANGLE_MIN 0
// #define MOTOR_ELEVAT_ANGLE_MAX 100 //177

// #define MAX_DSA 9  //!< Максимальная скорость движения по азимуту, градус/сек
// #define MAX_DSE 10 //!< Максимальная скорость движения по углу места, градус/сек


namespace mappi {

namespace receive {

// команды
static const int8_t kCmd_Move = 0x0; //!< двигаться с заданной скоростью
static const int8_t kCmd_Stop = 0x10; //!< стоп
static const int8_t kCmd_SetAngle = 0x20; //!< дойти до заданного угла и остановится
static const int8_t kCmd_GetAngle = 0x40; //!< текущее положение

static const int8_t kCmd_Status = 0x30; //!< статус драйвера привода
static const int8_t kCmd_Regen = 0x50; //!< регенерация положения антены
static const int8_t kCmd_SetTravel = 0x60; //!< привести антену к походному положению (азимут = 0, угол места - минимальный)
static const int8_t kCmd_SetWork = 0x70; //!< привести антену к рабочему положению (азимут 0 градусов, угол места 35)

static const int8_t kAnswMask = 0x70;
// ответ на месте команды (на ответ 0x1 и 0x2 можно запросить более подробный статус)
static const int8_t kAnsw_Ok = 0x0;
static const int8_t kAnsw_FailDriverEvent = 0x10; //!< означает, что произошло событие драйвера привода(сработал один из ограничителей) или состояние калибровки
static const int8_t kAnsw_FailMotorDriver = 0x20; //!< означает, что неисправен драйвера шагового двигателя
static const int8_t kAnsw_FailAngle = 0x40;  //!< задан нывыполнимый угол


class SuzhetAnt :
  public PrblAnt
{
public :
  explicit SuzhetAnt(conf::AntennaType type, QObject* parent = nullptr);
  explicit SuzhetAnt(QObject* parent = nullptr);
  virtual ~SuzhetAnt();

  virtual bool currentAzimut(float* az) override;
  virtual bool setAzimut(float angle, float* curAzimut = nullptr) override;
  virtual bool moveAzimut(float angle, float* curAzimut = nullptr) override;

  virtual bool currentElevat(float* el) override;
  virtual bool setElevat(float angle, float* curEl = nullptr) override;
  virtual bool moveElevat(float angle, float* curEl = nullptr) override;

  virtual bool currentAngles(float* az, float* el) override;
  virtual bool setPosition(float azimut, float elev, bool wait) override;
  virtual bool movePosition(float azimut, float elev, float* curAzimut = nullptr, float* curEl = nullptr) override;
  virtual bool stop(float* azimut, float* elevation) override;
  virtual void stop() override {};

  bool stopAzimut(float* angle);
  bool stopElevat(float* angle);

protected :
  bool currentAngle(uint8_t id, float angScale, float divisionScale, float* angle);
  bool setAngle(uint8_t id, float angScale, float divisionScale, float angle, float *curAngle = nullptr);
  bool move(uint8_t id, float angScale, float divisionScale, float angle, float *curAngle = nullptr);
  bool stop(uint8_t id, float angScale, float divisionScale, float* angle);

  bool readPortAngle(int16_t* angle);
  bool sendCommand(uint8_t id, uint8_t cmd, int16_t angle = 0);

  uint8_t createCrc(uint8_t b1, uint8_t b2, uint8_t b3);

  QByteArray readCommand();
  QByteArray readStatus();
};

}

}

#endif // MAPPI_DEVICE_ANTENNA_SUZHETANT_H
