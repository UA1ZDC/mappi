#include "profile.h"
#include <qstringlist.h>


namespace mappi {

namespace antenna {

static void rangeToProto(const Profile::range_t& other, conf::AngleRange* range)
{
  range->set_min(other.min);
  range->set_max(other.max);
}

static void rangeFromProto(const conf::AngleRange& other, Profile::range_t* range)
{
  range->min = other.min();
  range->max = other.max();
}


void Profile::toProto(const Profile& other, conf::AntennaProfile* profile)
{
  profile->set_type(other.type);

  rangeToProto(other.azimut, profile->mutable_azimut());
  rangeToProto(other.elevat, profile->mutable_elevat());

  conf::SpinParam* spin = profile->mutable_spin();
  spin->set_dsa(other.dsa);
  spin->set_dse(other.dse);

  profile->set_time_slot(other.timeSlot);
  profile->set_travel_mode(other.hasTravelMode);
  profile->set_feedhorn_control(other.hasFeedhornControl);
}

void Profile::fromProto(const conf::AntennaProfile& other, Profile* profile)
{
  profile->type = other.type();

  rangeFromProto(other.azimut(), &profile->azimut);
  rangeFromProto(other.elevat(), &profile->elevat);

  conf::SpinParam spin = other.spin();
  profile->dsa = spin.dsa();
  profile->dse = spin.dse();

  profile->timeSlot = other.time_slot();
  profile->hasTravelMode = other.travel_mode();
  profile->hasFeedhornControl = other.feedhorn_control();
}

Profile::Profile()
{
}

Profile::~Profile()
{
}

bool Profile::isValidAzimut(float v) const
{
  return ((azimut.min <= v) && (v <= azimut.max));
}

bool Profile::isValidElevat(float v) const
{
  return ((elevat.min <= v) && (v <= elevat.max));
}

bool Profile::isValidRange(float azimut, float elevat) const
{
  return (isValidAzimut(azimut) && isValidElevat(elevat));
}

bool Profile::isValidDSA(float v) const
{
  return ((0 <= v) && (v <= dsa));
}

bool Profile::isValidDSE(float v) const
{
  return ((0 <= v) && (v <= dse));
}

float Profile::dsaTempo(float v) const
{
  return (v * timeSlot);
}

float Profile::dsaTempo() const
{
  return dsaTempo(dsa);
}

float Profile::dseTempo(float v) const
{
  return (v * timeSlot);
}

float Profile::dseTempo() const
{
  return dseTempo(dse);
}

QString Profile::toString() const
{
  QStringList res = {
    "profile:",
    QString("  type:   %1").arg(type == conf::kVkaAntenna ? "vka" : "niitv"),
    QString("  azimut: [%1; %2]").arg(azimut.min).arg(azimut.max),
    QString("  elevat: [%1; %2]").arg(elevat.min).arg(elevat.max),
    QString("  dsa:    %1").arg(dsa),
    QString("  dse:    %1").arg(dse),
    QString("  time slot:   %1").arg(timeSlot),
    QString("  travel mode: %1").arg((hasTravelMode ? "true" : "false")),
    QString("  feedhorn control: %1").arg((hasFeedhornControl ? "true" : "false"))
  };

  return res.join('\n');
}

}

}
