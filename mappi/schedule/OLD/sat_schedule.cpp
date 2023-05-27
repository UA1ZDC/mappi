#include <math.h>

#include <qstringlist.h>

#include <commons/mathtools/mnmath.h>
#include <sat-commons/satellite/satellite.h>


#include "sat_schedule.h"

#define SETTL_CONFL_STEP 10 //!< Шаг для расчета траектории для разрешения конфликта (точность)

//#define DEBUGSCHED

using namespace Coords;
using namespace mappi;
using namespace schedule;

/*!
 * \param file Полный путь к файлу-телеграмме, 
 *        определяющей положение спутника (напр, TLE)
 * \param site Место наблюдения на Земле
 */
SatSchedule::SatSchedule( const QString& file, const GeoCoord& site ):
  _file(file), _site(site)
{
}

SatSchedule::~SatSchedule()
{
}

/*!
 * \brief Установка файла-телеграммы
 * \param file полный путь к файлу-телеграмме
 */
void SatSchedule::setFile( const QString& file )
{
  _file = file;
}

/*!
 * \brief Установка места наблюдения
 * \param site Географические координаты 
 */
void SatSchedule::setSite( const GeoCoord& site)
{
  _site = site;
}


/*!
 * \brief Составить расписание
 * \param dt         Момент времени, с которого расчитывать, в UTC
 * \param satts      Спутники, для которых необходимо составить расписание
 * \param hours      Количество часов, которое необходимо рассчитать
 * \param threshold  Пороговое значения для угла места (минимальный угол), в радианах
 * \param culmin     Пороговое значение угола кульминации(минимальный угол для включения в расписаие), радиан
 * \param sched      Составленное расписание (время в UTC)
 * \return false в случае некорректности телеграммы, иначе true
 */
bool SatSchedule::getSchedule( const QDateTime& dt_dt, const QStringList& satts,
			       float hours, float threshold, float culmin,
             QList<satellite::Schedule>& sched ) const
{
  QDateTime dt = dt_dt.addSecs(-60*20); //TODO заглушка
  bool ok = true;
  sched.clear();
  
  time_t aos=0,los=0;
  Satellite sat;


  for (int idx = 0; idx < satts.size(); idx++) {
    const QString& psat = satts.at(idx);
    if (! sat.readTLE( psat, _file ) ) {
      // return false;
      ok = false;
      continue;//TODO не найден этот спутник в TLE? пытаться продолжить с другими?
    }

    // геостационарные спутники в расписание не включаются
    // if ( psat.satType == Settings::GEOSTAT_SAT ) {
    //      continue;
    // }

    //  printf("revol=%lu\n", sat.revolNumber(2010, 113.33422024));
    
    time_t startTime = (time_t)(sat.timeFromTLE( dt ) * 60.0); //в секундах
    time_t t = startTime;
    aos=los=0;

    while (t < startTime + hours * 60*60) {
      if ( !sat.search_AOS_LOS( t, _site, &aos, &los, threshold) ) {
	//невозможно получить координаты для этого спутника
	ok = false;
	break;
      }
      if ( aos > los ) { //спутник уже в зоне
	aos = t;
      }
      double maxEl = 0;
      time_t mos;
      if (!sat.searchMaximum(aos, _site, &maxEl, &mos) ) {
	//невозможно получить координаты для этого спутника
	ok = false;
	break;
      }
      
      if (maxEl >= culmin) {
	//int dir = direction(sat, los);

	/*	time_t nlat;
	sat.searchNearLatTime(aos, los, _site, &nlat);
	time_t nlon;
	sat.searchNearLonTime(aos, los, _site, &nlon);
	printf("\n***\n");
	ulong raos = sat.revolNumber(dt.addSecs(aos - startTime));
	ulong rlos = sat.revolNumber(dt.addSecs(los - startTime));
	ulong rlat = sat.revolNumber(dt.addSecs(nlat - startTime));
	ulong rlon = sat.revolNumber(dt.addSecs(nlon - startTime));
	printf("aos=%s los=%s nlat=%s  nlon=%s r=%lu r=%lu r=%lu r=%lu\n",
	       dt.addSecs(aos - startTime).addSecs(4*60*60).toString().latin1(), 
	       dt.addSecs(los - startTime).addSecs(4*60*60).toString().latin1(),
	       dt.addSecs(nlat - startTime).addSecs(4*60*60).toString().latin1(),
	       dt.addSecs(nlon - startTime).addSecs(4*60*60).toString().latin1(),
	       raos, rlos, rlat, rlon      
	       );
	       printf("***\n");*/

	conf::SatDirection dir = static_cast<conf::SatDirection>(sat.direction(dt.addSecs(aos - startTime),dt.addSecs(los - startTime)));
        //NOTE номер витка определяется не в aos, т.к. захватываем во время приёма момент смены витка => может получиться, что у двух сеансов один номер витка (его самое начало и его конец)
	satellite::Schedule satSh(psat, 
				  dt.addSecs(aos - startTime), 
				  dt.addSecs(los - startTime), maxEl,
				  sat.revolNumber(dt.addSecs(mos - startTime)),
				  dir,
				  0//psat.freq()
				  );
	sched.append(satSh);
      }
      t =los + 300;//(time_t) (sat.timeFromTLE( dt.addSecs(int(los - startTime) )) * 60.0)+300;
    }
  }

#ifdef DEBUGSCHED

  sched.clear();
  QDateTime ddt = QDateTime::currentDateTime(Qt::UTC);
  for ( int i = 0; i < 10; ++i ) {
    schedule::Schedule satSh("NOAA 18", 
			     ddt.addSecs( i*60*5 + 5*60  ), 
			     ddt.addSecs( i*60*5 + 5*60 +2*60  ), 30,
			     1000+i,
			     0,
			     0.0);
    sched.append(satSh);
  }

#endif

  return ok;
}


