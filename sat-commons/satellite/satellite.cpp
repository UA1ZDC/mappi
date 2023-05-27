#include "satellite.h"

#include <cross-commons/debug/tlog.h>

//#include <math.h>
// #include <new>

// #include <qfile.h>
// #include <qtextstream.h>
// //#include <Q3ValueList>
// #include <qfileinfo.h>

#include <cross-commons/funcs/mn_funcs.h>
#include <commons/mathtools/mnmath.h>
#include <commons/geobasis/projection.h>


// #include <hrpt_defines.h>

using namespace MnMath;
using namespace MnSat;
using namespace Coords;
// using namespace std;

#define LITTLE_VALUE 1.e-6

/*!
 * \brief  Чтение информации о спутнике, с которого получены данные
 * \param infoName Полный путь к инфо-файлу
 * \param satName  Возвращаемое название спутника
 * \param beg      Возвращаемое время начала сеанса
 * \param end      Возвращаемое время окончания сеанса
 * \return ERR_NOERR в случае успеха, иначе код ошибки
 */
// int MnSat::SatInfo::getSatInfo( const QString& infoName ) {
//   FILE* file = fopen(infoName.toLatin1(),"r");

//   if (!file) {
//     error_log << "Can't open file: '" << infoName << "'";
//     return ERR_OPENFILE;
//   }

//   char satN[11];
//   uint tB, tE;
//   MnSettings::StreamType stype;
//   if (1 != fread(&stype, sizeof(stype), 1, file) ||
//       1 != fread(satN, sizeof(satN), 1, file) ||
//       1 != fread(&tB, sizeof(tB), 1, file) ||
//       1 != fread(&tE, sizeof(tE), 1, file) ||
//       1 != fread(&site, sizeof(Coords::GeoCoord), 1, file) ||
//       1 != fread(&tle, sizeof(MnSat::TLEParams), 1, file)
//       ) {
//     error_log << "Can't open file: '" << infoName.toLatin1() << "'";
//     fclose(file);
//     return ERR_READFILE;
//   }
//   fclose(file);

//   streamType = stype;
//   name = satN;
//   beg.setTime_t(tB);
//   end.setTime_t(tE);

//   if (name.isNull() || !beg.isValid() || !end.isValid() ) {
//     error_log << "Can't open file: '" << infoName.toLatin1() << "'";
//     return ERR_READFILE;
//   }

//   return ERR_NOERR;
// }

// int MnSat::getSatInfo( const QString& infoName, MnSat::SatInfo& sInfo ) {
//   return sInfo.getSatInfo( infoName );
// }

const double HR_PER_DAY   = 24.0;          // Hours per day   (solar)



Satellite::Satellite()
{
}

Satellite::~Satellite()
{
}


/*!
 * \brief Установка параметров TLE
 * \param infoName Полное имя инфо-файла
 * \return ERR_NOERR в случае успешного прочтения инфо-файла, иначе - код ошибки
 */
// int Satellite::setTLEParams( const QString& infoName ) {
//   MnSat::SatInfo info;
//   int err = ERR_NOERR;
//   if ( ERR_NOERR != ( err = getSatInfo( infoName, info ) ) ) {
//     return err;
//   }
//   SatelliteBase::setTLEParams( info.tle );
//   return err;
// }

/*!
 * \brief Поиск моментов времени AOS и LOS
 * \param startTime Время, прошедшее с TLE эпохи, с которого будут начаты вычисления, в секундах
 * \param site Местоположение на земле, относительно которого вычисляется положение спутника
 * \param aos, los - Следующее после startTime моменты времени AOS и LOS, в секундах с TLE эпохи
 * \param threshold - Пороговое значения для угла места (минимальный угол), в радианах
 */
bool Satellite::search_AOS_LOS(time_t startTime, const GeoCoord& site,
			       time_t* aos, time_t* los, float threshold/* = 0.0*/)const
{
  time_t _t = startTime, _t0, _t1;
  double e0, e1;
  TopoCoord topo;
  double t, tprev, t0, t1;;

  if (!getPosition(startTime/60.0, site, &topo)) {
    return false;
  }

  for (uint i = 0; i < 2; i++) {
    /* first, a coarse search in steps of 5 minutes, to find the next time
       at which the elevation changes sign */
    do {
      e0 = topo.el;
      _t0 = _t;
      _t += 300;
      if (!getPosition(_t/60.0, site, &topo)) {
	return false;
      }
      //   printf("e0=%f topo=%f thr=%f\n", rad2deg(e0), rad2deg(topo.el), rad2deg(threshold));
    } while ( (_t < startTime + SEC_PER_DAY || i > 0) &&
	      !( ( (e0 < threshold) && (topo.el >= threshold) ) ||
		 ( (e0 >= threshold) && (topo.el < threshold ) ))
	      );


    if (  !( ( e0<threshold  && topo.el>=threshold)  ||
	     ( e0>=threshold && topo.el<threshold ) ) ) {
      //    if ((e0) * (topo.el) >= 0.0) {
      /* Satellite won't rise or set in the next 24 hours - possibly too low inclination? */
      *aos = *los = 0;
      return false;
    }

    e1 = topo.el;
    _t1 = _t;
    t = _t0 - 1;
    t0=_t0;
    t1=_t1;

    /* do a "regula falsi" search to quickly approximate the AOS/LOS time */
    while (true) {
      tprev = t;
      if ( (e0<=threshold) && (e1>threshold)) {
	t = (0.5 + ((e0-threshold) * t1 - (e1-threshold) * t0) / (e0 - e1));
      } else {
	t = (0.5 + ((e0-threshold) * t1 - (e1-threshold) * t0) / (e0 - e1));
      }
      //   printf("t=%lu t_prev=%lu t1=%lu t0=%lu e0=%f e1=%f thresh=%e tt=%f\n",
      // 	     t, tprev, t1, t0, rad2deg(e0), rad2deg(e1), threshold, 0.5 + ((e0-threshold) * t1 - (e1-threshold) * t0) / (e0 - e1));

      if ((t - tprev)<LITTLE_VALUE)
	break;

      if (! getPosition(t/60.0, site, &topo)) {
	return false;
      }


      if ( !( ( e0<=threshold && topo.el>threshold ) ||
	      ( e0>=threshold && topo.el<threshold ) )) {
	//  if ((e0) * (topo.el) >= 0.0) {
	e0 = topo.el;
	t0 = t;
	//	printf("1 e0=%f topo=%f\n", rad2deg(e0), rad2deg(topo.el));
      } else {
	e1 =  topo.el;
	t1 = t;
	//	printf("2 e1=%f topo=%f\n", rad2deg(e1), rad2deg(topo.el));
      }
    }

    //  printf("finally, check the AOS/LOS time\n");

    /* finally, check the AOS/LOS time and correct if necessary */
    if (t - t0< LITTLE_VALUE) {
      do {
	e0 =  topo.el;
	if ( !getPosition(++t/60.0, site, &topo) ) {
	  return false;
	}
	//	printf("e0=%f topo=%f\n", rad2deg(e0), rad2deg(topo.el));
      } while (!( ( (e0 < threshold) && (topo.el >= threshold) ) ||
		  ( (e0 >= threshold) && (topo.el < threshold ) ))
	       );//((e0) * ( topo.el) > 0.0);

      e1 = topo.el;
      t--;
      //  printf("++ e1=%f t=%lu\n", rad2deg(e1), t);
    } else {
      do {
	e1 = topo.el;
	t1 = t;
	if ( !getPosition(--t/60.0, site, &topo)) {
	  return false;
	}
	//	printf("e1=%f t=%lu\n", rad2deg(e1), t);
      } while (!( ( (e1 < threshold) && (topo.el >= threshold) ) ||
		  ( (e1 >= threshold) && (topo.el < threshold ) ))
	       );
      //((e1) * (topo.el) > 0.0);

      e0 = topo.el;
      //  printf("-- e0=%f t=%lu\n", rad2deg(e0), t);
    }

    /* round in the correct direction */
    if (fabs(e0) > fabs(e1)) t++;

    // printf("e0=%f e1=%f\n", rad2deg(e0),  rad2deg(e1));

    if (e1 > e0) {
      *aos = (time_t)t;
      //printf("\nel=%f e0=%f e1=%f az=%f\n", rad2deg(topo.el), rad2deg(e0), rad2deg(e1), rad2deg(topo.az));
    }  else {
      *los = (time_t)t;
      //printf("\nel=%f e0=%f e1=%f az=%f\n", rad2deg(topo.el), rad2deg(e0), rad2deg(e1), rad2deg(topo.az));
    }

    t = t1;
    topo.el = e1;
  }

  return true;
}


