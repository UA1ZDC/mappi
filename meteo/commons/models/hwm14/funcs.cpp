#include "funcs.h"

#include <cross-commons/app/paths.h>
#include <cross-commons/debug/tlog.h>

#include <commons/mathtools/mnmath.h>

namespace meteo {
namespace hwm14 {

QString hwmPath()
{ return MnCommon::etcPath("meteo") + "/models/hwm"; }

QList<WindPoint> heightProfile(float latDeg,
                               float lonDeg,
                               const QList<float>& altKmList,
                               const QDateTime& dt,
                               CalcFlag flag,
                               float ap)
{
  float notUsedFloat = 0;

  QList<WindPoint> list;
  list.reserve(altKmList.size());

  QDateTime utc = dt.toUTC();
  int day = utc.date().dayOfYear();
  int yyddd = QString("%1%2").arg(utc.toString("yy")).arg(day, 3, 10, QChar('0')).toInt();
  float sec = utc.time().hour() * 3600.0 + utc.time().minute() * 60.0 + utc.time().second();

  float apArr[2] = {0,ap};

  const char* oldenv = ::getenv("HWMPATH");
  ::setenv("HWMPATH", hwmPath().toLocal8Bit().constData(), 1);

  WindPoint wp;
  wp.lat = latDeg;
  wp.lon = lonDeg;

  float w[2] = {0,0};
  float alt;
  for ( int i=0,isz=altKmList.size(); i<isz; ++i ) {
    alt = altKmList[i];
    hwm14_(&yyddd, &sec, &alt, &latDeg, &lonDeg, &notUsedFloat, &notUsedFloat, &notUsedFloat, apArr, w);
    wp.v = w[0];
    wp.u = w[1];
    wp.alt = alt;
    if ( kConvertUV2DDFF == flag ) { MnMath::preobrUVtoDF(wp.u, wp.v, &wp.dd, &wp.ff); }

    list << wp;
  }

  if ( nullptr != oldenv ) {
    ::setenv("HWMPATH", oldenv, 1);
  }
  else {
    ::unsetenv("HWMPATH");
  }

  return list;
}

} // hwm14
} // meteo
