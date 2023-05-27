/*****************************************************************************\
 * TimeOps.cpp
 *
 * TimeOps contains misc. time operations
 *
 * author: mark huss (mark@mhuss.com)
 *
\*****************************************************************************/

#include "TimeOps.h"

#include <time.h>
#include <math.h>
#include <stdio.h>

double TimeOps::tzOffsetInDays() {
  time_t tt = time(0);
  struct tm* pt = localtime(&tt);
  int localh = pt->tm_hour;
  pt = gmtime(&tt);
  int gmth = pt->tm_hour;
  if ( gmth < localh )
    gmth += 24;
  return Astro::toDays( localh - gmth );  // local - GMT
}

double TimeOps::dstOffsetInDays() {
  time_t tt = time(0);
  struct tm* pt = localtime(&tt);
  return (pt->tm_isdst > 0) ? Astro::toDays(-1) : 0.;
}

void TimeOps::formatTime( char* buf, double t ) {
  if (!buf)
    return;
  int hours = int( floor( t * Astro::HOURS_PER_DAY ) );
  int minutes = int ((t - Astro::toDays(hours)) * Astro::MINUTES_PER_DAY + Astro::ROUND);
  sprintf( buf, "%02d:%02d", hours, minutes );
}

void TimeOps::toHMS(double dayFrac, int& h, int& m, int& s) {
  dayFrac += .5;  // JD starts at noon
  double f = dayFrac - floor(dayFrac);
  f *= Astro::HOURS_PER_DAY;
  h = int(f);
  f -= h;
  f *= Astro::MINUTES_PER_HOUR;
  m = int(f);
  f -= m;
  s = int(f * Astro::SECONDS_PER_MINUTE + Astro::ROUND);
}
