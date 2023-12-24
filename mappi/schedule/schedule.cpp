#include "schedule.h"
#include <commons/geobasis/coords.h>
#include <qtextstream.h>
#include <qfile.h>


namespace mappi {

namespace schedule {

static const int SETTL_CONFL_STEP = 10; // Шаг для расчета траектории для разрешения конфликта (точность)

Session Schedule::default_;

Schedule::Schedule()
{
}

Schedule::~Schedule()
{
}

bool Schedule::make(const Configuration& conf, const QDateTime& dt /*=*/)
{
  QDateTime dtStart = dt.addSecs(-60 * 20);             // -20 мин
  unsigned int period = (conf.period * 60 * 60);        // часы -> сек.
  float elevatMin = MnMath::deg2rad(conf.elevation.min());
  float elevatMax = MnMath::deg2rad(conf.elevation.max());

  data_.clear();

  Satellite satellite;
  foreach(const QString& satName, conf.satellite) {
    if (!satellite.readTLE(satName, conf.tlePath))
      continue ;

    time_t ts = satellite.timeFromTLE(dtStart) * 60; // в секундах
    time_t nextTs = ts;

    time_t aos = 0;
    time_t los = 0;

    while (nextTs < (ts + period)) {
      if (!satellite.search_AOS_LOS(nextTs, conf.coord, &aos, &los, elevatMin))
        break ;

      // спутник уже в зоне
      if (los < aos)
        aos = nextTs;

      SessionData sessionData;
      time_t mos;
      if (!satellite.searchMaximum(aos, conf.coord, &sessionData.elevatMax, &mos))
        break ;

      if (elevatMax <= sessionData.elevatMax) {
        sessionData.satellite = satName;
        sessionData.aos = dtStart.addSecs(aos - ts);
        sessionData.los = dtStart.addSecs(los - ts);
        sessionData.direction = static_cast<conf::SatDirection>(satellite.direction(sessionData.aos, sessionData.los));
        sessionData.revol = satellite.revolNumber(dtStart.addSecs(mos - ts));

        data_.append(sessionData);
      }

      nextTs = los + 300;
    }
  }

  qSort(data_);

  return true;
}

void Schedule::resolvConfl(const Configuration& conf, bool saveConflResolByUser /*=*/)
{
  for (auto lhs = data_.begin(); lhs != data_.end(); ++lhs) {
    if (lhs->data().conflState == conf::kOtherState)
      continue;

    if (saveConflResolByUser)
      if (lhs->data().conflState == conf::kUserSelfState
          || lhs->data().conflState == conf::kUserOtherState) {
        continue;
      }

    bool isOk = false;
    for (auto rhs = lhs + 1; rhs != data_.end(); ++rhs) {
      if (rhs->data().conflState == conf::kOtherState)
        continue;

      if (lhs->data().los > rhs->data().los) {
        isOk = true;

        if (0 <= cmpConflict(*lhs, *rhs, conf)) {
          lhs->data().conflState = conf::kSelfState;
          rhs->data().conflState = conf::kOtherState;
        } else {
          lhs->data().conflState = conf::kOtherState;
          rhs->data().conflState = conf::kSelfState;
        }

        break;
      }
    }

    if (!isOk && lhs->data().conflState == conf::kUnkState)
      lhs->data().conflState = conf::kNormalState;
  }
}

void Schedule::addGeo(const Configuration& conf){
  Satellite satellite;
  QDateTime currentDateTime = QDateTime::currentDateTime();
  for(const QString& satName : conf.geoTimes.keys()) {
    if (!satellite.readTLE(satName, conf.tlePath)) continue;
    for(const QString recTime : conf.geoTimes[satName]){

      QDateTime aosDateTime = currentDateTime;
      aosDateTime.setTime(QTime::fromString(recTime, "HH:mm"));
      QDateTime losDateTime = aosDateTime.addSecs(conf.geoDuration[satName]);

      SessionData sessionData;
      sessionData.satellite = satName;
      sessionData.aos = aosDateTime;
      sessionData.los = losDateTime;
      sessionData.direction = static_cast<conf::SatDirection>(satellite.direction(sessionData.aos, sessionData.los));
      sessionData.revol = satellite.revolNumber(sessionData.aos);
      sessionData.elevatMax = 0.7; //changeme
      data_.append(sessionData);
    }
  }

  qSort(data_);
}

void Schedule::clear()
{
    data_.clear();
}

bool Schedule::isEmpty() const
{
  return data_.isEmpty();
}

int Schedule::size() const
{
  return data_.size();
}

int Schedule::getIdx(const Session& session) const
{
  for (int idx = 0; idx < data_.size(); ++idx) {
    if (data_[idx] == session)
     return idx;
  }

  return -1;
}

Session& Schedule::getSession(int idx)
{
  return (data_.size() <= idx ? default_ : data_[idx]);
}

Session& Schedule::getSession(const QString& satellite, const unsigned revol)
{
  for (int idx = 0; idx < data_.size(); ++idx) {
    Session& session = data_[idx];
    if ((session.data().revol == revol) && (session.data().satellite == satellite))
      return data_[idx];
  }

  return default_;
}

Session& Schedule::first()
{
  return (data_.isEmpty() ? default_ : data_.first());
}

Session& Schedule::last()
{
  return (data_.isEmpty() ? default_ : data_.last());
}

Session& Schedule::near(bool useProceed /*=*/)
{
  return near(QDateTime::currentDateTimeUtc(), useProceed);
}

Session& Schedule::near(const QDateTime& dt, bool useProceed /*=*/)
{
  for (int idx = 0; idx < data_.size(); ++idx) {
    Session& session = data_[idx];
    if (session.isIgnored())
      continue ;

    switch (session.stage(dt)) {
      case Session::WAITING :
        return session;

      case Session::PROCEED :
        if (useProceed)
          return session;

      default :
        break ;
    }
  }

  return default_;
}

Session& Schedule::next()
{
  for (int idx = getIdx(near()) + 1; idx < data_.size(); ++idx) {
    Session& session = data_[idx];
    if (session.isIgnored())
      continue ;

    if (session.stage() == Session::WAITING)
      return session;
  }

  return default_;
}

int Schedule::percentCompleted() const
{
  int total = 0;
  for (int idx = 0; idx < data_.size(); ++idx) {
    const Session& session = data_[idx];
    if (session.stage() == Session::COMPLETED)
      ++total;
  }

  return (total * 100. / data_.size());
}

QString Schedule::toString(bool isPretty /*=*/) const
{
  QStringList list;
  for (auto session : data_)
    list.append(session.data().toString(isPretty));

  return list.join('\n');
}

QString Schedule::toNumberString() const
{
  QStringList list;

  int idx = 0;
  for (auto session : data_)
    list.append(QString("%1 %2")
      .arg(idx++, -3)
      .arg(session.data().toString(true))
    );

  return list.join('\n');
}

bool Schedule::save(const QString& filePath) const
{
  QFile file(filePath);
  if (file.open(QFile::WriteOnly) == false)
    return false;

  QTextStream out(&file);
  out << toString(false) << '\n';
  file.close();

  return true;
}

bool Schedule::load(const QString& filePath)
{
  QFile file(filePath);
  if (file.open(QIODevice::ReadOnly) == false)
    return false;

  data_.clear();

  QTextStream in(&file);
  while (!in.atEnd()) {
    SessionData data;
    if (data.fromString(in.readLine()))
      data_.append(Session(data));
  }

  file.close();

  return true;
}

// число меньше 0 - приоритет спутника lhs меньше rhs
// равно 0        - приоритеты равны
// число больше 0 - приоритет спутника lhs больше rhs
int Schedule::cmpConflict(const Session& lhs, const Session& rhs, const Configuration& conf) const
{
  switch (conf.conflResol) {
    case conf::kStartResol :
      return lhs.data().aos.secsTo(rhs.data().aos);

    case conf::kDurationResol :
      return (lhs.duration() - rhs.duration());

    case conf::kWestResol :
      return (timeWest(lhs, SETTL_CONFL_STEP, conf) - timeWest(rhs, SETTL_CONFL_STEP, conf));

    case conf::kEastResol :
      return (timeWest(rhs, SETTL_CONFL_STEP, conf) - timeWest(lhs, SETTL_CONFL_STEP, conf));

    default :
      break ;
  }

  return 0;
}

int Schedule::timeWest(const Session& session, int step, const Configuration& conf) const
{
  Satellite satellite;
  if (!satellite.readTLE(session.data().satellite, conf.tlePath))
    return 0;

  QVector<Coords::GeoCoord> track;
  satellite.trajectory(session.data().aos, session.data().los, step, track);

  int nPoint = 0; //количество точек траектории западнее места наблюдения
  for (auto it = track.begin(); it != track.end(); ++it) {
    if ((::fabs(conf.coord.lon - (*it).lon) <= M_PI ? (*it).lon : (*it).lon - MnMath::M_2PI) < conf.coord.lon)
      ++nPoint;
  }

  return (nPoint * step);
}

int Schedule::direction(const Satellite& satellite, double aos) const
{
  Coords::EciPoint ecip;
  if (!satellite.getPosition(aos / 60, &ecip))
    return -1;

  if (0 < ecip.vel.z)
    return 1;
  else if (ecip.vel.z < 0)
    return 0;

  return -1;
}

}

}
