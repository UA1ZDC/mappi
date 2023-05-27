/*****************************************************************************\
 * Lunar.cpp
 *
 * Lunar is a class that can calculate lunar fundmentals for any reasonable
 *   time.
 *
 * author: mark huss (mark@mhuss.com)
 * Based on Bill Gray's open-source code at projectpluto.com
 *
\*****************************************************************************/

#include "Lunar.h"
#include "LunarTerms.h"  // data extracted from vsop.bin file

#include "MathOps.h"
#include "AstroOps.h"
#include "Vsop.h"
#include "PlanetData.h"
#include "RiseSet.h"

#include <stdlib.h>

const double Lunar::SYNODIC_MONTH = 29.530588861;
const double Lunar::LUNATION_BASE = 2423436.40347;

//-------------------------------------------------------------------------
/**
  * calculate current phase angle in radians (Meeus' easy lower precision method)
  */
double Lunar::phaseAngle() {
  if ( !m_initialized )
    return -1.;

  return normalize(
      180 - Astro::toDegrees(m_f.D)
          - 6.289 * sin( m_f.Mp )
          + 2.110 * sin( m_f.M )
          - 1.274 * sin( (2 * m_f.D) - m_f.Mp )
          - 0.658 * sin( 2 * m_f.D )
          - 0.214 * sin( 2 * m_f.Mp )
          - 0.110 * sin( m_f.D )
          );
}

//-------------------------------------------------------------------------
double Lunar::illuminatedFraction() {
  if ( !m_initialized )
    return -1.;

  return (1. + cos( phaseAngle() )) / 2.;
}

//-------------------------------------------------------------------------
/**
  * Calculate age of the moon in days (0.0 to 29.53...)
  * @param jd - Julian day for which lunar age is required
  */
double Lunar::ageOfMoonInDays( double jd, double* pFraction ) {

  double jCenturies = Astro::toMillenia( jd ); // convert jd to jm ref. J2000
  Lunar luna(jCenturies);
  if ( pFraction )
    *pFraction = luna.illuminatedFraction();

  // first calculate solar ecliptic longitude (in RAD)
  //
  double earthLon = Vsop::calcLoc( jCenturies, EARTH, Vsop::ECLIPTIC_LON );
  /*
  * What we _really_ want is the location of the sun as seen from
  * the earth (geocentric view).  VSOP gives us the opposite
  * (heliocentric) view, i.e., the earth as seen from the sun.
  * To work around this, we add PI to the longitude (rotate 180 degrees)
  */
  double sunLon = earthLon + Astro::PI;

  // next calculate lunar ecliptic longitude (in RAD)
  //
  double moonLon = luna.longitudeRadians();

  // age of moon in radians = difference
  double moonAge = AstroOps::normalizeRadians( Astro::TWO_PI - (sunLon - moonLon) );

  // convert radians to Synodic day
  double sday = SYNODIC_MONTH * (moonAge / Astro::TWO_PI);
  return sday;
}

//----------------------------------------------------------------------------
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
  int Lunar::lunation( double jd ) {
    int lun = (int)( ( jd - LUNATION_BASE ) / SYNODIC_MONTH );
    // "++" to make 1-based (vs. 0-based)
    return ++lun;
  }