/*!
 * \brief Поиск кульминации
 * \param aos Время, прошедшее с TLE эпохи, с которого будут начаты вычисления, в секундах (момент появления спутника)
 * \param max Угол места, соответствующий кульминации
 */
bool Satellite::searchMaximum(time_t aos,  const GeoCoord& site, double* max, time_t* mos /* =0*/) const
{
  double e0;
  TopoCoord topo;

  if ( !getPosition(aos/60.0, site, &topo)) {
    return false;
  }
  //  //printf("e=%f\n", rad2deg(topo.el));
  do {
    e0 = topo.el;
    aos += 60;//сначало поминутно ищем
    if ( !getPosition(aos/60.0, site, &topo)) { return false; }
    //printf("min e=%f\n", rad2deg(topo.el));
  } while (e0 <= topo.el);

  aos -= 60;

  if ( !getPosition((aos+1)/60.0, site, &topo)) { return false; }
  //printf("r e=%f\n", rad2deg(topo.el));
  if ( e0 < topo.el) {
    aos +=1;
    //проверить значения вправо посекундно
    do {
      e0 = topo.el;
      if ( !getPosition(++aos/60.0, site, &topo)) { return false; }
      //printf("r e=%f\n", rad2deg(topo.el));
    } while (e0 <= topo.el);
  } else { //проверить влево посекундно
    topo.el = e0;

    do {
      e0 = topo.el;
      if ( !getPosition(--aos/60.0, site, &topo)) { return false; }
      //printf("l e=%f e0=%f\n", rad2deg(topo.el),  rad2deg(e0));
    } while (e0 <= topo.el);
  }
  *max = e0;
  if (0 != mos) {
    *mos = aos;
  }
  return true;
}

bool Satellite::searchNearLatTime(time_t aos, time_t los, const GeoCoord& site, time_t* near)const
{
  GeoCoord geo;
  double dPrev, d;
  time_t start=aos;

  if ( !getPosition(aos/60.0, &geo)) {
    return false;
  }

  GeoCoord p(site.lat, geo.lon);
  d = geoDistance(geo, p);

  do {
    dPrev = d;
    aos += 60;//сначало поминутно ищем
    if ( !getPosition(aos/60.0, &geo)) { return false; }
    p.lon = geo.lon;
    d = geoDistance(geo, p);
  } while(d <= dPrev && aos < los);

  aos -= 60;

  if ( !getPosition((aos+1)/60.0, &geo)) { return false; }
  p.lon = geo.lon;
  d = geoDistance(geo, p);

  if (d < dPrev) {
    aos +=1;
    //проверить значения вправо посекундно
    do {
      dPrev = d;
      if ( !getPosition(++aos/60.0, &geo)) { return false; }
      p.lon = geo.lon;
      d = geoDistance(geo, p);
    } while (d <= dPrev && aos < los );
  } else { //проверить влево посекундно
    d = dPrev;

    do {
      dPrev = d;
      if ( !getPosition(--aos/60.0, &geo)) { return false; }
      p.lon = geo.lon;
      d = geoDistance(geo, p);
    } while (d <= dPrev && aos > start);
  }

  *near = aos;
  return true;
}

bool Satellite::searchNearLonTime(time_t aos, time_t los, const GeoCoord& site, time_t* near)const
{
  GeoCoord geo;
  double dPrev, d;
  time_t start=aos;

  if ( !getPosition(aos/60.0, &geo)) {
    return false;
  }

  GeoCoord p(geo.lat, site.lon);
  d = geoDistance(geo, p);

  do {
    dPrev = d;
    aos += 60;//сначало поминутно ищем
    if ( !getPosition(aos/60.0, &geo)) { return false; }
    p.lat = geo.lat;
    d = geoDistance(geo, p);
  } while(d <= dPrev && aos < los);

  aos -= 60;

  if ( !getPosition((aos+1)/60.0, &geo)) { return false; }
  p.lat = geo.lat;
  d = geoDistance(geo, p);

  if (d < dPrev) {
    aos +=1;
    //проверить значения вправо посекундно
    do {
      dPrev = d;
      if ( !getPosition(++aos/60.0, &geo)) { return false; }
      p.lat = geo.lat;
      d = geoDistance(geo, p);
    } while (d <= dPrev && aos < los );
  } else { //проверить влево посекундно
    d = dPrev;

    do {
      dPrev = d;
      if ( !getPosition(--aos/60.0, &geo)) { return false; }
      p.lat = geo.lat;
      d = geoDistance(geo, p);
    } while (d <= dPrev && aos > start);
  }

  *near = aos;
  return true;
}

/*!
 * \brief Составление траектории полёта спутника в топографических координатах (для антенны)
 * \param beg Начальное время (в UTC)
 * \param end Конечное время (в UTC)
 * \param step Шаг по времени, секунд
 * \param site Местоположение на земле, относительно которого вычисляется положение спутника
 * \param track Возвращаемая траектория
 * \return true в случае успеха, иначе false
 */
