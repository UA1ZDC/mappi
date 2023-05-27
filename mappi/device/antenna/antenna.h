#pragma once

#include "feedhorn.h"
#include "protocol.h"
#include "transport.h"
#include <cross-commons/app/waitloop.h>
#include <sat-commons/satellite/tracktopopoint.hpp>


namespace mappi {

namespace antenna {

/**
 * Антенна - объединяет транспортный уровень и протокол управления антенной под общим интерфейсом.
 * Особенности работы в демонстрационном режиме:
 * - транспортный уровень доступен, используются реальные значения текущего положения антенны.
 * - транспортный уровень не доступен, поэтому чтобы как-то смоделировать движение антенны
 *   используется метод (forecastPosition) предсказывающий следующие положение антенны с учетом профиля.
 *
 * Механизм предказания следующего положения антенны работает всегда,
 * только в случае доступности транспорта - предсказанные значения перетираются реальными.
 *
 * !!! Все значения угла в градусах.
 */
class Antenna :
  public QObject
{
  Q_OBJECT
public :
  explicit Antenna(QObject* parent = nullptr);
  virtual ~Antenna();

  bool init(const Configuration& conf);
  const Profile& profile() const { return profile_; }

  // управление транспортным уровнем
  bool open();
  void close();

  // текущее состояние антенны: приводы (азимут, угол места) и облучатель
  void snapshot(Drive* azimut, Drive* elevat, Feedhorn* feedhorn = nullptr);

public slots :
  // коррекция антенной делается всегда, флаг alreadyCorrect говорит о том что, коррекция по азимуту уже была сделана
  void setPosition(MnSat::TrackTopoPoint point, bool alreadyCorrect = false);

  void stop();
  void move(float dsa, float dse);
  void travelMode();

  void feedhorn(bool turnOn);
  void feedhornOutput(Feedhorn::output_t n, bool turnOn);

private slots :
  void onRecv(const QByteArray& buf);

private :
  Protocol* protocol_;
  Transport* transport_;        // QObject

  float azimutCorrect_;
  int txTimeout_;               // msec
  Drive azimut_;
  Drive elevat_;
  Feedhorn feedhorn_;

  Profile profile_;
  meteo::WaitLoop lock_;
};

}

}