//----------------------------------------------------------------------------
 /**
  * Build a current lunar information summary String.
  * <P>
  * The summary contains the lunation, age in days, rise, and set
  * times.
  *
  * @param buf - where to create the summary string
  * @param oi - The observer's location
  */
  void Lunar:: summary(char* buf, ObsInfo& oi) {
    if ( !buf )
      return;

    double now = DateOps::now();
    double frac = 0.;
    double age = ageOfMoonInDays(now, &frac);
    int iDays = (int)age;
    int iHours = (int)(((age - iDays) * Astro::HOURS_PER_DAY) + Astro::ROUND);

    frac *= 100.; frac += .05;

    sprintf( buf, "Lunation %d\nage %dd %dh\n%2.1f%% illuminated\n",
        lunation(now), iDays, iHours, frac );

    char* pBuf = buf + strlen(buf);

    double offset = TimeOps::tzOffsetInDays() + TimeOps::dstOffsetInDays();
    double jd = floor(DateOps::now()) - offset - 0.5;

    TimePairRS rs;
    double rise = -1., set = -1.;

    RiseSet::getTimes( rs, RiseSet::MOON, jd, oi );
    rise = rs.rise;
    set = rs.set;
    if ( set < rise ) {
      RiseSet::getTimes( rs, RiseSet::MOON, jd+1, oi );
      set = rs.set;
    }

    char riseTime[8] = { 0 };
    if ( rise >= 0. )
      TimeOps::formatTime(riseTime, rise);

    char setTime[8] = { 0 };
    if ( set >= 0. )
      TimeOps::formatTime(setTime, set);

    if ( *riseTime && *setTime )
      sprintf( pBuf, "Rises %s, Sets %s\n", riseTime, setTime);
    else if ( *riseTime )
      sprintf( pBuf, "Rises %s\n", riseTime);
    else // ( *setTime )
      sprintf( pBuf, "Sets %s\n", setTime);

  }

 /**
  * Build a current lunar information summary String for Phila, PA,
  * USA (a shameless convenience function for dvaa.org).
  * <P>
  * The default summary contains the lunation, age in days, rise,
  * and set times for Philadelphia, PA, USA as the observer's
  * location.
  *
  * @return The summary string
  static String summaryPHL() {
    return summary( new ObsInfo( new Latitude(39.95), new Longitude(-75.16) ) )
           + " (Phila, PA)";
  }
  */

//----------------------------------------------------------------------------
// calculate an individual fundimental
//  tptr - points to array of doubles
//  t - time in decimal Julian centuries
//
double Lunar::getFund( const double* tptr, double t )
{
  double d = *tptr++;
  double tpow = t;
  for( int i=4; i!=0; i-- ) {
    d += tpow * (*tptr++);
    tpow *= t;
  }
  return normalize( d );
}

//----------------------------------------------------------------------------
// calculate the fundamanentals given the vsop.bin data and a time
//   ad has vsop.bin data
//   t = decimal julian centuries
//
void Lunar::calcFundamentals( double t )
{
  m_f.Lp = getFund( LunarFundimentals_Lp, t );
  m_f.D = getFund( LunarFundimentals_D, t );
  m_f.M = getFund( LunarFundimentals_M, t );
  m_f.Mp = getFund( LunarFundimentals_Mp, t );
  m_f.F = getFund( LunarFundimentals_F, t );

  m_f.A1 = normalize( 119.75 + 131.849 * t );
  m_f.A2 = normalize( 53.09 + 479264.290 * t );
  m_f.A3 = normalize( 313.45 + 481266.484 * t );
  m_f.T  = normalize( t );

  // indicate values need to be recalculated
  m_lat = m_lon = m_r = -1.;

  // set init'd flag to true
  m_initialized = true;
}

//----------------------------------------------------------------------------
// calculate longitude and radius
//
// NOTE: calcFundamentals() must have been called first
//
void Lunar::calcLonRad()
{
  if ( !m_initialized ) {
    m_r = m_lon = -1.;
  }
  else {
    const LunarTerms1* tptr = LunarLonRad;

    double sl = 0., sr = 0.;
    const double e = 1. - .002516 * m_f.T - .0000074 * m_f.T * m_f.T;

    for( int i=N_LTERM1; i!=0; i-- ) {
      if( labs( tptr->sl ) > 0 || labs( tptr->sr ) > 0 ) {
        double arg;

        switch( tptr->d )
        {
          case  1:   arg = m_f.D;          break;
          case -1:   arg =-m_f.D;          break;
          case  2:   arg = m_f.D+m_f.D;    break;
          case -2:   arg =-m_f.D-m_f.D;    break;
          case  0:   arg = 0.;             break;
          default:   arg = (double)(tptr->d) * m_f.D;  break;
        }

        switch( tptr->m )
        {
          case  1:   arg += m_f.M;         break;
          case -1:   arg -= m_f.M;         break;
          case  2:   arg += m_f.M+m_f.M;   break;
          case -2:   arg -= m_f.M+m_f.M;   break;
          case  0:           ;             break;
          default:   arg += (double)(tptr->m) * m_f.M;  break;
        }

        switch( tptr->mp )
        {
          case  1:   arg += m_f.Mp;        break;
          case -1:   arg -= m_f.Mp;        break;
          case  2:   arg += m_f.Mp+m_f.Mp; break;
          case -2:   arg -= m_f.Mp+m_f.Mp; break;
          case  0:           ;             break;
          default:   arg += (double)(tptr->mp) * m_f.Mp;  break;
        }

        switch( tptr->f )
        {
          case  1:   arg += m_f.F;         break;
          case -1:   arg -= m_f.F;         break;
          case  2:   arg += m_f.F+m_f.F;   break;
          case -2:   arg -= m_f.F+m_f.F;   break;
          case  0:           ;             break;
          default:   arg += (double)(tptr->f) * m_f.F;  break;
        }

        if( tptr->sl )
        {
          double term = (double)(tptr->sl) * sin(arg);
          for( int j=abs(tptr->m); j!=0; j-- )
            term *= e;
          sl += term;
        }

        if( tptr->sr )
        {
          double term = (double)(tptr->sr) * cos(arg);
          for( int j=abs(tptr->m); j!=0; j-- )
            term *= e;
          sr += term;
        }
      }
      tptr++;
    }

    sl += 3958. * sin( m_f.A1 ) +
          1962. * sin( m_f.Lp - m_f.F ) +
          318.  * sin( m_f.A2 );

    m_lon = (m_f.Lp * 180. / Astro::PI) + sl * 1.e-6;

    // reduce signed angle to ( 0 < m_lon < 360 )
    m_lon = AstroOps::normalizeDegrees( m_lon );
    m_r = 385000.56 + sr / 1000.;
  }
}