bool Satellite::trajectory(const QDateTime& beg, const QDateTime& end, float step,
         const meteo::GeoPoint& asite, QList<MnSat::TrackTopoPoint>& track)
{
  Coords::GeoCoord site(asite.lat(), asite.lon(), asite.alt()/1000.);
  return trajectory(beg, end, step, site, track);
}


/*!
 * \brief Составление траектории полёта спутника в топографических координатах (для антенны)
 * \param beg Начальное время (в UTC)
 * \param end Конечное время (в UTC)
 * \param step Шаг по времени, секунд
 * \param site Местоположение на земле, относительно которого вычисляется положение спутника
 * \param track Возвращаемая траектория
 * \return true в случае успеха, иначе false
 */
bool Satellite::trajectory(const QDateTime& beg, const QDateTime& end, float step,
				 const Coords::GeoCoord& site,
				 QList<MnSat::TrackTopoPoint>& track)
{
	track.clear();

  // FILE* file = fopen(fileN.toLatin1(), "w");
  // if (!file) return false;

  // fprintf(file, "%s, %s\n", beg.toString(Qt::ISODate).toLatin1().data(), end.toString(Qt::ISODate).toLatin1().data());

  double tsince = timeFromTLE(beg);//in minutes
  double tsinceE = timeFromTLE(end);
  TopoPoint topo;
  QDateTime cur = beg;

  while (tsince < tsinceE) {
    if (! getPosition(tsince, site, &topo) || topo.pos.el < 0) {
      //      fclose(file);
      return false;
    }

    TrackTopoPoint point(topo.pos.az, topo.pos.el, topo.vel.az, topo.vel.el, cur);
    track.append(point);

    // fprintf(file, "%s %f %f\n", cur.toString("hh:mm:ss.zzz").toLatin1().data(), MnMath::rad2deg(topo.pos.az),  MnMath::rad2deg(topo.pos.el));

    tsince += step / 60.0;
    cur = cur.addMSecs(step*1000);
  }

  //  fclose(file);
  return true;
}

/*!
 * \brief Составление траектории полёта спутника в топографических координатах (для антенны)
 * \param beg Начальное время (в UTC)
 * \param end Конечное время (в UTC)
 * \param step Шаг по времени, секунд
 * \param site Местоположение на земле, относительно которого вычисляется положение спутника
 * \param track Возвращаемая траектория
 * \return true в случае успеха, иначе false
 */
bool Satellite::trajectory(const QDateTime& beg, const QDateTime& end, float step, const GeoCoord& site,
         const QString& fileN, QVector<TrackTopoPoint>& track)
{
  track.clear();

  FILE* file = fopen(fileN.toLatin1(), "w");
  if (!file) return false;

  fprintf(file, "%s, %s\n", beg.toString(Qt::ISODate).toLatin1().data(), end.toString(Qt::ISODate).toLatin1().data());

  double tsince = timeFromTLE(beg);//in minutes
  double tsinceE = timeFromTLE(end);
  TopoPoint topo;
  QDateTime cur = beg;

  while (tsince < tsinceE) {
    if (! getPosition(tsince, site, &topo) || topo.pos.el < 0) {
      fclose(file);
      return false;
    }

    TrackTopoPoint point(topo.pos.az, topo.pos.el, topo.vel.az, topo.vel.el, cur);
    track.append(point);

    fprintf(file, "%f %f\n", topo.pos.az,  topo.pos.el);

    tsince += step / 60.0;
    cur = cur.addMSecs(step*1000);
  }

  fclose(file);
  return true;
}

/*!
 * \brief Составление траектории полёта спутника в геодезических координатах
 * \param beg Начальное время (в UTC)
 * \param end Конечное время (в UTC)
 * \param step Шаг по времени, секунд
 * \param site Местоположение на земле, относительно которого вычисляется положение спутника
 * \param track Возвращаемая траектория
 * \return true в случае успеха, иначе false
 */
bool Satellite::trajectory(const QDateTime& beg, const QDateTime& end, float step,
			   QVector<GeoCoord>& track)
{
  track.clear();

  double tsince = timeFromTLE(beg);//in minutes
  double tsinceE = timeFromTLE(end);
  GeoCoord geo;

  while (tsince < tsinceE) {
    if (! getPosition(tsince, &geo)) {
      return false;
    }
    track.append(geo);

    tsince += step / 60.0;
  }
  return true;
}

/*!
 * \brief Составление траектории полёта спутника в геоцентрических координатах
 * \param beg Начальное время (в UTC)
 * \param end Конечное время (в UTC)
 * \param step Шаг по времени, секунд
 * \param track Возвращаемая траектория
 * \return true в случае успеха, иначе false
 */
bool Satellite::trajectory(const QDateTime& beg, const QDateTime& end, float step,
			   QVector<EciPoint>& track)
{
  track.clear();

  double tsince = timeFromTLE(beg);//in minutes
  double tsinceE = timeFromTLE(end);
  EciPoint eci;

  while (tsince < tsinceE)
    {
      if (!getPosition(tsince, &eci))
	{
	  return false;
	}

      track.append(eci);
      tsince += step / 60.0;
    }
  return true;
}

/*!
 * \brief Составление траектории полёта спутника в геодезических координатах. Расчёт заканчивается, когда дуга, соединяющая точку местоположения на земле и точку траектории спутника, станет больше maxAngle радиан
 * \param beg Начальное время (в UTC)
 * \param step Шаг по времени, секунд
 * \param maxAngle Максимальное значение дуги, соединяющей точку местоположения на земле и точку траектории спутника, радиан
 * \param site Местоположение на земле, относительно которого вычисляется положение спутника
 * \param futur Возвращаемая траектория, которую ещё лететь
 * \param past  Возвращаемая траектория, которую уже пролетели
 * \return true в случае успеха, иначе false
 */
