#include "satellitebase.h"
#include "noradsgp4.h"
#include "noradsdp4.h"

#include <commons/geobasis/coords.h>
#include <cross-commons/debug/tlog.h>

#include <qfile.h>

#include <commons/mathtools/mnmath.h>

#include <QDebug>

#define TLE_LINE_BYTES   (69)

#define TLE_EPOCH_YEAR_COL (18)
#define TLE_EPOCH_YEAR_LEN  (2)
#define TLE_EPOCH_COL      (20)
#define TLE_EPOCH_LEN      (12)
#define TLE_BSTAR_COL      (53)
#define TLE_BSTAR_LEN       (6)
#define TLE_BSTAR_EXP_COL  (59)
#define TLE_BSTAR_EXP_LEN   (2)
#define TLE_INCLIN_COL      (8)
#define TLE_INCLIN_LEN      (8)
#define TLE_RAAN_COL       (17)
#define TLE_RAAN_LEN        (8)
#define TLE_ECCENTR_COL    (26)
#define TLE_ECCENTR_LEN     (7)
#define TLE_PERIGEE_COL    (34)
#define TLE_PERIGEE_LEN     (8)
#define TLE_ANOMALY_COL    (43)
#define TLE_ANOMALY_LEN     (8)
#define TLE_MOTION_COL     (52)
#define TLE_MOTION_LEN     (11)
#define TLE_REVOL_COL      (63)
#define TLE_REVOL_LEN       (5)

using namespace MnMath;

QDataStream& operator<<(QDataStream& out, const MnSat::TLEParams& tp)
{
  out << tp.epochYear;
  out << tp.epochDay;
  out << tp.bstar;
  out << tp.inclination;
  out << tp.raan;
  out << tp.eccentr;
  out << tp.perigee;
  out << tp.meanAnomaly;
  out << tp.meanMotion;
  out << (quint64)tp.revolNumber;

  return out;
}

QDataStream& operator>>(QDataStream& in, MnSat::TLEParams& tp)
{
  in >> tp.epochYear;  
  in >> tp.epochDay;   
  in >> tp.bstar;      
  in >> tp.inclination;
  in >> tp.raan;       
  in >> tp.eccentr;    
  in >> tp.perigee;    
  in >> tp.meanAnomaly;
  in >> tp.meanMotion; 
  quint64 rn;
  in >> rn;
  tp.revolNumber = rn;

  return in;  
}



SatelliteBase::SatelliteBase():_noradModel(0)
{
  memset(&_tle, 0, sizeof(_tle));
}

SatelliteBase::~SatelliteBase()
{
  delete _noradModel;
  _noradModel = 0;
}


//!чтение необходимых параметров из TLE и инициализация переменных
bool SatelliteBase::readTLE( const QString& satName, const QString& fileNameTLE )
{
  bool ok = true;
  MnSat::STLEParams stle;
  ok &= getSTLE(satName, fileNameTLE, &stle);
  ok &= readTLE(stle);
  return ok;
}

bool SatelliteBase::readTLE(const MnSat::STLEParams& stle)
{
  bool ok = parseTLE(stle);

  if ( ok ) {
    initVariables();
  }

  return ok;
}

/*!
 * \brief Чтение необходимых параметров из TLE
 * \param satName название спутника
 * \param fileNameTLE полное имя файла TLE
 * \return true в случае успеха, иначе false
 */
bool SatelliteBase::parseTLE( const QString& satName, const QString& fileNameTLE)
{
  MnSat::STLEParams stle;
  bool ok = true;
  ok &= getSTLE(satName, fileNameTLE, &stle);
  if(ok) {
    ok &= parseTLE(stle);
  }
  return ok;
}

