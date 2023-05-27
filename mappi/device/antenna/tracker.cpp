#include "tracker.h"
#include "print.h"


namespace mappi {

namespace antenna {

static const float DEFAULT_VELOCITY = 0;

Tracker::Tracker(QObject* parent /*=*/) :
    Script(parent),
  idx_(0),
  last_(0)
{
}

Tracker::~Tracker()
{
}

bool Tracker::run(Antenna* antenna)
{
  if (track_.isEmpty())
    return false;

  return Script::run(antenna);
}

void Tracker::setTrack(const QList<MnSat::TrackTopoPoint>& track, const QDateTime& timeStamp)
{
  track_ = track;
  last_ = track.count() - 1;

  idx_ = 0;
  for (auto point : track_) {
    if (timeStamp < point.time())
      break;

    ++idx_;
  }

  // qDebug() << QObject::tr("first point: %1/%2 %3")
  //   .arg(idx_, FW_INDEX)
  //   .arg(last_, -FW_INDEX)
  //   .arg(toString(track_[idx_].time));
}

void Tracker::setTrack(const QList<MnSat::TrackTopoPoint>& track)
{
  track_ = track;
  last_ = track.count() - 1;
  idx_ = 0;
}

bool Tracker::exec(Antenna* antenna)
{
  if (last_ < idx_)
    return false;

  const MnSat::TrackTopoPoint& point = track_[idx_];
  float azimut = MnMath::rad2deg(point.az());
  float elevat = MnMath::rad2deg(point.el());

  debug_log << QObject::tr("%1/%2 %3")
    .arg(idx_, FW_INDEX)
    .arg(last_, -FW_INDEX)
    .arg(toString(point.time(), azimut, elevat));

  //debug_log << QString("Sending to antenna: %1").arg(point.toString());
  antenna->setPosition(point, true);
  ++idx_;

  return true;
}

}

}
