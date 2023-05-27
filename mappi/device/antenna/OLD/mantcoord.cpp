/**
    Класс предназначен для преобразования координат полученных из траектории спутника
    в координаты допустимые для анетнны. Корректировки подвергается только азимут.
    По умолчанию мера угла радианая.

    @author	Бурыкин Е.А.
    @date	26.02.09
*/
#include "mantcoord.h"
#include "prbl-ant.h"

#include <cross-commons/debug/tlog.h>
#include <commons/mathtools/mnmath.h>

#include <math.h>

// вывод отладочной информации
//#define _MDEBUG_
#ifdef _MDEBUG_
#include <stdio.h>
#endif

#define RAD_360 6.28318530718
// #define MIN_QUARTER_IV 250.00001
//#define MIN_QUARTER_IV 4.363323129985823942
//#define MAX_QUARTER_IV 359.99999
#define MAX_QUARTER_IV 6.283185132646661277
#define RAD_359_99 6.28301077425
#define RAD_180 3.14159265359
#define RAD_179_99 3.14141812066
#define RAD_90 1.570796326794896619
#define ULTIMATE_AZIMUT 1.57079632679
//#define JUMP_ZERO 260
#define JUMP_ZERO 4.5378560551852569
//#define COUP_VALUE 88.9
#define COUP_VALUE 1.551597705022958994
using namespace MnDevice;

#define RAD_359_99 6.28301077425
#define RAD_270 4.71238898

3,1415926535

6.279825555555556


void MAntCoord::print() {
    unsigned int countPoint = m_listTrack.count();
    for ( unsigned int i = 0; i < countPoint; ++i ) {
      ::printf( "%i\t%g\t%g\n", i, MnMath::rad2deg( m_listTrack[ i ].az ), MnMath::rad2deg( m_listTrack[ i ].el ) );
    };
}

// координаты антенны
MAntCoord::MAntCoord(mappi::receive::PrblAnt* ant):
  m_ant(ant)
{
    m_angleType = RADIAN;
    m_indexMaxEl = 0;
    m_maxEl = 0;
    m_azFullRate = MnMath::deg2rad( m_ant->dsaMax() );
}

MAntCoord::~MAntCoord() {
}

/** Установить меру в которой будет возвращаться угол. */
void MAntCoord::setAngleType( const angle_t& type ) {
    m_angleType = type;
}

/** Возвращает количество точек траектории спутника. */
unsigned int MAntCoord::count() const {
    return m_listTrack.count();
}

/** Доступ к точке с индексом index. */
MAntPoint& MAntCoord::operator[]( unsigned int index ) {
    static MAntPoint point;
    if ( m_angleType == DEGREE ) {
	point.setPoint( MnMath::rad2deg( m_listTrack[ index ].az ),
	    MnMath::rad2deg( m_listTrack[ index ].el ) );
    } else {
	point.setPoint( m_listTrack[ index ].az, m_listTrack[ index ].el );
    };

#ifdef _MDEBUG_
    printf( "[ MAntCoord::operator[](1) ] - azimut[%g,%g] elevat[%g,%g];\n",
	m_listTrack[ index ].az, point.azimut(),
	m_listTrack[ index ].el, point.elevat()
    );
#endif
    return point;
}

/** Метод возвращает допустимый угол для начальной точки траектории по азимуту. */
double MAntCoord::startPointAzimut(float zeroCorrect) {
    double pointTrack = +0.0;
    unsigned int countPoint = m_listTrack.count();
    for ( unsigned int i = 0; i < countPoint; ++i ) {
	pointTrack = m_listTrack[ i ].az;
	if ( m_ant->isAzimutValid( MnMath::rad2deg( pointTrack ), zeroCorrect) )
	    break;
    };

    if ( m_angleType == DEGREE )
	pointTrack = MnMath::rad2deg( pointTrack );

#ifdef _MDEBUG_
    printf( "[ MAntCoord::startPointAzimut() ] - start point azimut=[%g];\n", pointTrack );
#endif
    return pointTrack;
}