bool SatelliteBase::parseTLE(const MnSat::STLEParams& stle)
{
  bool ok = true;
  ok &= false == stle.firstString.isEmpty() && isValidLineTLE(stle.firstString);
  ok &= false == stle.secondString.isEmpty() && isValidLineTLE(stle.secondString);
  if(ok) {
    _name = stle.satName;

    _tle.epochYear  =  stle.firstString.mid(TLE_EPOCH_YEAR_COL, TLE_EPOCH_YEAR_LEN).toUInt();
    _tle.epochDay   =  stle.firstString.mid(TLE_EPOCH_COL,      TLE_EPOCH_LEN     ).toDouble();
    _tle.bstar      = (stle.firstString.mid(TLE_BSTAR_COL,      1) + "." +
                       stle.firstString.mid(TLE_BSTAR_COL + 1,  TLE_BSTAR_LEN - 1) + "E" +
                       stle.firstString.mid(TLE_BSTAR_EXP_COL,  TLE_BSTAR_EXP_LEN)).toDouble();
//    _tle.bstar      = (stle.firstString.mid(TLE_BSTAR_COL,      TLE_BSTAR_LEN ) + "E" +
//                       stle.firstString.mid(TLE_BSTAR_EXP_COL,  TLE_BSTAR_EXP_LEN)).toDouble();
    _tle.epochYear += (_tle.epochYear < 57 ? 2000 : 1900);

    _tle.inclination = deg2rad(stle.secondString.mid(TLE_INCLIN_COL,  TLE_INCLIN_LEN  ).toDouble());
    _tle.raan        = deg2rad(stle.secondString.mid(TLE_RAAN_COL,    TLE_RAAN_LEN    ).toDouble());
    _tle.eccentr     = ("0." + stle.secondString.mid(TLE_ECCENTR_COL, TLE_ECCENTR_LEN)).toDouble();
    _tle.perigee     = deg2rad(stle.secondString.mid(TLE_PERIGEE_COL, TLE_PERIGEE_LEN ).toDouble());
    _tle.meanAnomaly = deg2rad(stle.secondString.mid(TLE_ANOMALY_COL, TLE_ANOMALY_LEN ).toDouble());
    _meanMotion      =         stle.secondString.mid(TLE_MOTION_COL,  TLE_MOTION_LEN  ).toDouble();
    _tle.revolNumber =         stle.secondString.mid(TLE_REVOL_COL,   TLE_REVOL_LEN   ).toUInt();
    _tle.meanMotion  = _meanMotion * M_2PI / MIN_PER_DAY;
  }
  return ok;
}

/*!
 * \brief Получение TLE для заданного спутника из телеграммы
 * \param satName      Название спутника
 * \param fileNameTLE  Файл - телеграмма
 * \param stle         Возвращаемые TLE данные
 * \return true в случае успеха, иначе false
 */
bool SatelliteBase::getSTLE( const QString& satName, const QString& fileNameTLE, MnSat::STLEParams* stle ) {
  QFile file(fileNameTLE);
  bool ok = file.open(QIODevice::ReadOnly);
  if(ok)
  {
    QString lineTLE;
    QTextStream fileTLE(&file);
    while (!fileTLE.atEnd())
    {
      lineTLE = fileTLE.readLine();
      if(satName != lineTLE.trimmed()) {
        continue;
      }
      stle->satName = lineTLE.trimmed();

      lineTLE = fileTLE.readLine();
      ok &= false == lineTLE.isEmpty() && isValidLineTLE(lineTLE);
      if(ok) {
        stle->firstString = lineTLE;
        lineTLE = fileTLE.readLine();
        ok &= false == lineTLE.isEmpty() && isValidLineTLE(lineTLE);
        if(ok) {
          stle->secondString = lineTLE;
        }
      }
      else {
        break;
      }
    }
  }
  file.close();
  //printf("%s\n%s\n", stle->firstString.toStdString().data(),  stle->secondString.toStdString().data());
  return ok;
}


//! проверка контрольной суммы строки TLE
bool SatelliteBase::isValidLineTLE( const QString& line )
{
  int cs = 0;
  for ( uint i = 0; i < uint(line.length()) - 1; i++ ) {
    if ( line[i].isDigit() ) {
      cs += line[i].digitValue();
    } else if ( line[i] == '-' ) {
      cs++;
    }
  }
  cs %= 10;//последняя цифра получившейся суммы

  return  (cs == line[ line.length() - 1 ].digitValue() );
}

/*!
 * \brief Установка параметров TLE
 * \param tle параметры TLE телеграммы
 */