bool Satellite::trajectory(const QDateTime& beg, float step, float maxAngle, const GeoCoord& site,
			   QVector<GeoCoord>& futur, QVector<GeoCoord>& past)
{
  //TODO если задать слишком большой maxAngle, то мы можем никогда не достичь этого значения(уйдём в бесконечный цикл).. надо бы это как-то предотвратить :) можно проверить, что сначала мы приближались к maxAngle, а потом начали отдаляться.. или как-нить от наклонения посчитать максимально возможное теоретическое значение отклонения спутника от местоположения на Земле

  futur.clear();
  past.clear();

  double tsince = timeFromTLE(beg);//in minutes

  GeoCoord geo;
  //float cos_d=0;
  float d  = 0;

  maxAngle =  MnMath::fmod2p( maxAngle );

  //в сторону обратную движению спутника
  do {
    if (! getPosition(tsince, &geo)) {
      return false;
    }

    past.append(geo);

    tsince -= step / 60.0;

    //     float a = spAcos(cos(geo.lat)*cos(fabs(geo.lon-site.lon)));
    //     cos_d = cos(site.lat)*cos(a) + sin(site.lat)*sin(a)*cos(M_PI_2 - sin(geo.lat)/sin(a));
    //     d = spAcos(cos_d);
    d = geoDistance(geo, site);

    //     printf("max=%.2f d=%.2f  cos_m=%f cos_d=%f lat=%f lon=%.2f lat_s=%.2f lon_s=%.2f\n",
    // 	   rad2deg(maxAngle), rad2deg(d), cos(maxAngle), cos_d, rad2deg(geo.lat), rad2deg(geo.lon), rad2deg(site.lat), rad2deg(site.lon) );

    //  } while (!( point.lat < (lat_b + deg2rad(5)) &&  point.lat > (lat_b - deg2rad(5))) );
    //  } while ( 0 <= cos(fabs(point.lat - site.lat))*cos(fabs(point.lon - site.lon)) );
  } while ( maxAngle > d );


  tsince = timeFromTLE(beg);// + step / 60.0;
  //в сторону движения спутника
  do {
    if (! getPosition(tsince, &geo)) {
      return false;
    }
    futur.append(geo);

    tsince += step / 60.0;

    //     float a = spAcos(cos(geo.lat)*cos(fabs(geo.lon-site.lon)));
    //     float A = spAsin(sin(geo.lat)/sin(a));
    //     cos_d = cos(site.lat)*cos(a) + sin(site.lat)*sin(a)*cos(M_PI_2 - A);
    //     // cos_d = cos(site.lat)*cos(a) + sin(site.lat)*sin(a)*cos(M_PI_2 - asin(sin(geo.lat)/sin(a)));

    //     d = spAcos(cos_d);

    d = geoDistance(geo, site);

    //  printf("max=%.2f d=%.2f  cos_m=%f cos_d=%f lat=%f lon=%.2f lat_s=%.2f lon_s=%.2f\n",
    // 	   rad2deg(maxAngle), rad2deg(d), cos(maxAngle), cos_d, rad2deg(geo.lat), rad2deg(geo.lon), rad2deg(site.lat), rad2deg(site.lon) );x

    //  } while (!( point.lat < (lat_e + deg2rad(5)) &&  point.lat > (lat_e - deg2rad(5))) );
    //  } while ( 0 <= cos(fabs(point.lat - site.lat))*cos(fabs(point.lon - site.lon)) );
    // } while ( 0 <= cos_d );
  } while ( maxAngle > d );

  return true;
}



/*!
 * \brief  определение направления движения спутника (восходящее, нисходящее)
 * \param beg время начала приёма
 * \param end время окончания приёма
 * \return  Направление (-1 - неизвестно,0 - C->Ю, 1 - Ю->С)
 */
int Satellite::direction(const QDateTime& beg, const QDateTime& end)
{
  if (meanMotion() < LITTLE_VALUE ) {
    error_log << "Неверно определены параметры орбиты. meanMotion == 0. Деление на 0.";
    return -1;
  }

  double aos = (timeFromTLE(beg))*60.;
  JullianDate jd(end);
  JullianDate jd2(beg);
  // фактическое время начала записи
  double Tnr=jd2.fromJan1_12h_1900()*86400.;
  // фактическое время окончания записи
  double Tkr=jd.fromJan1_12h_1900()*86400.;

  Coords::EciPoint eci;
  Coords::GeoCoord geo;
  getPosition(aos/60.,&eci);
  geo=eci.posToGeo();

  double vs=sin(geo.lat)/sin(inclination());
  if(vs>1.) vs=1.;
  double L_s=asin(vs);

  if(eci.vel.z < 0) {
    L_s=M_PI-L_s;
  }

  //Период обращения
  double TKA=86400./meanMotion();
  // время восходящего узла
  double T0 = Tnr - TKA * L_s / MnMath::M_2PI;
  // время вершины
  double Tb = T0 + TKA / 4.0;

  double ttmp = (Tnr + Tkr) / 2.0;
  if (ttmp < Tb) return 1;

  return 0;
}


// /*!
//  * \brief Расчёт параметров сеанса спутника (подспутниковая точка в начале и конце сеанса связи, середина передаваемого изображение и другое) - используется для географической привязки (почти что преведущая, только 2 входными параметрами указан временной интервал, в котором должен находиться сеанс спутника)
//  * \param type тип изображение
//  * \param dt время начала интервала времени когда должен состояться сеанс
//  * \param dtends время начала интервала времени когда должен закончиться сеанс
//  * \param *orbParam возвращаемая структура параметров сеанса спутника
//  * \return  true в случае нахождения сеанса спутника в заданном временном интервале, иначе false
//  */

// bool Satellite::sat2TOrbParam(TOrbParam *orbParam, QDateTime dt,QDateTime dtends, int type, int width, int height )
// {
//   if (0 >= width || 0> height) {
//     return false;
//   }

//   if (meanMotion() < LITTLE_VALUE ) {
//     lclog(LOG_ERROR, "Неверно определены параметры орбиты.");
//     return false;
//   }

//   orbParam->type = type;
//   double nak=orbParam->Nak=inclination();
//   // TODO проверку на отличие от нуля с указанной степенью точности
//   if ( meanMotion() == 0 )
//   {
//     lclog(LOG_ERROR,"meanMotion == 0 division by zero");
//     return false;
//   }
//   orbParam->TKA=86400./meanMotion();

