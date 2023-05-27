#pragma once

#include <sat-commons/satellite/satellite.h>
#include <commons/mathtools/mnmath.h>
#include <sat-commons/satellite/tracktopopoint.hpp>
#include <mappi/device/antenna/profile.h>
#include <qvector.h>


namespace mappi {

namespace antenna {

class TrackUtils final {
public:
  static QString info(const QList<MnSat::TrackTopoPoint>& track);
  static void gap_removal(QList<MnSat::TrackTopoPoint>& track);
  static void correction(QList<MnSat::TrackTopoPoint>& track, float offset);
  static bool adaptation(QList<MnSat::TrackTopoPoint>& track, Profile::range_t azimuth_limits);

private:
  TrackUtils() = delete;
  ~TrackUtils() = delete;
};

}

}
