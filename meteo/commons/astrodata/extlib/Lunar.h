/*****************************************************************************\
 * Lunar.h
 *
 * Lunar is a class that can calculate lunar fundmentals for any reasonable
 * time.
 *
 * author: mark huss (mark@mhuss.com)
 * Based on Bill Gray's open-source code at projectpluto.com
 *
\*****************************************************************************/
#if !defined( LUNAR__H )
#define LUNAR__H

#include <math.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include "AstroOps.h"
#include "DateOps.h"
#include "TimeOps.h"

// A struct to hold the fundmental elements
// The member names should be familiar to Meeus fans ;-)
struct LunarFundamentals {
  double Lp;
  double D;
  double M;
  double Mp;
  double F;
  double A1;
  double A2;
  double A3;
  double T;

  LunarFundamentals():Lp(0.),D(0.),M(0.),Mp(0.),F(0.),A1(0.),A2(0.),A3(0.),T(0.) {}
};

// terms for longitude & radius
static const int N_LTERM1 = 60;
struct LunarTerms1 {
  char d, m, mp, f;
  long sl, sr;
};

// terms for latitude
static const int N_LTERM2 = 60;
struct LunarTerms2 {
  char d, m, mp, f;
  long sb;
};

// our main class -- calculates Lunar fundamentals, lat, lon & distance
class Lunar {
public:
  // default c'tor
  Lunar() : m_initialized( false ), m_lon(-1.), m_lat(-1.), m_r(-1.)
  {}

  // data & time c'tor
  // t = decimal julian centuries
  Lunar( double t )
  {
     calcFundamentals( t );
  }

/**
 * Exact number of days from one new moon to the next new moon.
 */
  static const double SYNODIC_MONTH;// = 29.530588861;

  double illuminatedFraction();

  static double ageOfMoonInDays( double jd, double* fraction = 0 );
/**
  * Calculate the current age of the moon in days (0.0 to 29.53...)
  */
  static double ageOfMoonInDays( /* now */ ) {
    return ageOfMoonInDays( DateOps::now() );
  }

  static double ageOfMoonInDays( int day, int month, long year, long second ) {
    return ageOfMoonInDays( DateOps::dmyToDoubleDay( day, month, year, second ) );
  }


 /**
  * The base Julian day for Ernest W. Brown's numbered series of
  * lunations = 1923 Jan 17 02:41 UT.
  * <P>
  * This date has been widely quoted as "Jan 16 1923" and indeed it
  * was (in EST) at Yale University where Prof. Brown worked.
  */
  static const double LUNATION_BASE;

 /**
  * Calculate the lunation for specified Julian day.
  * <P>
  * A "lunation" is E. W. Brown's numbered series of lunar cycles.
  * Lunation 1 was on January 16, 1923.
  *
  * @param jd Julian day
  *
  * @return Lunation number
  */
  static int lunation( double jd );
  static int lunation( long jd ) {
    return lunation( double( jd ) );
  }
  static int lunation( /* now */ ) {
    return lunation( DateOps::now() );
  }

  static void summary(char* buf, ObsInfo& oi);


  // calculates the fundamanentals given the time
  // t = decimal julian centuries
  //
  void calcFundamentals( double t );

  //
  // NOTE: calcFundamentals() must be called before calling the functions
  //       below, or an invalid result (-1.) will be returned.
  //

  double phaseAngle();
  // returns lunar latitude
  double latitude();         // returns -1 if not initialized
  double latitudeRadians() {  // returns -1 if not initialized
    return ( m_initialized ) ? Astro::toRadians( latitude() ) : -1.;
  }

  // returns lunar longitude
  double longitude()         // returns -1 if not initialized
  {
    if ( m_lon < 0. )
      calcLonRad();
    return m_lon;
  }

  double longitudeRadians()  // returns -1 if not initialized
  {
    return ( m_initialized ) ? Astro::toRadians( longitude() ) : -1.;
  }

  // returns lunar distance
  double radius()     // returns -1 if not initialized
  {
    if ( m_r < 0. )
      calcLonRad();
    return m_r;
  }

  // calculate all three location elements of the spec'd body at the given time
  void calcAllLocs(
      double& lon,            // returned longitude
      double& lat,            // returned latitude
      double& rad,            // returned radius vector
      double t)               // time in decimal centuries
  {
    calcFundamentals( t );
    lon = longitudeRadians();
    lat = latitudeRadians();
    rad = radius();
  }

private:
  // reduce (0 < d < 360) a positive angle and convert to radians
  double normalize( double d ) {
    return Astro::toRadians( AstroOps::normalizeDegrees( d ) );
  }

  // calculate an individual fundimental
  //  tptr - points to array of doubles
  //  t - time in decimal Julian centuries
  double getFund( const double* tptr, double t );

  // calculate longitude and radius
  //
  // NOTE: calcFundamentals() must have been called first
  void calcLonRad();

  // ***** data  *****
  // our calculated fundmentals
  LunarFundamentals m_f;

  // true if calcFundamentals has been called
  bool m_initialized;

  // longitude, latitude, and radius (stored in _degrees_)
  double m_lon, m_lat, m_r;
};

#endif  /* #if !defined( LUNAR__H ) */
