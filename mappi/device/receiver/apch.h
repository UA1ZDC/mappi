#pragma once

#include <commons/geobasis/coords.h>
#include <sat-commons/satellite/satellite.h>
#include <qobject.h>


namespace mappi {

namespace receiver {

/**
 * Блок автоматической подстройки частоты.
 * Вынесен в отдельный класс из-за наличия разных реализаций приёмников.
 */
class ApchUnit :
  public QObject
{
  Q_OBJECT
public :
  explicit ApchUnit(const Coords::GeoCoord& site, QObject* parent = nullptr);
  virtual ~ApchUnit();

  void turnOn(Satellite* satellite, float freq);
  void turnOff();

signals :
  void update(float freq);

protected :
  virtual void timerEvent(QTimerEvent* event);

private :
  Coords::GeoCoord site_;
  int timerId_;

  Satellite* satellite_;
  float freq_;
};

}

}