/** Установить траекторию, для дальнейшего пересчета. */
void MAntCoord::setTrack( const QList< MnSat::TrackTopoPoint >& track ) {
    m_listTrack.clear();
    m_listTrack = track;
   
    var(m_ant->type());

    findMaxEl();
    if (m_ant->type() == mappi::conf::kSuzhetAntenna ||
	m_ant->type() == mappi::conf::kCafAntenna) {
      
      // переворот
      if ( isCoup() ) {
	coup();
      } //разгон
      else if ( isMaxRateAzimut() ) {
	correctRacingIntervalEl();
      }
      
    }  else if ( isMaxRateAzimut() ) {
      correctRacingIntervalEl();
    }
    

      //var(isMaxRateAzimut());

    // коррекция точек траектории в IV четверти
    convertTransitionAz();
}

/** Поиск максимального угла места в траектории. */
void MAntCoord::findMaxEl() {
    m_indexMaxEl = 0;
    m_maxEl = 0;
    unsigned int countPoint = m_listTrack.count();
    for ( unsigned int i = 0; i < countPoint; ++i ) {
	if ( 0 < ( m_listTrack[ i ].el - m_maxEl ) ) {

	    m_maxEl = m_listTrack[ i ].el;
	    m_indexMaxEl = i;
	};
    };
}

/*!
 * Коррекция угла места, на возможном интервале разгона.
 * Границы возможного интервала разгона определяются из условия
 * ( ( 90 - угол_места ) < ( максимальная_скорость_по_углу_места_за_1_сек. \ 2 ) )
 * Коррекция заключается в замене значения угла места на 90 градусов
 * на всем возможном интервале разгона.
 */
void MAntCoord::correctRacingIntervalEl() {
    // коррекция к началу интервала
  double correction = MnMath::deg2rad( ( m_ant->dseMax() / 2 ) - 0.5 );	// 0.5 - запас

    for( int i = m_indexMaxEl; 0 < i; --i ) {
      if ( 0 <= ( ( RAD_90 - m_listTrack[ i ].el ) - correction ) )
          break;
      m_listTrack[ i ].el = +RAD_90;
    };

    unsigned int countPoint = m_listTrack.count();
    // коррекция к концу интервала, предполагается что интервал может быть несеммитричный
    for( unsigned int i = m_indexMaxEl; i < countPoint; ++i ) {
      if ( 0 <= ( ( RAD_90 - m_listTrack[ i ].el ) - correction ) )
          break;
      m_listTrack[ i ].el = +RAD_90;
    };
}

/** Проверка, есть переворот? */
bool MAntCoord::isCoup() {
    return ( ( isMaxRateAzimut() ) && ( ( COUP_VALUE - m_listTrack[ m_indexMaxEl ].el ) <= 0 ) );
}

/** Проверка, есть превышение максимальной скорости по азимуту. */
bool MAntCoord::isMaxRateAzimut() 
{
  // чтобы не выйти за границы траектории
  if (0 == m_listTrack.count() ||
      0 == m_indexMaxEl || 
      m_listTrack.count() - 1 <= (int)m_indexMaxEl) {
    return false;
  }
  
  unsigned int nextIndex = m_indexMaxEl + 1;
  unsigned int previousIndex = m_indexMaxEl - 1;
  
  double diffNextPoint = ::fabs( m_listTrack[ m_indexMaxEl ].az - m_listTrack[ nextIndex ].az );
  double diffPreviousPoint = ::fabs( m_listTrack[ m_indexMaxEl ].az - m_listTrack[ previousIndex ].az );
  double azFullRateHalf = ( m_azFullRate / 2 );
  return (
	  ( 0 < ( diffNextPoint - azFullRateHalf ) )
	  && ( 0 < ( diffPreviousPoint - azFullRateHalf ) )
	  );
}

