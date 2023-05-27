#include "sessiondata.h"
#include <commons/mathtools/mnmath.h>
#include <QtDebug>

#define DT_FORMAT Qt::ISODate
#define FIELDS_COUNT 7


namespace mappi {

namespace schedule {

static const QString FILE_DT_FORMAT = "yyyyMMddThhmmss";
static const QString FILE_EXTENSION = "raw";

static const int FW_SATELLITE = -12;
static const int FW_REWOL = -6;
static const int FW_DT = -20;
static const int FW_DIRECT = 3;
static const int FW_ELEVAT_MAX = -7;

QString SessionData::dateToString(const QDateTime& dt)
{
  return dt.toString(DT_FORMAT);
}

QDateTime SessionData::dateFromString(const QString& str)
{
  return QDateTime::fromString(str, DT_FORMAT);
}

QString SessionData::info(const QString& satellite, unsigned long revol)
{
  SessionData data;
  data.satellite = satellite;
  data.revol = revol;

  return data.info();
}


SessionData::SessionData()
{
  setDefault();
}

SessionData::~SessionData()
{
}

bool SessionData::isDefault() const
{
  return ((satellite.isEmpty()) && (revol == 0));
}

void SessionData::setDefault()
{
  satellite = "";
  aos = QDateTime();
  los = QDateTime();
  elevatMax = 89.99;
  revol = 0;
  direction = conf::kUnkDirection;
  conflState = conf::kUnkState;
}

QString SessionData::info() const
{
  return QString("%1, %2")
    .arg(satellite)
    .arg(revol);
}

QString SessionData::toString(bool isPretty /*=*/) const
{
  return QString("%1\t%2\t%3\t%4\t%5\t%6\t%7")
    .arg(satellite, FW_SATELLITE)
    .arg(revol, FW_REWOL)
    .arg(aos.toString(DT_FORMAT), FW_DT)
    .arg(los.toString(DT_FORMAT), FW_DT)
    .arg((isPretty ? directionName() : QString::number(direction)))
    .arg(MnMath::rad2deg(elevatMax), FW_ELEVAT_MAX, 'g', 4)
    .arg((isPretty ? conflStateName() : QString::number(conflState)));
}

bool SessionData::fromString(const QString& str)
{
  QStringList field = str.split('\t', QString::SkipEmptyParts);
  if (field.count() == FIELDS_COUNT) {
    satellite = field[0].trimmed();
    revol = field[1].trimmed().toUInt();
    aos = QDateTime::fromString(field[2], DT_FORMAT);
    los = QDateTime::fromString(field[3], DT_FORMAT);
    direction = static_cast<conf::SatDirection>(field[4].toInt());
    elevatMax = MnMath::deg2rad(field[5].trimmed().toDouble());
    conflState = static_cast<conf::ConflState>(field[6].toInt());

    return true;
  }

  return false;
}

QString SessionData::fileName() const
{
  QString tmp = satellite;
  return QString("%1_%2_%3_%4_%5.%6")
    .arg(aos.toString(FILE_DT_FORMAT))
    .arg(los.toString(FILE_DT_FORMAT))
    .arg(tmp.remove(' '))
    .arg(revol)
    .arg(directionName())
    .arg(FILE_EXTENSION);
}

QString SessionData::directionName() const
{
  switch (direction) {
    case conf::kDescending : return "DSС";
    case conf::kAscending :  return "ASC";
    default : break ;
  }

  return "UNK";
}

QString SessionData::conflStateName() const
{
  switch (conflState) {
    case conf::kNormalState :    return "NORMAL";
    case conf::kSelfState :      return "SELF";
    case conf::kOtherState :     return "OTHER";
    case conf::kUserSelfState :  return "USER_SELF";
    case conf::kUserOtherState : return "USER_OTHER";
    default : break ;
  }

  return "UNKNOWN";
}

bool SessionData::operator ==(const SessionData& other) const
{
  // return ((satellite == other.satellite)
  //   && (aos.toTime_t() == other.aos.toTime_t())
  //   && (los.toTime_t() == other.los.toTime_t())
  //   && (::fabsf(elevatMax - other.elevatMax) < 0.001)
  //   && (revol == other.revol)
  //   && (direction == other.direction)
  //   && (conflState == other.conflState)
  // );
  return ((satellite == other.satellite) && (revol == other.revol));
}

bool SessionData::operator <(const SessionData& other) const
{
  return (0 < aos.secsTo(other.aos));
}

bool SessionData::operator !=(const SessionData& other) const
{
  return !(*this == other);
}

}

}