//   //определяем тип сканера
//   orbParam->Lon=0.;
//   switch (orbParam->type){
//     case HRPT_NOAA:
//     case HRPT_FY:
//       orbParam->NPix = 2048;
//     //   orbParam->NPix = width;
//       orbParam->Nbeg = 0;
//       orbParam->Nend = 0;
//       orbParam->Fsc = 60; // частота сканирования
//       orbParam->Btm = 55.37 * PiI180;
//       orbParam->Xmax=width;
//       //       orbParam->Xmax=2048;
//       break;
//     case HRPT_METEOR:
//       orbParam->NPix = 1540;
//     //   orbParam->NPix = width;
//       orbParam->Nbeg = 0;
//       orbParam->Nend = 0;
//       orbParam->Fsc = 65; // частота сканирования
//       orbParam->Btm = 54. * PiI180;
//       orbParam->Xmax=width;
//       //       orbParam->Xmax=2048;
//       break;
//     case APT_NOAA:
//       orbParam->NPix = 1048;
//       orbParam->Nbeg = 45;
//       orbParam->Nend = 32;
//       orbParam->Fsc = 60;
//       orbParam->Btm = 55.37 * PiI180;
//       orbParam->Xmax=width;
//       break;
//     case APT_OKEAN:
//       orbParam->NPix = 954; //1200;
//       orbParam->Nbeg = 1; //184;
//       orbParam->Nend = 1; //60;
//       orbParam->Fsc = 20;
//       orbParam->Btm = 50.1 * PiI180;  // ???
//       break;
//     case APT_RESURS:
//       orbParam->NPix = 954; // 1200;
//       orbParam->Nbeg = 1; // 168;
//       orbParam->Nend = 1; // 78;
//       orbParam->Fsc = 20;
//       orbParam->Btm = 50.1 * PiI180;  // ???
//       break;
//     case APT_METEOR:
//       orbParam->NPix = 954;
//       orbParam->Nbeg = 1; // 184;
//       orbParam->Nend = 1; // 60;
//       orbParam->Fsc = 20;
//       orbParam->Btm = 45.0 * PiI180;  // ???
//       break;
//     case APT_METEOR_Cut:
//       orbParam->NPix = 954;
//       orbParam->Nbeg = 0;
//       orbParam->Nend = 0;
//       orbParam->Fsc = 20;
//       orbParam->Btm = 45.0 * PiI180;  // ???
//       break;
//     case WEFAX:
//       orbParam->NPix = 600;
//       orbParam->Nbeg = 2;
//       orbParam->Nend = 42;
//       orbParam->Fsc = 0;
//       orbParam->Btm = 0;
//       orbParam->Xmax=width;
//       orbParam->Lon=63.;
//       orbParam->Up = 1;
//       //       orbParam->Xmax=2048;
//       break;
//     default:
//       orbParam->NPix = 600;
//       orbParam->Nbeg = 47;
//       orbParam->Nend = 30;
//       orbParam->Fsc = 20;
//       orbParam->Btm = 55.37 * PiI180;
//       break;
//   } // end switch
// //  orbParam->Nimg05 = ftoi_norm((orbParam->NPix - orbParam->Nbeg - orbParam->Nend)*0.5);
// //  orbParam->Ximg05 = orbParam->Nimg05 + orbParam->Nbeg;
//   // определяем время начала и окончания записи
// //  double curTime = timeFromTLE(dt);

//   double aos,los;
// //  time_t aos_t,los_t;

// //  search_AOS_LOS((time_t)(curTime*60.), site, &aos_t, &los_t, deg2rad(angle));
// //  los=los_t*60.;
// //  aos=aos_t*60.;
// //  QDateTime d_aos=QDateTime::currentDateTime(Qt::UTC);
// //  QDateTime d_los=QDateTime::currentDateTime(Qt::UTC);
//   //QDateTime temp_aos=QDateTime::currentDateTime(Qt::UTC);
//   //QDateTime temp_los=QDateTime::currentDateTime(Qt::UTC);

// //  d_aos=dt.addSecs(ftoi_norm(aos - curTime*60.));
// //  d_los=dt.addSecs(ftoi_norm(los - curTime*60.));
//   aos = ((timeFromTLE(dt))*60.);
//   los = ((timeFromTLE(dtends))*60.);
// //  debug(dt.toString());
// //  debug(dtends.toString());
//   // if(d_aos>d_los)//возврат
//   //   {return false;}
//   // //возврат отрицания при выходе времени сеанса за заданные параметры
//   // if((d_aos<temp_aos)||(d_aos>dtends)||(d_los>temp_los)||(d_los<dt))
//   //   {return false;}
//   //dt=dt.addSecs(-30);
//   //dt=dtends.addSecs(30);
//   JullianDate jd(dtends);
//   JullianDate jd2(dt);
//   // фактическое время начала записи
//   orbParam->Tnr=jd2.fromJan1_12h_1900()*86400.;
//   // фактическое время окончания записи
//   orbParam->Tkr=jd.fromJan1_12h_1900()*86400.;
//   orbParam->Tn=orbParam->Tnr;
//   Coords::EciPoint eci;
//   Coords::GeoCoord geo;

//   getPosition(aos/60.,&eci);
//   orbParam->Tnr_utc=aos;
//   geo=eci.posToGeo();
//   //широта подсп. точки в момент начала записи
//   orbParam->FiKAnr=geo.lat;
//   // долгота подсп. точки в момент начала записи
//   orbParam->LdKAnr=geo.lon;

//   orbParam->Vz=eci.vel.z;
//   orbParam->RKA=eci.pos.r;

//   orbParam->Alfm = asin(orbParam->RKA/R_E*sin(orbParam->Btm))-orbParam->Btm;
//   orbParam->Nimg05 = ftoi_norm((orbParam->NPix - orbParam->Nbeg - orbParam->Nend)*0.5);
//   orbParam->Ximg05 = orbParam->Nimg05 + orbParam->Nbeg;


//   getPosition(los/60.,&eci);
//   orbParam->Tkr_utc=los;
//   geo=eci.posToGeo();
//   //широта подсп. точки в момент окончания записи
//   orbParam->FiKAkr=geo.lat;
//   // долгота подсп. точки в момент окончания записи
//   orbParam->LdKAkr=geo.lon;
//   orbParam->LdKAkr=PiToPi(orbParam->LdKAkr);

//   double fiKA=orbParam->FiKAnr;
//   orbParam->FiKA=orbParam->FiKAnr;
//   double ldKA=orbParam->LdKA=orbParam->LdKAnr;

//   double vs=sin(fiKA)/sin(nak);
//   if(vs>1.) vs=1.;
//   double L_s=asin(vs);
//   double dLd_s;

//   if(fabs(nak-M_2PI)<LITTLE_VALUE) dLd_s=0.;
//   else
//     {
//       vs=fabs(tan(fiKA)/tan(nak));
//       if(vs>1.) vs=1.;
//       dLd_s=asin(vs);
//     }
//   if(orbParam->Vz<0)
//     {L_s=M_PI-L_s;
//       dLd_s=M_PI-dLd_s;
//     }
//   if(nak>M_PI_2) dLd_s=-dLd_s;

//   // время восходящего узла
//   orbParam->T0 = orbParam->Tnr - orbParam->TKA * L_s / M_2PI;
//   // время нисходящего узла
//   orbParam->Tk = orbParam->T0 + orbParam->TKA / 2.0;
//   // время вершины
//   orbParam->Tb = orbParam->T0 + orbParam->TKA / 4.0;

//   //расчитываем длину картинки
//   //orbParam->Ymax=ftoi_norm((orbParam->Tkr -orbParam->Tnr)*orbParam->Fsc);
//   orbParam->Ymax=height;

