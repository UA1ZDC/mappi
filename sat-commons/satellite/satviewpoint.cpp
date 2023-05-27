#include "satviewpoint.h"

#include <commons/mathtools/mnmath.h>
#include <cross-commons/debug/tlog.h>
//#include <cross-commons/funcs/mn_funcs.h>

using namespace MnMath;
using namespace Coords;

SatViewPoint::SatViewPoint() :
  _isOk(false)
{
}

SatViewPoint::SatViewPoint(const QDateTime& dtStart, const QDateTime& dtEnd)
{
  setDateTime(dtStart, dtEnd);
}

SatViewPoint::~SatViewPoint()
{
}

void SatViewPoint::setDateTime(const QDateTime& dtStart, const QDateTime& dtEnd)
{
  _dtStart = dtStart;
  _dtEnd = dtEnd;
  if (_dtStart.isValid() && _dtEnd.isValid()) {
    _isOk = true;
  }
}

/*!
 * \brief Расчет упрощенной рамки области сканирования. При маленьком шаге может работать медленно. (Только боковые линии, первая и последняя строки будут ровной линией)
 * \param vStep Шаг по вертикали, милисекунды. 
 * \param maxAngle Максимальный угол сканирования от надира до центра крайней области сканирования, радиан
 * \param points Возвращаемый массив с координатами сканирования
 * \return true в случае успеха, иначе false
 */
bool SatViewPoint::countBorder(int vStep, float maxAngle, meteo::GeoVector* points) const
{
  if (!_isOk) {
    error_log<< QObject::tr("Не задано время начала и окончания сканирования");
    return false;
  }

  if (0 == vStep || 0 == points) {
    error_log<< QObject::tr("Неверные параметры для расчёта рамки");
    return false;
  }

  QDateTime cur(_dtStart);

  while (cur < _dtEnd) {
    GeoCoord left, right;
    bool ok = countViewedMaxPoints(cur, maxAngle, &left, &right);
    if (!ok) return false;
    points->append(meteo::GeoPoint(left.lat, left.lon));
    points->prepend(meteo::GeoPoint(right.lat, right.lon));

    //    cur = MnCommon::addMSecToQDT(cur, vStep);
    cur = cur.addMSecs(vStep);
  }

  return true;
}


//долгота +/-Pi
bool SatViewPoint::countMinMaxBorders(float gStep, int vStep, float maxAngle,
				      GeoCoord* min, GeoCoord* max) const
{
  if (!_isOk) {
    error_log<< QObject::tr("Не задано время начала и окончания сканирования");
    return false;
  }

  if (0 == vStep || nullptr == min || nullptr == max) {
    error_log<< QObject::tr("Неверные параметры для расчёта рамки") << vStep << min << max;
    return false;
  }

  min->lat = 9999;
  min->lon = 9999;
  max->lat = -9999;
  max->lon = -9999;
  
  GeoCoord geo;
  QVector<GeoCoord> line;
   
  //первая строка
  QDateTime cur(_dtStart);
  if (!getPosition( timeFromTLE(cur.addMSecs(-vStep)), &geo) ) {
    error_log<< QObject::tr("Ошибка определения координат");
    return false;
  }
  
  bool ok  = countScanLine(cur, gStep, maxAngle, line, &geo);
  if (!ok) return false;

  // for (auto p: line) {
  //   debug_log << p.lon*180/3.14 << p.lat*180/3.14;
  // }
  
  
  //боковые
  while (cur < _dtEnd) {
    GeoCoord left, right;
    bool ok = countViewedMaxPoints(cur, maxAngle, &left, &right);
    if (!ok) return false;
    line.prepend(left);
    line.append(right);
    cur = cur.addMSecs(vStep);
    //  debug_log << right.lon*180/3.14 << right.lat*180/3.14 << left.lon*180/3.14 << left.lat*180/3.14;
  }
  
  
  //последняя строка
  cur = _dtEnd;
  if (!getPosition( timeFromTLE(cur.addMSecs(-vStep)), &geo) ) {
    error_log<< QObject::tr("Ошибка определения координат");
    return false;
  }
  ok  = countScanLine(cur, gStep, maxAngle, line, &geo);
  if (!ok) return false;


  // for (auto p: line) {
  //   debug_log << p.lon*180/3.14 << p.lat*180/3.14;
  // }
  
  for (auto pnt : line) {
    if (pnt.lat < min->lat) {
      min->lat = pnt.lat;
    }
    if (pnt.lat > max->lat) {
      max->lat = pnt.lat;
    }
    float lon = pnt.lon;
    if (lon < min->lon) {
      min->lon = lon;
    }
    if (lon > max->lon) {
      max->lon = lon;
    }
  }

  
  return true;
}

