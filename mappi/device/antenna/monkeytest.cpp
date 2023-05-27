#include "monkeytest.h"


namespace mappi {

namespace antenna {

static const float DEFAULT_VELOCITY = 0;

MonkeyTest::MonkeyTest(QObject* parent /*=*/) :
    Script(parent),
  dsa_(0),
  azimut_(0),
  isPositiveAzimut_(false),
  dse_(0),
  elevat_(0),
  isPositiveElevat_(false)
{
}

MonkeyTest::~MonkeyTest()
{
}

bool MonkeyTest::run(Antenna* antenna)
{
  if (Script::run(antenna)) {
    azimut_ = azimut.self;
    isPositiveAzimut_ = (0 < azimut.self);

    elevat_ = elevat.self;
    isPositiveElevat_ = (90 < elevat.self);
    return true;
  }

  return false;
}

void MonkeyTest::setDSA(float v)
{
  dsa_ = v;
}

void MonkeyTest::setDSE(float v)
{
  dse_ = v;
}

bool MonkeyTest::exec(Antenna* antenna)
{
  const Profile& profile = antenna->profile();
  if (profile.isValidDSA(dsa_))
    move(&azimut_, profile.dsaTempo(dsa_), &isPositiveAzimut_, profile.azimut);

  if (profile.isValidDSE(dse_))
    move(&elevat_, profile.dseTempo(dse_), &isPositiveElevat_, profile.elevat);

  float azimut_velocity = (isPositiveAzimut_ ? 1 : -1) * dsa_; //dsa и dse всегда положительные
  float elevat_velocity = (isPositiveElevat_ ? 1 : -1) * dse_;
  MnSat::AngleWithVelocity az(MnSat::Angle::fromDeg(azimut_), MnSat::Angle::fromDeg(azimut_velocity));
  MnSat::AngleWithVelocity el(MnSat::Angle::fromDeg(elevat_), MnSat::Angle::fromDeg(elevat_velocity));
  MnSat::TrackTopoPoint point(az, el);

  antenna->setPosition(point, true);

  return true;
}

void MonkeyTest::move(float* angle, float offset, bool* isPositive, const Profile::range_t& range)
{
  if (offset == 0)
    return ;

  // движение к максимальному значению
  if (*isPositive) {
    if ((*angle + offset) <= range.max)
      *angle += offset;
    else {
      *isPositive = false;      // смена направления
      *angle -= offset;
    };
  // движение к минимальному значению
  } else {
    if (range.min <= (*angle - offset))
      *angle -= offset;
    else {
      *isPositive = true;       // смена направления
      *angle += offset;
    }
  }
}

}

}
