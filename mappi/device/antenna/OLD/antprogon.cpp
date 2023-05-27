/**
    Класс представляет собой реализацию алгоритма прогона.

    @author	Бурыкин Е.А.
    @date	21.08.2009
*/
#include "antprogon.h"
#include "prbl-ant.h"

#include <unistd.h>

using namespace mappi;
using namespace receive;


AntProgon::AntProgon(float dsaMax, float dseMax, float azMin, float azMax, float elMin, float elMax) :
  m_azimut( 0.0 ),
  m_elevat( 0.0 ),
  m_azMin( azMin ),
  m_azMax( azMax ),
  m_elMin( elMin ),
  m_elMax( elMax ),
  m_DSA( 0.0 ),
  m_DSE( 0.0 ),
  m_dsaMax( dsaMax ),
  m_dseMax( dseMax ),  
  m_isPositivDirectAzimut( true ),
  m_isPositivDirectElevat( true )
{
}

AntProgon::~AntProgon() {
}

/** Определение направления хода. */
void AntProgon::direction() 
{
  m_isPositivDirectAzimut = true;
  if ( m_azimut < 0 ) {
    m_isPositivDirectAzimut = false;
  }

  m_isPositivDirectElevat = true;
  if ( m_elevat <= 90 ) {
    m_isPositivDirectElevat = false;
  }
}

/**
    Установка начального положения антенны, градусы.
    @return true - если значения допустимы.
*/
void AntProgon::setStartPosition( const double& az, const double& el )
{
  m_azimut = az;
  m_elevat = el;
  direction();
}

/**
    Устанвока шага прогона по азимуту, градусы.
    Максимальное значение DSA = 9 град\сек.
    @return true - если значение допустимо.
*/
bool AntProgon::setDSA( const double& value )
{
  bool is_OK = false;
  if ( (0 <= value) && (value <= m_dsaMax) ) {
    m_DSA = value / 2;	// т.к. команды необходимо писать один раз в полсекунды
    is_OK = true;
  };
  
  return is_OK;
}

/**
    Установка шага прогона по углу места, градусы.
    Максимальное значение DSE = 10 град\сек.
    @return true - если значение допустимо.
*/
bool AntProgon::setDSE( const double& value ) 
{
  bool is_OK = false;
  if ( (0 <= value) && (value <= m_dseMax) ) {
    m_DSE = value / 2;
    is_OK = true;
  };
  
  return is_OK;
}

/** Следующие значение для азимута. */
double AntProgon::nextAzimut() 
{
  // положительное направление по азимуту
  if ( m_isPositivDirectAzimut ) {
    
    if ( ( m_azimut + m_DSA ) <= m_azMax )
      m_azimut += m_DSA;
    else {
      m_isPositivDirectAzimut = false;
      m_azimut -= m_DSA;
    };
    // отрицательное направление по азимуту
  } else {
    
    if ( m_azMin <= ( m_azimut - m_DSA ) )
      m_azimut -= m_DSA;
    else {
      m_isPositivDirectAzimut = true;
      m_azimut += m_DSA;
    };
  };
  
  return m_azimut;
}

/** Следующие значение для угла места. */
double AntProgon::nextElevat() 
{
  // положительное направление по углу места
  if ( m_isPositivDirectElevat ) {
    
    if ( ( m_elevat + m_DSE ) <= m_elMax )
      m_elevat += m_DSE;
    else {
      m_isPositivDirectElevat = false;
      m_elevat -= m_DSE;
    };
    // отрицательное направление по углу места
  } else {
    
    if ( m_elMin <= ( m_elevat - m_DSE ) )
      m_elevat -= m_DSE;
    else {
      m_isPositivDirectElevat = true;
      m_elevat += m_DSE;
    };
  };
  
  return m_elevat;
}