/*!
 * \brief Расчет координат области сканирования. Данные добавляются в список от начального времени к конечному и от меньшей долготы к большей (на изображении слева на право, если спутник летит на север)
 * \param gStep Шаг по горизонтали (угол между FOV, в радианах)
 * \param vStep Шаг по вертикали, милисекунды
 * \param maxAngle Максимальный угол сканирования от надира до центра крайней области сканирования, радиан
 * \param points Возвращаемый массив с координатами сканирования
 * \return true в случае успеха, иначе false
 */
bool SatViewPoint::countGrid(float gStep, float vStep, float maxAngle, QVector<GeoCoord>& points) const
{
  if (!_isOk) {
    error_log<< QObject::tr("Не задано время начала и окончания сканирования");
    return false;
  }

  QDateTime cur(_dtStart);
  GeoCoord geo;
  if (!getPosition( timeFromTLE(cur.addMSecs(-vStep)), &geo) ) {
    return false;
  }

  float vSum = vStep;
  while (cur < _dtEnd) {
    bool ok  = countScanLine(cur, gStep, maxAngle, points, &geo);
    if (!ok) return false;
    cur = _dtStart.addMSecs(round(vSum));
    vSum += vStep;
  }

  return true;
}

/*!
 * \brief Расчет координат области сканирования. Данные добавляются в список от начального времени к конечному и от меньшей долготы к большей (на изображении слева на право, если спутник летит на север)
 * \param gStep Шаг по горизонтали (угол между FOV, в радианах)
 * \param vStep Шаг по вертикали, милисекунды
 * \param linesCnt Количество строк для расчёта
 * \param maxAngle Максимальный угол сканирования от надира до центра крайней области сканирования, радиан
 * \param points Возвращаемый массив с координатами сканирования
 * \return true в случае успеха, иначе false
 */
bool SatViewPoint::countGridPart(float gStep, int vStep, int linesCnt, float maxAngle, QVector<GeoCoord>& points) const
{
  if (!_isOk) {
    error_log<< QObject::tr("Не задано время начала и окончания сканирования");
    return false;
  }

  QDateTime cur(_dtStart);
  GeoCoord geo;
  if (!getPosition( timeFromTLE(cur.addMSecs(-vStep)), &geo) ) {
    error_log<< QObject::tr("Ошибка определения координат");
    return false;
  }

  while ( 0 < linesCnt--) {
    bool ok  = countScanLine(cur, gStep, maxAngle, points, &geo);
    if (!ok) return false;
    cur = cur.addMSecs(vStep);
  }

  return true;
}

/*!
 * \brief Расчёт координат линии сканирования. Данные добавляются в список слева на право и так, как будто спутник летит на север (т.е. от меньшей долготы к большей).
 * \param dt Момент времени, для которого рассчитывается линия сканирования, UTC
 * \param sigma_step Шаг, с которым необходимо рассчитать точки линии (углы сканирования), радиан
 * \param maxAngle Максимальный угол сканирования от надира до центра крайней области сканирования, радиан
 * \param points Возвращаемый массив указателей с координатами сканирования
 * \param num Индекс заполненяемого элемента массива points
 */
bool SatViewPoint::countScanLine( const QDateTime& dt, float sigma_step, float maxAngle,
				  QVector<GeoCoord>& points, GeoCoord* oldGeo ) const
{
  GeoCoord geo;
  if (!getPosition( timeFromTLE(dt), &geo) ) {
    return false;
  }

  //debug_log << "view" << oldGeo->lat*180/3.14 << geo.lat * 180/3.14;
  
  int sign = -1;
  if (oldGeo->lat> geo.lat) {
    sign = 1;
  }

  if (sign == 1) {
    float sigma = maxAngle;
    while ( sigma > 0) {
      //debug_log << sigma*180/3.14 << sign << -1;
      if (!addViewedPoint(geo, sigma, sign, -1, points)) {
	return false;
      }
      sigma -= sigma_step;
    }

    sigma = 0;
    while ( sigma <= maxAngle ) {
      //debug_log << sigma*180/3.14 << sign << 1;
      if (!addViewedPoint(geo, sigma, -sign, +1, points)) {
	return false;
      }
      sigma += sigma_step;
    }
  } else {
    float sigma =maxAngle;
    while ( sigma > 0) {
      //debug_log << sigma*180/3.14 << -sign << 1;
      if (!addViewedPoint(geo, sigma, -sign, +1, points)) {
	return false;
      }
      sigma -= sigma_step;
    }

    sigma = 0;
    while ( sigma <= maxAngle ) {
      //debug_log << sigma*180/3.14 << sign << -1;
      if (!addViewedPoint(geo, sigma, sign, -1, points)) {
	return false;
      }
      sigma += sigma_step;
    }
  }

  *oldGeo = geo;
  return true;
}

