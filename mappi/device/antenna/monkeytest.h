#pragma once

#include "script.h"


namespace mappi {

namespace antenna {

/**
 * Обезьяний тест (прогонка) - постоянное и бесcмысленное передвижение антенны со скоростью,
 * определяемую параметрами DSA и DSE, параметры определяют сколько градусов в секунду проходит антенна
 * по азимуту и по углу места (допускается изменять в процессе). По достижении максимальных значений углов
 * антенна начинает двигаться в обратном направлении и так до бесконечноcти.
 *
 * Для корректного запуска сценария необходимо определить текущее положение антенны.
 */
class MonkeyTest :
  public Script
{
  Q_OBJECT
public :
  explicit MonkeyTest(QObject* parent = nullptr);
  virtual ~MonkeyTest();

  virtual Script::id_t id() const { return Script::MONKEY_TEST; }
  virtual bool exec(Antenna* antenna);

  virtual bool run(Antenna* antenna);

public slots :
  void setDSA(float v);
  void setDSE(float v);

private :
  void move(float* angle, float offset, bool* isPositive, const Profile::range_t& range);

private :
  float dsa_;
  float azimut_;
  bool isPositiveAzimut_;

  float dse_;
  float elevat_;
  bool isPositiveElevat_;
};

}

}