/*! Переворот. */
void MAntCoord::coup() {
    float azCorrect = RAD_180 * signCoup();
    unsigned int countPoint = m_listTrack.count();
    for ( unsigned int i = m_indexMaxEl /*+ 1*/; i < countPoint; ++i ) {
	m_listTrack[ i ].az = azCorrect + m_listTrack[ i ].az;
	m_listTrack[ i ].el = RAD_180 - m_listTrack[ i ].el;
    };
    for ( int i = 0; i <  m_listTrack.count(); ++i ) {
      if (m_listTrack[i].az >  MnMath::deg2rad(m_ant->azMax())) {
        m_listTrack[ i ].az -= RAD_360;
      }
      if (m_listTrack[i].az <  MnMath::deg2rad(m_ant->azMin())) {
        m_listTrack[ i ].az += RAD_360;
      }
    }
}

/*! Возвращает знак переворота (+1 или -1). */
int MAntCoord::signCoup() {
    // чтобы не выйти за границы траектории
    unsigned int countPoint = m_listTrack.count();
    unsigned int nextIndex = ( countPoint < ( m_indexMaxEl + 2 ) ? countPoint : m_indexMaxEl + 2 );
    unsigned int previousIndex = ( 2 < m_indexMaxEl ? m_indexMaxEl - 2 : 0 );

    return ( 0 < ( m_listTrack[ nextIndex ].az - m_listTrack[ previousIndex ].az ) ? -1 : +1 );
}

/**
 * Пересчет траектории по азимуту с учетом перехода (отрицательного\положительного)
 * Отрицательный переход ( 1 -> 4 четверть )
 * Положительный переход ( 4 -> 1 четверть )
 */
void MAntCoord::convertTransitionAz() {
    unsigned int posZero = 0;
    unsigned int posQuarterIV = 0;
    bool isJumpZero = isAzJumpZero( &posZero );
    bool isQuarterIV = isAzQuarterIV( &posQuarterIV );
    // если конвертация не требуется
    if ( ( !isJumpZero ) && ( !isQuarterIV ) )
	return;

    // по умолчанию, предполагается, что переход отрицательный (posZero == posQuarterIV)
    unsigned int startIndex = posZero;
    unsigned int stopIndex = m_listTrack.count();
    // если переход положительный
    if ( posQuarterIV < posZero ) {
      startIndex = 0;
      stopIndex = posZero;
    };

    for (unsigned  int i = startIndex; i <  stopIndex; ++i ) {
        m_listTrack[ i ].az -= RAD_360;
    }
}

/*!
 * Проверка траектории, есть ли по азимуту переход через ноль?
 * \param position - позиция(индекс) перехода через ноль в траектории.
 * \return true - по азимуту есть переход через ноль
 * \warning - позиция(индекс) перехода в траектории всегда больше нуля.
 * Если позиция равна 0, то перехода через ноль НЕ БЫЛО
 */
bool MAntCoord::isAzJumpZero( unsigned int* position /*= 0*/ ) {
    bool isJumpZero = false;
    unsigned int countPoint = m_listTrack.count();
    for ( unsigned int i = 1; i < countPoint; ++i ) {
	double diffPoint = ::fabs( m_listTrack[ i ].az - m_listTrack[ i - 1 ].az );
	if ( JUMP_ZERO - diffPoint <= 0 ) {

	    if ( position != 0 )
		*position = i;

	    isJumpZero = true;
	    break;
	};
    };
    return isJumpZero;
}

/*!
 * Проверка траектории, находиться ли значение азимута в IV четверти?
 * \param position - позиция(индекс) первого значения азимута в IV четверти
 * \return - true значения азимута находиться в IV четверти
 */
bool MAntCoord::isAzQuarterIV( unsigned int* position /*= 0*/ ) {
    bool isQuarterIV = false;
    unsigned int countPoint = m_listTrack.count();
    double min_Quarter_IV = MnMath::deg2rad( m_ant->azMax() + 0.1 );
    for ( unsigned int i = 0; i < countPoint; ++i ) {
	if ( ( 0 <= ( m_listTrack[ i ].az - min_Quarter_IV ) )
	    && ( ( m_listTrack[ i ].az - MAX_QUARTER_IV ) <= 0 ) ) {

	    if ( position != 0 )
		*position = i;

	    isQuarterIV = true;
	    break;
	};
    };
    return isQuarterIV;
}
