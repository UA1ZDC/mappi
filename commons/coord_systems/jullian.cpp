#include <math.h>

#include <commons/mathtools/mnmath.h>
#include "jullian.h"

using namespace MnMath;

const double EPOCH_1JAN_2000_12H = 2451545.0; //1 января 2000, 12h UTC по Юлианскому календарю
const double EPOCH_1JAN_1900_12H = 2415020.0; //1 января 1900, 12h UTC по Юлианскому календарю

JullianDate::JullianDate( )
{
  init( 2000, 1 );
}


/*! 
 * \param year год
 * \param day  день от начала года 
 */
JullianDate::JullianDate( uint year, double day )
{
  init( year, day );
}


JullianDate::JullianDate( const QDateTime& dt )
{
  uint year = dt.date().year();

  QTime time = dt.time();
  double day = dt.date().dayOfYear() + 
    (time.hour() + (time.minute() + ((time.second() + (time.msec() / 1000.0) )/ 60.0)) / 60.0) / 24.0;
  
  init( year, day );
}

JullianDate::~JullianDate()
{
}


void JullianDate::init( uint year, double day )
{
  // 1582 A.D.: 10 days removed from calendar
  // 3000 A.D.: Arbitrary error checking limit
  if ( year <= 1582 || year >= 3000 || 
       day < 0.0 || day > 367 ) { //с учётом високосного года
    _jDate = EPOCH_1JAN_2000_12H;
    return;
  }
  
  //подход Жана Мееса
  --year;
  
  int A = (year / 100);
  int B = 2 - A + (A / 4);
  double jYear = (int)(365.25 * year) + (int)(30.6001 * 14) + 1720994.5 + B;
  _jDate = jYear + day;  
}


void JullianDate::setfromJan1_12h_1900(double day)
{
  _jDate = EPOCH_1JAN_1900_12H + day;
}

/*! 
 * \brief преобразование Юлианской даты в гринвичевское сиредическое время(GMST)
 * \return  гринвичевское сиредическое время, в радианах (Theta GMST)
 */
double JullianDate::toGMST() const
{
  //+ 0.5, т.к. количество дней с 1 января 12h будет иметь нецелое значение (NN.5), чтоб не учитывать эти 0.5 при восстановлении части дня (т.е. времени) даты _jDate
  double dayFract = fmod( _jDate + 0.5, 1.0 );

  //Tu = du / 36525.0; du - количество дней UT, прошедших с JD 2451545.0 (1 января 2000, 12h UT1)
  double Tu = (_jDate - EPOCH_1JAN_2000_12H - dayFract) / 36525.0;

  //гринвичское сидерическое время в 0h (полночь) UTC
  double gmst_0h = 24110.54841 + Tu * ( 8640184.812866 + Tu * ( 0.093104 - 6.2e-6 * Tu ) );

  double gmst = fmod( gmst_0h + dayFract * SEC_PER_DAY * EARTH_SPEED, SEC_PER_DAY);

  if ( gmst < 0.0 ) {
    gmst += SEC_PER_DAY;
  }
    
  return  M_2PI * ( gmst / SEC_PER_DAY );
}


/*! 
 * \brief вычисление местного GMST (Greenwich Mean Sidereal Time)
 * \param longitude долгота (радиан)
 * \return местное GMST 
 */
double JullianDate::toLGMST( double longitude ) const
{
  return fmod( toGMST() + longitude, M_2PI );
}

double JullianDate::getDate() const
{
  return _jDate;
}

//! Получение модифицированной  юлианской даты на начало суток
double JullianDate::getModif() const
{
  //+ 0.5, т.к. количество дней с 1 января 12h будет иметь нецелое значение (NN.5), чтоб не учитывать эти 0.5 при восстановлении части дня (т.е. времени) даты _jDate
  double dayFract = fmod( _jDate + 0.5, 1.0 );
  
  //Tu = du / 36525.0; du - количество дней UT, прошедших с JD 2451545.0 (1 января 2000, 12h UT1)
  return  (_jDate - EPOCH_1JAN_2000_12H - dayFract) / 36525.0;
}

double JullianDate::fromJan1_12h_1900() const
{
  return _jDate - EPOCH_1JAN_1900_12H;
}

void JullianDate::addMin( double min )
{ 
  _jDate += (min / MIN_PER_DAY);
}
