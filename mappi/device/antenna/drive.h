#pragma once

#include <math.h>
#include <qobject.h>


namespace mappi {

namespace antenna {

/** Привод антенны. */
class Drive
{
  Q_GADGET;
public :
  // идентификатор привода
  enum id_t {
    AZIMUT = 0,
    ELEVAT
  };
  Q_ENUM(id_t)

  // состояние привода
  enum state_t {
    OK = 0,             // OK
    UNKNOWN,            // нет доступа к приводу, транспортный уровень отключен
    FAIL,               // ошибка привода
    SENSOR_ANGLE_FAIL,  // ошибка энкодера или неисправен датчик угла
    CRC_NOT_VALID       // не совпадает crc

    // TODO возможно нужно добавить походное положение
    // ON_MOVE;            // в движении
    // STOPPED;            // остановлен
  };
  Q_ENUM(state_t)

public :
  static Drive makeAzimut();
  static Drive makeElevat();

public :
  Drive();
  ~Drive();

  float precision() const;
  QString toString() const;

public :
  id_t id;
  state_t state;        // текущее состояние

  float self;           // текущее значение угла, градусы
  float dst;            // целевое значение угла, градусы

  unsigned int seq;     // количество ответов от антенны, за время существования объекта

};

}

}