/*!
 * \brief Составить расписание
 * \param dt         Момент времени, с которого расчитывать, в UTC
 * \param satts      Спутники, для которых необходимо составить расписание
 * \param hours      Количество часов, которое необходимо рассчитать
 * \param threshold  Пороговое значения для угла места (минимальный угол), в радианах
 * \param culmin     Пороговое значение угола кульминации(минимальный угол для включения в расписаие), радиан
 * \param sched      Составленное расписание (время в UTC)
 * \return false в случае некорректности телеграммы, иначе true
 */
bool SatSchedule::getSchedule( const QDateTime& dt_dt, const QString& tleName,
			       float hours, float threshold, float culmin,
             QList<satellite::Schedule>& sched ) const
{
  QDateTime dt = dt_dt;//.addSecs(-60*20); //TODO заглушка
  bool ok = true;
  sched.clear();
  
  time_t aos=0,los=0;
  Satellite sat;

  if (! sat.readTLE( tleName, _file ) ) {
    // return false;
    ok = false;
    return ok;
  }

  //  printf("revol=%lu\n", sat.revolNumber(2010, 113.33422024));
    
  time_t startTime = (time_t)(sat.timeFromTLE( dt ) * 60.0); //в секундах
  time_t t = startTime;
  aos=los=0;
  
  while (t < time_t(startTime + hours * 60*60)) {
    if ( !sat.search_AOS_LOS( t, _site, &aos, &los, threshold) ) {
      //невозможно получить координаты для этого спутника
      ok = false;
      break;
      }
      if ( aos > los ) { //спутник уже в зоне
	aos = t;
      }
      double maxEl = 0;
      time_t mos;
      if (!sat.searchMaximum(aos, _site, &maxEl, &mos) ) {
	//невозможно получить координаты для этого спутника
	ok = false;
	break;
      }
      
      if (maxEl >= culmin) {
	//int dir = direction(sat, los);

	/*	time_t nlat;
	sat.searchNearLatTime(aos, los, _site, &nlat);
	time_t nlon;
	sat.searchNearLonTime(aos, los, _site, &nlon);
	printf("\n***\n");
	ulong raos = sat.revolNumber(dt.addSecs(aos - startTime));
	ulong rlos = sat.revolNumber(dt.addSecs(los - startTime));
	ulong rlat = sat.revolNumber(dt.addSecs(nlat - startTime));
	ulong rlon = sat.revolNumber(dt.addSecs(nlon - startTime));
	printf("aos=%s los=%s nlat=%s  nlon=%s r=%lu r=%lu r=%lu r=%lu\n",
	       dt.addSecs(aos - startTime).addSecs(4*60*60).toString().latin1(), 
	       dt.addSecs(los - startTime).addSecs(4*60*60).toString().latin1(),
	       dt.addSecs(nlat - startTime).addSecs(4*60*60).toString().latin1(),
	       dt.addSecs(nlon - startTime).addSecs(4*60*60).toString().latin1(),
	       raos, rlos, rlat, rlon      
	       );
	       printf("***\n");*/
	conf::SatDirection dir = static_cast<conf::SatDirection>( sat.direction(dt.addSecs(aos - startTime),dt.addSecs(los - startTime)));
	if (dt.addSecs(aos - startTime) > dt_dt && aos < time_t(startTime + hours * 60*60)) {
	  //NOTE номер витка определяется не в aos, т.к. захватываем во время приёма момент смены витка => может получиться, что у двух сеансов один номер витка (его самое начало и его конец)
	  satellite::Schedule satSh(tleName, 
				    dt.addSecs(aos - startTime), 
				    dt.addSecs(los - startTime), maxEl,
				    sat.revolNumber(dt.addSecs(mos - startTime)),
            dir,
            0);
	  sched.append(satSh);
	}
      }
      t =los + 300;//(time_t) (sat.timeFromTLE( dt.addSecs(int(los - startTime) )) * 60.0)+300;
    }
 


  return ok;
}