//----------------------------------------------------------------------------
// calculate (or return prev. calculated) latitude
//
// NOTE: calcFundamentals() must have been called first
//
double Lunar::latitude()
{
  if ( !m_initialized )
    return -1.;

  if ( m_lat < 0. ) {
    const LunarTerms2* tptr = LunarLat;
    double rval = 0.;

    const double e = 1. - .002516 * m_f.T - .0000074 * m_f.T * m_f.T;

    for( int i=N_LTERM2; i!=0; i-- ) {

      if( labs( tptr->sb ) > 0. ) {
        double arg;

        switch( tptr->d )
        {
          case  1:   arg = m_f.D;          break;
          case -1:   arg =-m_f.D;          break;
          case  2:   arg = m_f.D+m_f.D;    break;
          case -2:   arg =-m_f.D-m_f.D;    break;
          case  0:   arg = 0.;             break;
          default:   arg = (double)(tptr->d) * m_f.D;  break;
        }

        switch( tptr->m )
        {
          case  1:   arg += m_f.M;         break;
          case -1:   arg -= m_f.M;         break;
          case  2:   arg += m_f.M+m_f.M;   break;
          case -2:   arg -= m_f.M+m_f.M;   break;
          case  0:           ;             break;
          default:   arg += (double)(tptr->m) * m_f.M;  break;
        }

        switch( tptr->mp )
        {
          case  1:   arg += m_f.Mp;        break;
          case -1:   arg -= m_f.Mp;        break;
          case  2:   arg += m_f.Mp+m_f.Mp; break;
          case -2:   arg -= m_f.Mp+m_f.Mp; break;
          case  0:           ;             break;
          default:   arg += (double)(tptr->mp) * m_f.Mp;  break;
        }

        switch( tptr->f )
        {
          case  1:   arg += m_f.F;         break;
          case -1:   arg -= m_f.F;         break;
          case  2:   arg += m_f.F+m_f.F;   break;
          case -2:   arg -= m_f.F+m_f.F;   break;
          case  0:           ;             break;
          default:   arg += (double)(tptr->f) * m_f.F;  break;
        }

        double term = (double)(tptr->sb) * sin( arg );
        for( int j = abs(tptr->m); j!=0; j-- )
          term *= e;

        rval += term;
      }
      rval += -2235. * sin( m_f.Lp ) +
               382.  * sin( m_f.A3 ) +
               175.  * sin( m_f.A1 - m_f.F ) +
               175.  * sin( m_f.A1 + m_f.F ) +
               127.  * sin( m_f.Lp - m_f.Mp ) -
               115.  * sin( m_f.Lp + m_f.Mp );

      tptr++;
    }
    m_lat = rval * 1.e-6;
  }
  return m_lat;
}

