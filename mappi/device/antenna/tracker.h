#pragma once

#include "script.h"
#include "track.h"


namespace mappi {

namespace antenna {

/**
 * Сценарий сопровождения спутника по заранее рассчитанной траектории.
 * Каждая точка траектории определяет:
 * - временную метку соответствующую точке;
 * - азимут;
 * - угол места.
 *
 * По завершении прохождения траектории сценарий завершает работу.
 * Предполагается, что траектория будет уже скорректирована и адаптирована.
 * Также при установке траектории можно задать временную метку смещения относительно начала траектории,
 * для случая когда начало сеанса пропущено.
 *
 * Точки траектории рассчитываются в радианах, а перед отправкой в антенну пересчитываются в градусы.
 */
class Tracker :
  public Script
{
  Q_OBJECT
public :
  explicit Tracker(QObject* parent = nullptr);
  virtual ~Tracker();

  virtual Script::id_t id() const { return Script::TRACKER; }
  virtual bool exec(Antenna* antenna);

  virtual bool run(Antenna* antenna);

public slots :
  // timeStamp - определяет смещение относитель начала траектории
  void setTrack(const QList<MnSat::TrackTopoPoint>& track, const QDateTime& timeStamp);
  void setTrack(const QList<MnSat::TrackTopoPoint>& track);

private :
  int idx_;
  int last_;
  QList<MnSat::TrackTopoPoint> track_;
};

}

}