/*! 
 * \brief Определение состояния конфликта и его разрешение
 * \param sched Составленное расписание (время в UTC), список отсортированный по времени приёма. Состояние конфликта у всех д.б. UNKNOWN_STATE, если состояние будет CONFL_STATE, то оно таким и останется.
 * \param type Спсоб разрешения конфликта
 */
void SatSchedule::settlementConflicts(QList<satellite::Schedule>& sched, mappi::conf::PriorType type, bool saveUserConflict)
{
  for(auto it = sched.begin(); it != sched.end(); ++it)
  {
    if((saveUserConflict && it->state == conf::kUserEnableState) ||
       (saveUserConflict && it->state == conf::kUserDisableState) ||
       it->state == conf::kConflState)
    {
      continue;
    }
    auto nextIt = it + 1;
    bool conf = false;
    for(; nextIt != sched.end(); ++nextIt)
    {
      if(nextIt->state == conf::kConflState) {
        continue;
      }
      if(it->los > nextIt->los) {
        conf = true;
        if(cmpConflict(*it, *nextIt, type) >= 0) {
          it->state     = conf::kSettlState;
          nextIt->state = conf::kConflState;
        }
        else {
          it->state     = conf::kConflState;
          nextIt->state = conf::kSettlState;
        }
        break;
      }
    }
   if(!conf && it->state == conf::kUnkState) {
     it->state = conf::kNormalState;
   }
  }
}

/*! 
 * \brief Разрешение конфликта
 * \param sat1 Первый спутник
 * \param sat2 Второй спутник
 * \param type Спсоб разрешения конфликта
 * \return целое число,  меньшее,  равное, или больше нуля, если спутник sat1, соответственно, имеет
           меньший, равный, или больший приоритет,чем sat2
*/
int SatSchedule::cmpConflict(satellite::Schedule& sat1, satellite::Schedule& sat2, mappi::conf::PriorType type) const
{
  switch(type) {
  case conf::kUnkPrior:
    return 0;
  case conf::kBeginPrior:
    return sat1.aos.secsTo(sat2.aos);
  case conf::kContinPrior:
    return sat1.aos.secsTo(sat1.los) - sat2.aos.secsTo(sat2.los);
  case conf::kWestPrior: {
    int west1 = timeWest(sat1, SETTL_CONFL_STEP);
    int west2 = timeWest(sat2, SETTL_CONFL_STEP);
    return west1 - west2;
  }
  case conf::kEastPrior: {
    int west1 = timeWest(sat1, SETTL_CONFL_STEP);
    int west2 = timeWest(sat2, SETTL_CONFL_STEP);
    return west2 - west1;
  }
  }
  return 0;
}

/*! 
 * \brief Опеделение длительности нахождения спутника западнее от места наблюдения
 * \param sat Спутник
 * \param step Шаг по времени, секунд (определяет точность оределения длительности)
 * \return Количество секунд нахождения в западном регионе
 */
int SatSchedule::timeWest(satellite::Schedule& satSh, int step) const
{
  Satellite sat;
  if (! sat.readTLE( satSh.name, _file ) ) {
    return 0;
  }

  QVector<GeoCoord> track;
  sat.trajectory(satSh.aos, satSh.los, step, track);
  int numpoints = 0; //количество точек траектории западнее места наблюдения

  for (QVector<GeoCoord>::iterator it = track.begin(); it != track.end(); ++it) {
    float lon = (fabs(_site.lon - (*it).lon ) <= M_PI) ? (*it).lon : (*it).lon - MnMath::M_2PI;
    if (_site.lon > lon) {
      ++numpoints;
    }
    //TODO если больше не западнее, то можно дальше не проверять.. обатно он вряд ли вернётся
  }
  
  return numpoints * step;
}


/*! 
 * \brief определение направления движения спутника (восходящее, нисходящее)
 * \param sat Спутник
 * \param aos Момент времени, когда спутник повяляется в зоне радиовидимости
 * \return Направление (-1 - неизвестно,0 - C->Ю, 1 - Ю->С)
 */
int SatSchedule::direction(const Satellite& sat, double aos) const
{
  int dir = -1;
  EciPoint ecip;
  if (!sat.getPosition(aos/60, &ecip) ) {
    return dir;
  }
  if (ecip.vel.z > 0) {
    dir = 1;
  } else if (ecip.vel.z < 0) {
    dir = 0;
  }
      
//   GeoCoord geo1, geo2;
//   if (!sat.getPosition(aos/60, &geo1) || 
//       !sat.getPosition((aos/60)+2, &geo2)) {
//     return dir;
//   }

//   if (geo1.lat < geo2.lat) {
//     return dir = 1;
//   } else if  (geo1.lat > geo2.lat){
//     return dir = 0;
//   }
  
  return dir;
}