void SatelliteBase::setTLEParams(const MnSat::TLEParams& tle)
{
  _tle = tle;
  _meanMotion = _tle.meanMotion * MIN_PER_DAY / M_2PI;

  initVariables();
  // _meanMotion = lineTLE.mid( TLE_MOTION_COL,
  // 			     TLE_MOTION_LEN ).toDouble( &ok );
  // _tle.meanMotion  = _meanMotion * M_2PI / MIN_PER_DAY;
}


//! выбор SGP4/SDP4 модели; заполнение переменных, расчитанных по TLE
void SatelliteBase::initVariables()
{
  _jdEpoch = JullianDate( _tle.epochYear, _tle.epochDay );

  /* Period > 225 minutes is deep space => SDP4*/
  double a1 = pow( XKE / _tle.meanMotion, TWO_THRD );
  double temp = (1.5 * CK2 * (3.0 * sqr( cos(_tle.inclination) ) - 1.0) /
		 pow(1.0 - sqr(_tle.eccentr), 1.5) );
  double delta1 = temp / sqr(a1);
  double a0     = a1 *
    (1.0 - delta1 *
     ((1.0 / 3.0) + delta1 *
      (1.0 + 134.0 / 81.0 * delta1)));
  double delta0 = temp / sqr(a0);

  _meanMotionRec = _tle.meanMotion / (1.0 + delta0);
  _aeAxisSemiMinorRec = a0 / (1.0 - delta0);
  _aeAxisSemiMajorRec = _aeAxisSemiMinorRec / sqrt( 1.0 - sqr(_tle.eccentr) );
  _kmPerigeeRec       = EQUATOR_RAD * (_aeAxisSemiMajorRec * (1.0 - _tle.eccentr) - AE);
  _kmApogeeRec        = EQUATOR_RAD * (_aeAxisSemiMajorRec * (1.0 + _tle.eccentr) - AE);

  delete _noradModel;
  _noradModel = 0;

  if ( M_2PI / _meanMotionRec >= 225.0) {
    // SDP4 - period >= 225 minutes.
    //printf("SDP\n");
    _noradModel = new(std::nothrow) NoradSDP4( *this );
  } else {
    // SGP4 - period < 225 minutes
    //printf("SGP\n");
    _noradModel = new(std::nothrow) NoradSGP4( *this );
  }
}


//!Номер витка спутника для даты dt
unsigned long SatelliteBase::revolNumber( const QDateTime& dt ) const
{
  JullianDate jul_d(dt);
  double since =jul_d.getDate() - _jdEpoch.getDate();
  // printf("since=%f %s  %f\n", since, dt.toString(Qt::ISODate).latin1(), rad2deg(_tle.meanAnomaly));
//  printf("%f %f %lu %f %f %f\n",
//	 (_meanMotion + since * _tle.bstar) * since + _tle.meanAnomaly/M_2PI,
//  	 floor((_meanMotion + since * _tle.bstar) * since + _tle.meanAnomaly/M_2PI),
//  	 _tle.revolNumber, _tle.meanAnomaly*180/3.14,
//	 floor((_meanMotion + since * _tle.bstar) * since) + _tle.revolNumber,
//	 floor((_meanMotion + since * _tle.bstar) * since + _tle.meanAnomaly/M_2PI) + _tle.revolNumber);

  return  (unsigned long ) floor((_meanMotion + since * _tle.bstar) * since
				 +_tle.meanAnomaly/M_2PI) + _tle.revolNumber;
}

//!Номер витка спутника для даты dt
unsigned long SatelliteBase::revolNumber(uint year, double day ) const
{
  JullianDate jul_d(year, day);
  double since =jul_d.getDate() - _jdEpoch.getDate();
  //   printf("since=%f motion=%f\n", since, _meanMotion);
  //   printf("***%lu\n", _tle.revolNumber + (unsigned)(_meanMotion*since) + 1);
  // printf("since=%f\n", since);
  // printf("%f %f %f %f\n", (_meanMotion + since * _tle.bstar) * since, (_meanMotion + since * _tle.bstar) * since +
  // 	 _tle.meanAnomaly/M_2PI, floor((_meanMotion + since * _tle.bstar) * since +
  // 				       _tle.meanAnomaly/M_2PI),
  // 	 floor((_meanMotion + since * _tle.bstar) * since +
  // 	       _tle.meanAnomaly/M_2PI) + _tle.revolNumber);
  return  (unsigned long ) floor((_meanMotion + since * _tle.bstar) * since
				 + _tle.meanAnomaly/M_2PI) + _tle.revolNumber;
}

