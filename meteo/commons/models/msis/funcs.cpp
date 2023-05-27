#include "funcs.h"

namespace meteo {
namespace msis {

QList<DataPoint> heightProfile(float latDeg,
                               float lonDeg,
                 const QList<float>& altKmList,
                    const QDateTime& dt, int calcFlags,
                               float f107,
                               float f107a,
                               float ap)
{
  QList<DataPoint> list;
  list.reserve(altKmList.size());

  nrlmsise_input input;
  input.ap = ap;
  input.f107 = f107;
  input.f107A = f107a;
  input.g_lat = latDeg;
  input.g_long = lonDeg;

  QDateTime utc = dt.toUTC();
  input.doy = utc.date().dayOfYear();
  input.year = utc.date().year();
  input.sec = utc.time().hour() * 3600.0 + utc.time().minute() * 60.0 + utc.time().second();
  input.lst = input.sec / 3600.0 + input.g_long / 15.0;

  DataPoint point;
  point.lon = input.g_long;
  point.lat = input.g_lat;

  nrlmsise_flags flags;
  flags.switches[0] = 0;
  for ( int i=1; i<24; ++i ) {
    flags.switches[i] = 1;
  }

  nrlmsise_output output;
  for ( int i=0,isz=altKmList.size(); i<isz; ++i ) {
    input.alt = altKmList[i];
    gtd7(&input,&flags,&output);

    point.alt = input.alt;
    point.data = output;
    if ( kConvertK2C & calcFlags ) {
      point.data.t[1] -= 273.15;
    }

    list << point;
  }

  return list;
}

} // msis
} // meteo
