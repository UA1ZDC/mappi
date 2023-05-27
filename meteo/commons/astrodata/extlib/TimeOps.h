/*****************************************************************************\
 * TimeOps.h
 *
 * TimeOps contains misc. time operations
 *
 * author: mark huss (mark@mhuss.com)
 *
\*****************************************************************************/
#if !defined( TIME_OPS__H )
#define TIME_OPS__H

#include "AstroOps.h"

class TimeOps {
public:
  static double tzOffsetInDays();
  static double dstOffsetInDays();
  static void formatTime( char* buf, double t );
  static void toHMS(double dayFrac, int& h, int& m, int& s);

private:

};

#endif  /* #if !defined( TIME_OPS__H ) */