/*!
 * \brief вычисление положения спутника в геоцентрических координатах
 * \param tsince время в минутах с TLE эпохи (GMT).
 * \param geo возвращаемые геогр.координаты
 * \return true в случае успеха, иначе false
 */
bool SatelliteBase::getPosition(double tsince, EciPoint *eci ) const
{
  bool ok = false;

  if ( _noradModel ) {
    ok = _noradModel->getPosition(tsince, *eci );
  }
  return ok;
}

/*!
 * \brief вычисление положения спутника в геоцентрических координатах
 * \param dt интересующий момент времени в UTC (или в GMT ?).
 * \param eci возвращаемые геоцентрические координаты
 * \return true в случае успеха, иначе false
 */
bool SatelliteBase::getPosition( const QDateTime& dt, EciPoint *eci) const
{
  JullianDate jul_d(dt);
  // debug_log << "tsince, min" << (jul_d.getDate()-epoch().getDate())*MIN_PER_DAY <<
  //   (jul_d.getDate()-epoch().getDate());

  double tsince = (jul_d.getDate()- epoch().getDate())*MIN_PER_DAY;

  return getPosition( tsince, eci );
}

/*!
 * \brief вычисление положения спутника в географических координатах
 * \param tsince время в минутах с TLE эпохи
 * \param geo возвращаемые геогр.координаты
 * \return true в случае успеха, иначе false
 */
bool SatelliteBase::getPosition(double tsince, GeoCoord *geo) const
{
  EciPoint eci;
  if ( ! getPosition(tsince, &eci)) {
    debug_log<<"error";
    return false;
  }

  *geo = eci.posToGeo();
  return true;
}

/*!
 * \brief вычисление положения спутника в географических координатах
 * \param dt интересующий момент времени в UTC
 * \param geo возвращаемые геогр.координаты
 * \return true в случае успеха, иначе false
 */
bool SatelliteBase::getPosition( const QDateTime& dt, Coords::GeoCoord* geo ) const {
  JullianDate jul_d(dt);
  double tsince = (jul_d.getDate()- epoch().getDate())*MIN_PER_DAY;
  EciPoint eci;
  if ( ! getPosition(tsince, &eci )) {
    return false;
  }

  *geo = eci.posToGeo();
  return true;
}

/*!
 * \brief Вычисление положения спутника в топографических координатах
 * \param tsince Время в минутах с TLE эпохи
 * \param site Местоположение на земле, относительно которого вычисляется положение спутника
 * \param topo Возвращаемые топографические координаты
 */
bool SatelliteBase::getPosition(double tsince, const GeoCoord& site, TopoCoord* topo) const
{
  EciPoint eci;
  if ( ! getPosition(tsince, &eci)) {
    return false;
  }

  *topo = eciToTopo( site, eci );
  return true;
}

/*!
 * \brief Вычисление положения спутника в топографических координатах
 * \param dt интересующий момент времени в UTC
 * \param site Местоположение на земле, относительно которого вычисляется положение спутника
 * \param topo Возвращаемые топографические координаты
 */
bool SatelliteBase::getPosition( const QDateTime& dt, const GeoCoord& site, TopoCoord* topo) const
{
  JullianDate jul_d(dt);
  double tsince = (jul_d.getDate()- epoch().getDate())*MIN_PER_DAY;
  EciPoint eci;
  if ( ! getPosition(tsince, &eci)) {
    return false;
  }

  *topo = eciToTopo( site, eci );
  return true;
}

/*!
 * \brief Вычисление положения спутника в топографических координатах
 * \param tsince Время в минутах с TLE эпохи
 * \param site Местоположение на земле, относительно которого вычисляется положение спутника
 * \param topo Возвращаемые топографические координаты
 */
bool SatelliteBase::getPosition(double tsince, const GeoCoord& site, TopoPoint* topo) const
{
    EciPoint eci;
    if ( ! getPosition(tsince, &eci)) {
        return false;
    }
    *topo = eciToTopoPoint( site, eci );
    return true;
}

