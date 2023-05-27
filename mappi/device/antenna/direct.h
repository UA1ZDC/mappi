#pragma once

#include "script.h"
#include <sat-commons/satellite/tracktopopoint.hpp>


namespace mappi {

namespace antenna {

/**
 * Сценарий позиционирования (наведения) антенны.
 * При достижении антенной заданного положения (с учетом погрешности) - сценарий прекращает работу.
 */
class Direct :
  public Script
{
  Q_OBJECT
public :
  explicit Direct(QObject* parent = nullptr);
  virtual ~Direct();

  virtual Script::id_t id() const { return Script::DIRECT; }
  virtual bool exec(Antenna* antenna);

public slots :
  void setPosition(float azimut_deg, float elevat_deg, float azimut_speed_deg,float elevat_speed_deg, bool alreadyCorrect = false);
  void setPosition(const MnSat::TrackTopoPoint& point, bool alreadyCorrect = false);

  //[[deprecated("Use TrackTopoPoint or 4-argument version instead.")]]
  Q_DECL_DEPRECATED void setPosition(float azimut_deg, float elevat_deg, bool alreadyCorrect = false) {
      setPosition(azimut_deg, elevat_deg, 0, 0, alreadyCorrect);
  }

private :
  bool alreadyCorrect_;
  MnSat::TrackTopoPoint point_;
};

}

}