//   // долгота восх. узла
//   orbParam->LdKA0 = ldKA - dLd_s + M_PI *(orbParam->Tn - orbParam->T0)/z_05t;
//   // долгота нисх. узла
//   orbParam->LdKAk = orbParam->LdKA0 + M_PI - orbParam->TKA * M_PI_2 / z_05t;

//   orbParam->LdKA0=PiToPi(orbParam->LdKA0);
//   orbParam->LdKAk=PiToPi(orbParam->LdKAk);

//   orbParam->U_KAnr = L_s;

//   getPosition((aos+los)*0.5/60.,&eci);
//   geo=eci.posToGeo();
//   //широта подсп. точки в момент окончания записи
//   orbParam->FiCntr=geo.lat;
//   // долгота подсп. точки в момент окончания записи
//   orbParam->LdCntr=geo.lon;
//   orbParam->LdCntr=PiToPi(orbParam->LdCntr);



//   countGridCorners(dt, dtends, orbParam->Btm, &orbParam->vl, &orbParam->vp, &orbParam->nl, &orbParam->np);

//   double ttmp = (orbParam->Tnr + orbParam->Tkr) / 2.0;
//   if (ttmp < orbParam->Tb) orbParam->Up = 1;
//   else orbParam->Up = 0;
//   return true;
// }



/*!
 * \brief Перевод сферических координат в спутниковые
 * \param lat  географическая широта в радианах
 * \param lon географическая долгота в радианах
 * \param time время от начала кадра в секундах
 * \param jj угол сканирования в радианах (от минус максимльного, до плюс максимального аппаратного)
 * \return
 */
/*bool MnSat::spherToSat(const Satellite& sat, const TOrbParam *opar, double lat, double lon,
  double *time, double *jj)
  {
  int spin;
  //  double  x, y, z, x_hd, y_hd, z_hd;
  double v_x, v_y, v_z;
  double t, t1, t2;
  //  double x_ed, y_ed, z_ed;
  //double x_m, y_m, z_m;
  double scal_hd, scal_ed, scal, cos_j, r2;
  *time = 0.0;
  *jj = 0.0;

  JullianDate jd;
  Coords::EciCoord eci;
  Coords::EciPoint eciSat;

  //заданные координаты и координаты спутника в середине приёма
  jd.setfromJan1_12h_1900((opar->Tnr+opar->Tkr)/2.0/86400.);
  eci = Coords::geoToEci(GeoCoord(lat, lon, 0), jd.toLGMST(lon));
  sat.getPosition((opar->Tnr_utc + opar->Tkr_utc)/2./60.0, &eciSat);
  if(eci.x * eciSat.pos.x + eci.y*eciSat.pos.y + eci.z*eciSat.pos.z < 0.0) {
  return false;
  }

  //заданные координаты и координаты спутника в начальный момент времени минус 1 минута
  jd.setfromJan1_12h_1900((opar->Tnr-60.)/86400.);
  eci = Coords::geoToEci(GeoCoord(lat, lon, 0), jd.toLGMST(lon));
  sat.getPosition(opar->Tnr_utc/60.0-1, &eciSat);
  scal_hd = eciSat.vel.x*(eci.x - eciSat.pos.x) +
  eciSat.vel.y*(eci.y - eciSat.pos.y) +
  eciSat.vel.z*(eci.z - eciSat.pos.z);
  if( scal_hd < 0.0 ) {
  return false;
  }

  //заданные координаты и координаты спутника в конечный момент времени плюс 1 минута
  jd.setfromJan1_12h_1900((opar->Tkr+60.)/86400.);
  eci = Coords::geoToEci(GeoCoord(lat, lon, 0), jd.toLGMST(lon));
  sat.getPosition(opar->Tkr_utc/60.0+1, &eciSat);
  scal_ed = eciSat.vel.x*(eci.x - eciSat.pos.x) +
  eciSat.vel.y*(eci.y - eciSat.pos.y) +
  eciSat.vel.z*(eci.z - eciSat.pos.z);
  if( scal_ed > 0.0 ) {
  return false;
  }

  t1 = opar->Tnr_utc/60.0 - 1; //начало приёма - 1, в минутах
  t2 = opar->Tkr_utc/60.0 + 1; //конец приёма +1, в минутах

  spin = 0;

  float  m_R2_max = sin(opar->Alfm)*R_E/sin(opar->Btm)*1.1;
  m_R2_max = m_R2_max*m_R2_max; // max допустимое расстояние от спутника до точки на Земле

  //поиск подспутниковой точки
  do {
  t = t1 - scal_hd*(t2-t1)/(scal_ed - scal_hd);
  //		t = (t1+t2)*0.5;

  sat.getPosition(t, &eciSat);
  jd = sat.epoch();
  jd.addMin(t);
  eci = Coords::geoToEci(GeoCoord(lat, lon, 0), jd.toLGMST(lon));
  scal = (eci.x - eciSat.pos.x)*eciSat.vel.x +
  (eci.y - eciSat.pos.y)*eciSat.vel.y +
  (eci.z - eciSat.pos.z)*eciSat.vel.z;
  if(scal>0.0) {
  t1 = t;
  scal_hd = scal;
  } else {
  t2 = t;
  scal_ed = scal;
  }
  spin ++;
  } while(fabs(scal) > 1e-5 && spin < 10);


  r2 = (eciSat.pos.x-eci.x)*(eciSat.pos.x-eci.x) +
  (eciSat.pos.y-eci.y)*(eciSat.pos.y-eci.y) +
  (eciSat.pos.z-eci.z)*(eciSat.pos.z-eci.z);
  if(r2 > m_R2_max) {
  return false;
  }

  //поиск угла по теореме косинусов
  scal = eciSat.pos.x*(eciSat.pos.x-eci.x) +
  eciSat.pos.y*(eciSat.pos.y-eci.y) +
  eciSat.pos.z*(eciSat.pos.z-eci.z);

  cos_j = scal/sqrt((eciSat.pos.x*eciSat.pos.x + eciSat.pos.y*eciSat.pos.y + eciSat.pos.z*eciSat.pos.z) *
  ((eciSat.pos.x-eci.x)*(eciSat.pos.x-eci.x) + (eciSat.pos.y-eci.y)*(eciSat.pos.y-eci.y) + (eciSat.pos.z-eci.z)*(eciSat.pos.z-eci.z)));

  if( cos_j < cos(1.1*opar->Btm) ) {
  return false;
  }

  *jj = acos(cos_j);
  *time = (t - opar->Tnr_utc/60.0)*60.0;

  v_x = eciSat.pos.z*(eciSat.pos.y-eci.y)-eciSat.pos.y*(eciSat.pos.z-eci.z);
  v_y = eciSat.pos.x*(eciSat.pos.z-eci.z)-eciSat.pos.z*(eciSat.pos.x-eci.x);
  v_z = eciSat.pos.y*(eciSat.pos.x-eci.x)-eciSat.pos.x*(eciSat.pos.y-eci.y);
  scal = v_x*eciSat.vel.x + v_y*eciSat.vel.y + v_z*eciSat.vel.z;
  if(scal<0.0) {
  *jj = -(*jj);
  }

  return true;
  }
*/
/*!
 * \brief получение времени видимости траектории полёта на карте  в зависимости проекции карты
 * \param beg Начальное время (в UTC)
 * \param step Шаг по времени, секунд
 * \param type_map тип карты (STEREO,MERCAT)
 * \param ret_time_start  Возвращаемое время вхождения спутника в зону видимости на карте
 * \param ret_time_end  Возвращаемое время вхождения спутника в зону видимости на карте
 * \return true в случае успеха, иначе false
 */