/*!
 * \brief Добавление рассчитанной точки сканирования
 * \param geo     Координаты спутника
 * \param sigma   Угол сканирования
 * \param sign    Знак для расчёта точки сканирования
 * \param lonSign Знак для добавления/вычитания отклонения долготы
 * \param points  Возвращаемый массив указателей с координатами сканирования
 * \return
 */
bool SatViewPoint::addViewedPoint(const GeoCoord& geo, float sigma, int sign, int lonSign,
				  QVector<GeoCoord>& points) const
{
  double v_lat, delta_lon;
  countViewedPoint(geo.lat, geo.alt, sigma, &v_lat, &delta_lon, sign);
  //points.append(GeoCoord( v_lat, fmod2p(geo.lon + lonSign*delta_lon))); //TODO не понятно, почему было так, будет не корректный переход при некоторых положениях
  //debug_log << "sat" << geo.lon * 180/3.14 << delta_lon*180/3.14 << (geo.lon + lonSign*delta_lon)*180/3.14 << v_lat*180/3.14;
  points.append(GeoCoord( v_lat, PiToPi(geo.lon) + lonSign*delta_lon));
  return true;
}



/*!
 * \brief расчёт положения точки сканирования (долгота принимается за 0-ое значение)
 * \param lat Широта спутника
 * \param alt Высота спутника
 * \param sigma Угол сканирования (угол от надира)
 * \param v_lat Широта заданной точки
 * \param delta_lon Долгота заданной точки относительно долготы спутника
 * \param sign знак для вычисления cos(alpha_L). Положительный, когда считаем точку в сторону увеличения долготы, от рицательный - в сторону уменьшения
 */
void SatViewPoint::countViewedPoint( float lat, float alt, double sigma,
				  double* v_lat, double* delta_lon, int sign) const
{
  double alpha_T;
  //the angle at the sub-point from north to the sub-track
  alpha_T = cos(inclination()) / cos(lat);
  if (fabs(inclination()) > M_PI_2) {
    alpha_T = spAsin( -alpha_T );
  } else {
    alpha_T = spAsin( alpha_T );
  }

  double alpha_L = alpha_T - M_PI_2; //the angle to the scan line
  //For a viewing nadir angle (scan angle) sigma, the corresponding geocentric arc distance:
  double theta = spAsin( (MEAN_RAD + alt) / MEAN_RAD * sin(sigma) ) - sigma;
  *v_lat = spAsin(sin(lat)*cos(theta) + cos(lat)*sin(theta)*cos(alpha_L)*sign);
  if (cos(*v_lat) == 0) {
    *delta_lon = 0;
  } else {
    double delta_cos = (cos(lat)*cos(theta) - sin(lat)*sin(theta)*cos(alpha_L)*sign) / cos(*v_lat);
    *delta_lon = spAcos( delta_cos );
  }
  //   printf("alpha_T=%f alpha_L=%f delta=%f lat=%f sign=%d\n", rad2deg(alpha_T), rad2deg(alpha_L),
  // 	 rad2deg(*delta_lon), rad2deg(*v_lat), sign);
}





/*!
 * \brief Расчёт  крайней левой и правой точек сканирования
 * \param geo      Координаты подспутниковой точки
 * \param maxAngle Максимальный угол сканирования от надира до центра крайней области сканирования, радиан
 * \param sign     Знак для расчёта точки сканирования
 * \param lgeo     Точка с меньшей долготой
 * \param mgeo     Точка с большей долготой
 */
void SatViewPoint::countViewedMaxPoints(const GeoCoord& geo, double maxAngle, int sign,
				     GeoCoord* lgeo, GeoCoord* mgeo) const
{
  double v_lat, delta_lon;
  countViewedPoint(geo.lat, geo.alt, maxAngle, &v_lat, &delta_lon, sign);
  lgeo->lat = v_lat;
  //  lgeo->lon = fmod2p(geo.lon - delta_lon);//TODO  не понятно, почему было так, будет не корректный переход при некоторых положениях
  lgeo->lon = PiToPi(geo.lon) - delta_lon;

  countViewedPoint(geo.lat, geo.alt, maxAngle, &v_lat, &delta_lon, -sign);
  mgeo->lat = v_lat;
  //  mgeo->lon = fmod2p(geo.lon + delta_lon); //TODO  не понятно, почему было так, будет не корректный переход при некоторых положениях
  mgeo->lon = PiToPi(geo.lon) + delta_lon;
}

/*!
 * \brief Расчёт крайней левой и правой точек сканирования
 * \param cur       Момент времени, соответствующий искомым точкам
 * \param maxAngle  Максимальный угол сканирования от надира до центра крайней области сканирования, радиан
 * \param left      Левая точка
 * \param right     Правая точка
 * \return  true в случае успеха, иначе false
 */
