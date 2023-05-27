#pragma once

#include <sat-commons/satellite/satellite.h>
#include <commons/mathtools/mnmath.h>
#include <qvector.h>


namespace mappi {

namespace antenna {

/**
 * Четверть - определяет границы подинтервала (индексы начала и конца) траектории антенны.
 * Четверть позволяет инвертировать все значения в подинтервале. Под инверсией понимается
 * изменение знака азимута точки в отрицателную сторону, вычитанием из неё 2Пи (кроме I четверти, т.к. она всегда положительна).
 * Зная порядок прохождения четвертей, траекторию возможно адаптировать под ограничения антенны.
 * Все значения угла в радианах.
 *
 * Деление азимута по четвертям:
 *           0
 *           |
 *       IV  |  I
 * 270 ______|______ 90
 *           |
 *      III  |  II
 *           |
 *          180
 */
class Quarter
{
public :
  typedef QVector<Quarter> sequence_t;

  // числовой диапазон границ четверти
  struct range_t {
    float first;
    float last;
  };

  enum id_t {
    QUARTER_I = 1,
    QUARTER_II = 2,
    QUARTER_III = 3,
    QUARTER_IV = 4
  };

public :
  static Quarter make(float angle);
  static sequence_t ident(const QList<MnSat::TrackTopoPoint>& track);
  static QString info(const QList<MnSat::TrackTopoPoint>& track);
  static void correction(QList<MnSat::TrackTopoPoint>& track, float offset);
  static bool adaptation(QList<MnSat::TrackTopoPoint>& track);

public :
  Quarter(const Quarter::id_t& id, const Quarter::range_t& range);
  ~Quarter();

  id_t id() const { return id_; }

  bool inRange(float angle) const;
  void invert(QList<MnSat::TrackTopoPoint>& track) const;

public :
  int first;
  int last;

private :
  id_t id_;
  range_t range_;
};


/**
 * Траектория.
 * Предоставляет полную информацию о траектории с разбивкой по четвертям.
 * Позволяет провести коррекцию по азимуту и получить адаптирированную траекторию под антенну.
 * Все значения угла в радианах.
 */
//class Track
//{
//public :
//  Track();
//  ~Track();

//  QString info(const QList<MnSat::TrackTopoPoint>& track);
//  void correction(QList<MnSat::TrackTopoPoint>& track, float offset);
//  bool adaptation(QList<MnSat::TrackTopoPoint>& track); // сам еще раз определит последовательность

  // QString path() const;
  // bool isCrossQuarter(Quarter::id_t v) const;
  // bool adaptation(float azimutCorrect);
  // QString toString() const;

//private :
//  QList<MnSat::TrackTopoPoint> track_;
//  Quarter::order_t order_;
//};

}

}