/*!
 * \brief Вычисление положения спутника в топографических координатах
 * \param dt интересующий момент времени в UTC
 * \param site Местоположение на земле, относительно которого вычисляется положение спутника
 * \param topo Возвращаемые топографические координаты
 */
bool SatelliteBase::getPosition( const QDateTime& dt, const GeoCoord& site, TopoPoint* topo) const
{
    JullianDate jul_d(dt);
    double tsince = (jul_d.getDate()- epoch().getDate())*MIN_PER_DAY;
    EciPoint eci;
    if ( ! getPosition(tsince, &eci)) {
        return false;
    }

    *topo = eciToTopoPoint( site, eci );
    return true;
}

/*!
 * \brief Вычисление положения спутника в топографических координатах
 * \param tsince Время в минутах с TLE эпохи
 * \param site Местоположение на земле, относительно которого вычисляется положение спутника
 * \param topo Возвращаемые топографические координаты
 * \param geo Возвращаемые географические координаты
 */
bool SatelliteBase::getPosition( double tsince, const GeoCoord& site, TopoCoord* topo, GeoCoord* geo ) const {
  EciPoint eci;
  if ( ! getPosition(tsince, &eci)) {
    return false;
  }

  *topo = eciToTopo( site, eci );
  *geo = eci.posToGeo();
  return true;
}

/*!
 * \brief Вычисление положения спутника в топографических координатах
 * \param tsince Время в минутах с TLE эпохи
 * \param site Местоположение на земле, относительно которого вычисляется положение спутника
 * \param topo Возвращаемые топографические координаты
 * \param geo Возвращаемые географические координаты
 */
bool SatelliteBase::getPosition( const QDateTime& dt, const GeoCoord& site, TopoCoord* topo, GeoCoord* geo ) const {
  JullianDate jul_d(dt);
  double tsince = (jul_d.getDate()- epoch().getDate())*MIN_PER_DAY;
  return getPosition( tsince, site, topo, geo );
}

/*!
 * \brief вычисляет время прошедшее с TLE эпохи
 * \param dt момент времени, для которого вычислять
 * \return  time in minutes since the TLE epoch (GMT)
 */
double SatelliteBase::timeFromTLE( const QDateTime& dt ) const
{
  JullianDate jul_d(dt);
  // JullianDate date1 = _jdEpoch;
  // printf("tsince=%f jd=%f jEpoch=%f\n",  (jul_d.getDate() - _jdEpoch.getDate())*MIN_PER_DAY,
  // 	 jul_d.getDate(), _jdEpoch.getDate());

  return (jul_d.getDate() - _jdEpoch.getDate())*MIN_PER_DAY;
}

/*!
 * \brief чтение всего списка КА из TLE (NASA Two Line Element format)
 * \param fileNameTLE имя файла TLE
 * \param nameSattelite возврат списка
 * \param type Тип спутников ( 0 - все, 1 - орбитальные, 2 - геостационарные )
 * \return  true в случае если есть КОРРЕКТНЫЕ сведения (в соответствии с NASA Two Line Element format), иначе false
 */

