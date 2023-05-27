#include "direct.h"


namespace mappi {

namespace antenna {

static const float DEVIATION = 0.5;

Direct::Direct(QObject* parent /*=*/) :
  Script(parent),
  alreadyCorrect_(false),
  point_{}
{}

Direct::~Direct()
{
}

void Direct::setPosition(float azimut_deg, float elevat_deg,float azimut_speed_deg,
                         float elevat_speed_deg, bool alreadyCorrect /*=false*/){
    MnSat::AngleWithVelocity az(MnSat::Angle::fromDeg(azimut_deg), MnSat::Angle::fromDeg(azimut_speed_deg));
    MnSat::AngleWithVelocity el(MnSat::Angle::fromDeg(elevat_deg), MnSat::Angle::fromDeg(elevat_speed_deg));
    MnSat::TrackTopoPoint point(az, el);
    setPosition(point, alreadyCorrect);
}
void Direct::setPosition(const MnSat::TrackTopoPoint& point, bool alreadyCorrect /*=false*/){
    alreadyCorrect_ = alreadyCorrect;
    point_ = point;
}


bool Direct::exec(Antenna* antenna)
{
  antenna->setPosition(point_, alreadyCorrect_);
  antenna->snapshot(&azimut, &elevat);

  return !((azimut.precision() < DEVIATION) && (elevat.precision() < DEVIATION));
}

}

}
