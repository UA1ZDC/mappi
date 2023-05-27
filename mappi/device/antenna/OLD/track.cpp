#include "track.h"
#include <QtDebug>


namespace mappi {

namespace antenna {

static constexpr float PI = 3.1415926535;
static constexpr float QUARTER_II_FIRST = (90 * PI) / 180;
static constexpr float QUARTER_III_FIRST = (180 * PI) / 180;
static constexpr float QUARTER_IV_FIRST = (270 * PI) / 180;
static constexpr float QUARTER_IV_LAST = (359.99 * PI) / 180;
static constexpr float AZIMUT_MAX = (2 * PI);

// class Quarter
Quarter Quarter::make(float angle)
{
  if ((0 <= angle) && (angle < QUARTER_II_FIRST))
    return Quarter(QUARTER_I, { 0, QUARTER_II_FIRST });
  else if ((QUARTER_II_FIRST <= angle) && (angle < QUARTER_III_FIRST))
    return Quarter(QUARTER_II, { QUARTER_II_FIRST, QUARTER_III_FIRST });
  else if ((QUARTER_III_FIRST <= angle) && (angle < QUARTER_IV_FIRST))
    return Quarter(QUARTER_III, { QUARTER_III_FIRST, QUARTER_IV_FIRST });

  return Quarter(QUARTER_IV, { QUARTER_IV_FIRST, QUARTER_IV_LAST });
}

Quarter::order_t Quarter::ident(const QList<MnSat::TrackTopoPoint>& track)
{
  Quarter::order_t res;

  int count = track.count();
  int i = 0;
  while (i < count) {
    const MnSat::TrackTopoPoint& point = track_[i];
    qDebug() << QString("%1 %2 [%3, %4]")
      .arg(i, 4)
      .arg(point.time.toString("yyyy.MM.dd hh:mm:ss.zzz"), 23)
      .arg(MnMath::rad2deg(point.az), 7, 'f', 2)
      .arg(MnMath::rad2deg(point.el), 7, 'f', 2);

    if (quarter == nullptr) {
      order_.append(Quarter::make(point.az));
      quarter = &order_.last();

      quarter->first = i;
      quarter->last = i;
    } else {
      if (!quarter->inRange(point.az)) {
        quarter = nullptr;

        continue ;
      }

      quarter->last = i;
    }

    ++i;
  }

}

Quarter::Quarter(const Quarter::id_t& id, const Quarter::range_t& range) :
  first(-1),
  last(-1),
  id_(id),
  range_(range)
{
}

Quarter::~Quarter()
{
}

bool Quarter::inRange(float angle) const
{
  return ((range_.first <= angle) && (angle < range_.last));
}

void Quarter::invert(QList<MnSat::TrackTopoPoint>& track) const
{
  switch (id_) {
    // case Quarter::QUARTER_I : четверть всегда положительна
    case Quarter::QUARTER_II :
    case Quarter::QUARTER_III :
    case Quarter::QUARTER_IV :
      for (int i = first; i <= last; ++i)
        track[i].az = -AZIMUT_MAX;

      break ;

    default :
      break ;
  }
}


// class Track
Track::Track(QList<MnSat::TrackTopoPoint>& track) :
  track_(track)
{
  // float maxElevat = 0;
  Quarter* quarter = nullptr;

  int count = track.count();
  int i = 0;
  while (i < count) {
    const MnSat::TrackTopoPoint& point = track_[i];
    // qDebug() << point.time << i << MnMath::rad2deg(point.az) << ";" << MnMath::rad2deg(point.el);
    if (quarter == nullptr) {
      order_.append(Quarter::make(point.az));
      quarter = &order_.last();

      quarter->first = i;
      quarter->last = i;
      // quarter->maxElevat = i;
      // maxElevat = point.el;
    } else {
      if (!quarter->inRange(point.az)) {
        quarter = nullptr;

        continue ;
      }

      quarter->last = i;
      // if (maxElevat < point.el) {
      //   maxElevat = point.el;
      //   quarter->maxElevat = i;
      // }
    }

    ++i;
  }
}

Track::~Track()
{
}

QString Track::path() const
{
  QString res;
  for (auto item : order_)
    res.append(QString::number(item.id()));

  return res;
}

bool Track::isCrossQuarter(Quarter::id_t v) const
{
  for (auto item : order_) {
    if (v == item.id())
      return true;
  }

  return false;
}

QString Track::toString() const
{
  QStringList res;
  res.append("");

  for (auto item : order_)
    res.append(QString("quarter (id: %1, azimut: %2 %3, eleval: %4)")
      .arg(item.id())
      .arg(MnMath::rad2deg(track_[item.first].az), 7, 'f', 2)
      .arg(MnMath::rad2deg(track_[item.last].az), 7, 'f', 2)
      .arg(MnMath::rad2deg(track_[item.maxElevat].el), 6, 'f', 2)
    );

  return res.join('\n');
}

bool Track::adaptation(float azimutCorrect)
{
/*
qDebug() << azimutCorrect;
  // коррекция по азимиту
  for (auto& item : track_) {
    item.az += azimutCorrect;
    //if ((item.az + azimutCorrect) < 0)
    //  item.az = AZIMUT_MAX - item.az;

    if (QUARTER_IV_LAST < item.az)
      item.az -= AZIMUT_MAX;
  }
*/
  switch (path().toInt()) {
    // без прересчета
    case 1 :      // +
    case 12 :     // ++
    case 123 :    // +++
    case 1234 :   // +++- с потерей
    case 2 :      // +
    case 21 :     // ++
    case 23 :     // ++
    case 3 :      // +
    case 32 :     // ++
    case 321 :    // +++
      qDebug() << "without adaptation";
      break ;

    // отрицательный переход, после перехода в IV четверть все инвертируется
    case 14 :     // +-
    case 143 :    // +--
    case 1432 :   // +---
    case 214 :    // ++-
    case 2143 :   // ++--
    case 3214 :   // +++-
      {
        qDebug() << "negative transition, adaptation";
        bool isCrossing = false;
        for (auto quarter : order_) {
          if (quarter.id() == Quarter::QUARTER_IV)
            isCrossing = true;

          if (isCrossing)
            quarter.invert(track_);
        }
      }
      break ;

    // полный пересчет, инверсия всех четвертей
    case 234 :    // ---
    case 34 :     // --
    case 4 :      // -
    case 43 :     // --
    case 432 :    // ---
      qDebug() << "full adaptation";
      for (auto quarter : order_)
        quarter.invert(track_);
      break ;

    // положительный переход, до перехода в I четверть все инвертируется
    case 2341 :   // ---+
    case 341 :    // --+
    case 3412 :   // --++
    case 41 :     // -+
    case 412 :    // -++
    case 4123 :   // -+++
    case 4321 :   // ---+
      qDebug() << "positive transition, adaptation";
      for (auto quarter : order_) {
        if (quarter.id() == Quarter::QUARTER_I)
          break ;

        quarter.invert(track_);
      }
      break ;

    // недопустимый переход
    default :
      return false;
  }

  return true;
}

}

}