bool Satellite::trajectoryTimeIn(const QDateTime& beg, const QDateTime& end, float step,int type_map,QDateTime& ret_time_start,QDateTime& ret_time_end,bool ug_)
{
  double tsince = timeFromTLE(beg);//in minutes
  double tsinceE = timeFromTLE(end);
  //  double tm_tsince=0.,tm_tsince_pre=0.,tm_tsinceE=0.,pre_site=0.,past_site=0.,delta_sec=0.,delta_sec2=0.;
  double tm_tsince=0.,tm_tsince_pre=0.,tm_tsinceE=0.,past_site=0.,delta_sec=0.,delta_sec2=0.;

  GeoCoord geo,site_s,geo1,geo2,geo3;
  double porog_t=0.0,d1_=0.,d2_=0.,d3_=0.;
  int direction_t=2,dbu_p=0;
  QDateTime vst_time_start;
  bool pok_maps=false;

  ret_time_end=beg.addSecs((int)-step);
  ret_time_start=beg;
  vst_time_start=beg;

  if (!getPosition(tsince, &geo1))
    {return false;}

  if (!getPosition((tsince+5.), &geo2))
    {return false;}

  d1_ = geoDistance(geo1,geo2);
  if (d1_<0.05) return true;

  switch (type_map)
    {
    case meteo::STEREO:

      if(ug_) porog_t=-0.2;
      else porog_t=0.2;

      tsince = timeFromTLE(beg);//in minutes
      tsinceE = timeFromTLE(end);
      tm_tsince = tsince;//in minutes
      tm_tsinceE = tsinceE;
      tm_tsince_pre=tsince-(tsinceE-tsince);

      pok_maps=false;
      if (!getPosition(tsince, &geo))
	{return false;}

      if (((geo.lat>porog_t)&&(ug_))||((geo.lat<porog_t)&&(!ug_)))
	{pok_maps=true;}

      geo1.lat=deg2rad(90.);
      geo1.lon=deg2rad(0.);

      //if (!getPosition(tsince, &geo))
      // {return false;}
      geo.lon=geo1.lon;
      d1_ = geoDistance(geo,geo1);

      if (!getPosition((tsince+.25), &geo2))
	{return false;}
      geo2.lon=geo1.lon;
      d2_ = geoDistance(geo2,geo1);

      if (!getPosition((tsince+2.*(.25)), &geo3))
	{return false;}
      geo3.lon=geo1.lon;
      d3_ = geoDistance(geo3,geo1);

      //найти куда он летит
      if (!getPosition(tsince, &geo))
	{return false;}
      if (ug_)
	{
	  direction_t=2;
	  if((d1_>d2_)||(d2_>d3_))
	    direction_t=0;//с юга на север
	  if((d1_<d2_)||(d2_<d3_))
	    direction_t=1;//.с севера наюг

	  if((!pok_maps)&&(direction_t==0))//загнать на карту - недолет
	    {
	      //while((geo.lat<porog_t)&&(dbu_p<1000)&&(tm_tsince<tsinceE))
	      while((geo.lat<porog_t)&&(dbu_p<1000))
                {
		  tm_tsince +=1.;//сначало поминутно ищем
		  if (!getPosition(tm_tsince,&geo)) {return false;}
		  dbu_p++;
                }
	    }

	  if((!pok_maps)&&(direction_t==1))//загнать на карту - перелет
	    {
	      while((geo.lat<porog_t)&&(dbu_p<1000)&&(tm_tsince>tm_tsince_pre))
		{
		  tm_tsince -=1.;//сначало поминутно ищем
		  if (!getPosition(tm_tsince,&geo)) {return false;}
		  dbu_p++;
		}
            }

	  //while((geo.lat>porog_t)&&(dbu_p<1000)&&(tm_tsince<tsinceE))
	  while((geo.lat>porog_t)&&(dbu_p<1000))
	    {
	      tm_tsince +=1.;//сначало поминутно ищем
	      if (!getPosition(tm_tsince,&geo)) {return false;}
	      dbu_p++;
	    }

	  tm_tsince -=1.;
	  if (!getPosition(tm_tsince,&geo)) {return false;}

	  //while((geo.lat>porog_t)&&(dbu_p<1000)&&(tm_tsince<tsinceE))
	  while((geo.lat>porog_t)&&(dbu_p<1000))
	    {
	      if (!getPosition(tm_tsince,&geo)) {return false;}
	      tm_tsince +=1./60.;//gjctreylyj
	      //fprintf(stderr,"12   %f    %f    %f \n",rad2deg(geo.lat),rad2deg(geo.lon),tm_tsince);
	      dbu_p++;
	    }

	  tm_tsince -=1./60.;
	  delta_sec=60.*(tm_tsince-tsince);
	  ret_time_end=beg.addSecs((int)delta_sec);//нашли конец
	  tm_tsinceE=tm_tsince;

	  dbu_p=0;

	  tm_tsince=tm_tsince-1.;
	  if (!getPosition(tm_tsince,&geo)) {return false;}
	  //while((geo.lat>porog_t)&&(dbu_p<1000)&&(tm_tsince>tm_tsince_pre))
	  while((geo.lat>porog_t)&&(dbu_p<1000))
	    {
	      tm_tsince -=1.;//сначало поминутно ищем
	      if (!getPosition(tm_tsince,&geo)) {return false;}
	      dbu_p++;
	    }

	  tm_tsince +=1.;
	  if (!getPosition(tm_tsince,&geo)) {return false;}
	  while((geo.lat>porog_t)&&(dbu_p<1000)&&(tm_tsince<tsinceE))
	    {
	      if (!getPosition(tm_tsince,&geo)) {return false;}
	      tm_tsince -=1./60.;//сначало поминутно ищем
	      //fprintf(stderr,"14   %f    %f    %f \n",rad2deg(geo.lat),rad2deg(geo.lon),tm_tsince);
	      dbu_p++;
	    }
	  delta_sec2=60.*(tm_tsinceE-tm_tsince);
	  ret_time_start=ret_time_end.addSecs((int)-delta_sec2);//нашли конец
        }// if ug_
      //}

      break;
    case meteo::MERCAT:
    default:
      pok_maps=false;
      tsince = timeFromTLE(beg);//in minutes
      tsinceE = timeFromTLE(end);

      if (!getPosition(tsince, &geo1))
	{return false;}
      if (!getPosition((tsince+step/60.0), &geo2))
	{return false;}
      if (!getPosition((tsince+2.*(step/60.0)), &geo3))
	{return false;}

      direction_t=2;
      if((geo1.lon<geo2.lon)||(geo2.lon<geo3.lon))
	direction_t=0;
      if((geo1.lon>geo2.lon)||(geo2.lon>geo3.lon))
	direction_t=1;

      site_s.lat=0.;
      //site_s.lon=217.;
      site_s.lon=180.;
      switch (direction_t)
        {
        case 0:
	  tm_tsince = tsince;//in minutes
	  tm_tsinceE = tsinceE;

	  //pre_site=deg2rad(360.);
	  past_site=deg2rad(site_s.lon);

	  if (!getPosition(tm_tsince,&geo)) {return false;}
	  dbu_p=0;

	  while((geo.lon>=past_site)&&(dbu_p<1000))
	    {
	      tm_tsince +=1.;//сначало поминутно ищем
	      if (!getPosition(tm_tsince,&geo)) {return false;}
	      dbu_p++;
	    }
          //tm_tsince -=1.;
          //if (!getPosition(tm_tsince,&geo)) {return false;}
	  while((geo.lon<past_site)&&(dbu_p<1000))
	    {
	      tm_tsince +=1.;//сначало поминутно ищем
	      if (!getPosition(tm_tsince,&geo)) {return false;}
	      dbu_p++;
	    }
	  tm_tsince -=1.;
	  if (!getPosition(tm_tsince,&geo)) {return false;}
	  while((geo.lon<past_site)&&(dbu_p<1000))
	    {
	      if (!getPosition(tm_tsince,&geo)) {return false;}
	      tm_tsince +=1./60.;//сначало поминутно ищем
	      dbu_p++;
	    }
	  //tm_tsince -=1./60.;
	  delta_sec=60.*(tm_tsince-tsince);
	  ret_time_end=beg.addSecs((int)delta_sec);
	  dbu_p=0;
	  tm_tsince = tsince;//in minutes
	  tm_tsinceE = tsinceE;

	  //        pre_site=deg2rad(360.);
	  past_site=deg2rad(site_s.lon);
	  if (!getPosition(tm_tsince,&geo)) {return false;}
	  while((geo.lon<=past_site)&&(dbu_p<1000))
	    {
	      tm_tsince -=1.;//сначало поминутно ищем
	      if (!getPosition(tm_tsince,&geo)) {return false;}
	      dbu_p++;
	    }
	  //tm_tsince +=1.;
	  if (!getPosition(tm_tsince,&geo)) {return false;}

	  while((geo.lon>past_site)&&(dbu_p<1000))
	    {
	      tm_tsince -=1.;//сначало поминутно ищем
	      if (!getPosition(tm_tsince,&geo)) {return false;}
	      dbu_p++;
	    }

	  tm_tsince +=1.;
	  if (!getPosition(tm_tsince,&geo)) {return false;}

	  while((geo.lon>past_site)&&(dbu_p<1000))
	    {
	      if (!getPosition(tm_tsince,&geo)) {return false;}
	      tm_tsince -=1./60.;//сначало поминутно ищем
	      dbu_p++;
	    }
	  tm_tsince +=1./60.;
	  delta_sec2=60.*(tsince-tm_tsince);

	  ret_time_start=beg.addSecs((int)-delta_sec2);
	  break;

        case 1:
	  tm_tsince = tsince;//in minutes
	  tm_tsinceE = tsinceE;
	  //        pre_site=deg2rad(360.);
	  past_site=deg2rad(site_s.lon);

	  if (!getPosition(tm_tsince,&geo)) {return false;}
	  dbu_p=0;

	  while((geo.lon<=past_site)&&(dbu_p<1000))
	    {
	      tm_tsince +=1.;//сначало поминутно ищем
	      if (!getPosition(tm_tsince,&geo)) {return false;}
	      dbu_p++;
	    }
          //tm_tsince -=1.;
          //if (!getPosition(tm_tsince,&geo)) {return false;}
	  while((geo.lon>past_site)&&(dbu_p<1000))
	    {
	      tm_tsince +=1.;//сначало поминутно ищем
	      if (!getPosition(tm_tsince,&geo)) {return false;}
	      dbu_p++;
	    }
	  tm_tsince -=1.;
	  if (!getPosition(tm_tsince,&geo)) {return false;}
	  while((geo.lon>past_site)&&(dbu_p<1000))
	    {
	      if (!getPosition(tm_tsince,&geo)) {return false;}
	      tm_tsince +=1./60.;//сначало поминутно ищем
	      dbu_p++;
	    }
	  //tm_tsince -=1./60.;
	  delta_sec=60.*(tm_tsince-tsince);
	  ret_time_end=beg.addSecs((int)delta_sec);


	  dbu_p=0;
	  tm_tsince = tsince;//in minutes
	  tm_tsinceE = tsinceE;
	  //        pre_site=deg2rad(360.);
	  past_site=deg2rad(site_s.lon);
	  if (!getPosition(tm_tsince,&geo)) {return false;}

	  while((geo.lon>=past_site)&&(dbu_p<1000))
	    {
	      tm_tsince -=1.;//сначало поминутно ищем
	      if (!getPosition(tm_tsince,&geo)) {return false;}
	      dbu_p++;
	    }
	  //tm_tsince +=1.;
	  if (!getPosition(tm_tsince,&geo)) {return false;}

	  while((geo.lon<past_site)&&(dbu_p<1000))
	    {
	      tm_tsince -=1.;//сначало поминутно ищем
	      if (!getPosition(tm_tsince,&geo)) {return false;}
	      dbu_p++;
	    }

	  tm_tsince +=1.;
	  if (!getPosition(tm_tsince,&geo)) {return false;}

	  while((geo.lon<past_site)&&(dbu_p<1000))
	    {
	      if (!getPosition(tm_tsince,&geo)) {return false;}
	      tm_tsince -=1./60.;//сначало поминутно ищем
	      dbu_p++;
	    }
	  tm_tsince +=1./60.;
	  delta_sec2=60.*(tsince-tm_tsince);

	  ret_time_start=beg.addSecs((int)-delta_sec2);
	  break;
        }
      break;
    }//end  swith
  return true;
}