bool SatelliteBase::readSatelliteList( const QString& fileNameTLE, QStringList* nameSattelite, const int type )
{
  bool ok = false;
  QString lineTLE;
  QString name_ones;
  bool nal_info=false;
  int vs_ints=0;
  double vs_doubles=0.;
  QFile file( fileNameTLE );
  if ( !file.open( QIODevice::ReadOnly ) ) {
    return false;
  }

  QTextStream fileTLE( &file );
  while (!fileTLE.atEnd() ) {
    lineTLE = fileTLE.readLine();
    name_ones = lineTLE.trimmed();
    lineTLE = fileTLE.readLine();
    if (!lineTLE.isEmpty() && isValidLineTLE(lineTLE) ) {
      vs_ints  = lineTLE.mid(TLE_START_CODE1,TLE_LONG_CODE).toUInt(&ok);
      if(!ok){break;}
      if(vs_ints!=TLE_CODE_LINE1){break;}
      vs_ints=lineTLE.mid( TLE_EPOCH_YEAR_COL,  TLE_EPOCH_YEAR_LEN ).toUInt(&ok);
      if(!ok){break;}
      vs_doubles= lineTLE.mid( TLE_EPOCH_COL, TLE_EPOCH_LEN ).toDouble(&ok);
      if(!ok){break;}
      vs_doubles=(lineTLE.mid( TLE_BSTAR_COL, 1 ) + "." + lineTLE.mid( TLE_BSTAR_COL + 1, TLE_BSTAR_LEN - 1 ) + "E" +lineTLE.mid( TLE_BSTAR_EXP_COL, TLE_BSTAR_EXP_LEN )).toDouble(&ok);
      if(!ok){break;}
      lineTLE = fileTLE.readLine();
      if ( ! lineTLE.isEmpty() && isValidLineTLE(lineTLE) ) {
        vs_ints= lineTLE.mid(TLE_START_CODE2,TLE_LONG_CODE).toUInt(&ok);
        if(!ok){break;}
        if(vs_ints!=TLE_CODE_LINE2){break;}
        vs_doubles = deg2rad( lineTLE.mid( TLE_INCLIN_COL,TLE_INCLIN_LEN ).toDouble(&ok) );
        if(!ok){break;}
        vs_doubles = deg2rad( lineTLE.mid( TLE_RAAN_COL,TLE_RAAN_LEN ).toDouble(&ok) );
        if(!ok){break;}
        vs_doubles     = ("0." + lineTLE.mid( TLE_ECCENTR_COL,TLE_ECCENTR_LEN )).toDouble(&ok);
        if(!ok){break;}
        vs_doubles     = deg2rad( lineTLE.mid( TLE_PERIGEE_COL,TLE_PERIGEE_LEN ).toDouble(&ok) );
        if(!ok){break;}
        vs_doubles = deg2rad( lineTLE.mid( TLE_ANOMALY_COL,TLE_ANOMALY_LEN ).toDouble(&ok) );
        if(!ok){break;}
        vs_doubles = lineTLE.mid( TLE_MOTION_COL,TLE_MOTION_LEN ).toDouble( &ok );
        if(!ok){break;}
        else if ( type == 1 && vs_doubles < 2.0 ) {
          continue;
        }
        else if ( type == 2 && vs_doubles > 2.0 ) {
          continue;
        }
        vs_ints = lineTLE.mid( TLE_REVOL_COL,TLE_REVOL_LEN ).toUInt( &ok );
        if(!ok){break;}
      }
    }
    else
      break;
    nameSattelite->append(name_ones);
    nal_info=true;
  }
  file.close();
  return nal_info;
}

/*!
 * \brief чтение геоцентрических координат для всего списка КА из TLE (NASA Two Line Element format)
 * \param fileNameTLE имя файла TLE
 * \param beg время для которого нужны координаты
 * \param position_s возврат списка с координатами и именами
 * \return  true в случае если есть КОРРЕКТНЫЕ сведения (в соответствии с NASA Two Line Element format), иначе false
 */
bool SatelliteBase::readPozitionNow( const QString& fileNameTLE,const QDateTime& beg,QVector<Coords::EciPointName>&position_s)
{
  bool ok = false;
  bool nal_info = false;
  double tsince=0.;
  EciPoint eci;
  EciPointName pos_ones;
  QString name_satellite_ones;
  
  QStringList name_satellite;
  ok = readSatelliteList(fileNameTLE, &name_satellite);//
  if(!ok)return ok;
  
  for (QList<QString>::Iterator it = name_satellite.begin(); it != name_satellite.end(); ++it )
    {
      name_satellite_ones=(*it);
      ok = readTLE(name_satellite_ones,fileNameTLE);//
      tsince = timeFromTLE(beg);//in minutes
      if(ok) {
	if (getPosition(tsince, &eci)) {
	  pos_ones.e_point=eci;
	  pos_ones.name_ka=name_satellite_ones;
	  position_s.append(pos_ones);
	  nal_info=true;
	}
      }
    }
  return nal_info;
}

void SatelliteBase::lastParams( double* raan, double* u, double* dt, double* i ) const
{
  return _noradModel->lastParams( raan, u, dt, i );
}