bool SatViewPoint::countViewedMaxPoints(const QDateTime& cur, float maxAngle,
				     GeoCoord* left, GeoCoord* right) const
{
  GeoCoord geo, oldGeo;

  if (!getPosition( timeFromTLE(cur.addSecs(-5)), &oldGeo) ) {
    return false;
  }
  if (!getPosition( timeFromTLE(cur), &geo) ) {
    return false;
  }

  int sign = -1;
  if (oldGeo.lat > geo.lat) {
    sign = 1;
  }

  if (sign == 1) {
    countViewedMaxPoints(geo, maxAngle, sign, left, right);
  } else {
    countViewedMaxPoints(geo, maxAngle, sign, right, left);
  }

  return true;
}

/*!
 * \brief Расчёт координат углов области сканирования
 * \param beg Момент времени, соответствующий началу сканирования
 * \param end Момент времени, соответствующий окончанию сканирования
 * \param maxAngle  Максимальный угол сканирования от надира до центра крайней области сканирования, радиан
 * \param bl Начальный левый
 * \param br Начальный правый
 * \param el Конечный левый
 * \param er Конечный правый
 * \return  true в случае успеха, иначе false
 */
bool SatViewPoint::countGridCorners(float maxAngle, meteo::GeoPoint* bl, meteo::GeoPoint* br, meteo::GeoPoint* el, meteo::GeoPoint* er) const
{
  if (!_isOk) {
    error_log<< QObject::tr("Не задано время начала и окончания сканирования");
    return false;
  }

  GeoCoord g1;
  GeoCoord g2;

  g1.lat=bl->lat();
  g1.lon=bl->lon();
  g2.lat=br->lat();
  g2.lon=br->lon();

  if (!countViewedMaxPoints(_dtStart, maxAngle, &g1, &g2)) {
    return false;
  }
  bl->setLat(g1.lat);
  bl->setLon(g1.lon);
  br->setLat(g2.lat);
  br->setLon(g2.lon);

  g1.lat=el->lat();
  g1.lon=el->lon();
  g2.lat=er->lat();
  g2.lon=er->lon();
  if (!countViewedMaxPoints(_dtEnd, maxAngle, &g1, &g2)) {
    return false;
  }

  el->setLat(g1.lat);
  el->setLon(g1.lon);
  er->setLat(g2.lat);
  er->setLon(g2.lon);
  return true;
}

/*!
 * \brief Расчёт координат углов области сканирования
 * \param beg Момент времени, соответствующий началу сканирования
 * \param end Момент времени, соответствующий окончанию сканирования
 * \param maxAngle  Максимальный угол сканирования от надира до центра крайней области сканирования, радиан
 * \param bl Начальный левый
 * \param br Начальный правый
 * \param el Конечный левый
 * \param er Конечный правый
 * \return  true в случае успеха, иначе false
 */
 bool SatViewPoint::countGridCorners(float maxAngle,
				     GeoCoord* bl, GeoCoord* br, GeoCoord* el, GeoCoord* er) const
{
  if (!_isOk) {
    error_log<< QObject::tr("Не задано время начала и окончания сканирования");
    return false;
  }

  if (!countViewedMaxPoints(_dtStart, maxAngle, bl, br)) {
    return false;
  }

  return countViewedMaxPoints(_dtEnd, maxAngle, el, er);
}

/*!
 * \brief Расчёт координат углов области сканирования
 * \param beg Момент времени, соответствующий началу сканирования
 * \param end Момент времени, соответствующий окончанию сканирования
 * \param maxAngle  Максимальный угол сканирования от надира до центра крайней области сканирования, радиан
 * \param gp Массив из 4 точек ( начальный левый, начальный правый, конечный левый, конечный правый )
 * \return  true в случае успеха, иначе false
 */
bool SatViewPoint::countGridCorners(float maxAngle, meteo::GeoPoint* gp ) const {
  return countGridCorners( maxAngle, gp, gp+1, gp+2, gp+3 );
}

/*!
 * \brief Расчёт координат углов области сканирования
 * \param beg Момент времени, соответствующий началу сканирования
 * \param end Момент времени, соответствующий окончанию сканирования
 * \param maxAngle  Максимальный угол сканирования от надира до центра крайней области сканирования, радиан
 * \param gc Массив из 4 точек ( начальный левый, начальный правый, конечный левый, конечный правый )
 * \return  true в случае успеха, иначе false
 */
bool SatViewPoint::countGridCorners(float maxAngle, GeoCoord* gc ) const {
  return countGridCorners( maxAngle, gc, gc+1, gc+2, gc+3 );
}

