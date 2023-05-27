#ifndef METEO_COMMONS_MODELS_HWM14_FUNCS_H
#define METEO_COMMONS_MODELS_HWM14_FUNCS_H

#include <qlist.h>
#include <qdatetime.h>

extern "C" {

//!!!============================================================================
//!!! Input arguments:
//!!!        iyd - year and day as yyddd
//!!!        sec - ut(sec)
//!!!        alt - altitude(km)
//!!!        glat - geodetic latitude(deg)
//!!!        glon - geodetic longitude(deg)
//!!!        stl - not used
//!!!        f107a - not used
//!!!        f107 - not used
//!!!        ap - two element array with
//!!!             ap(1) = not used
//!!!             ap(2) = current 3hr ap index
//!!!
//!!! Output argument:
//!!!        w(1) = meridional wind (m/sec + northward)
//!!!        w(2) = zonal wind (m/sec + eastward)
//!!!
//!!!============================================================================
void hwm14_(int *iyd, float *sec, float *alt, float *glat, float *glon, float *stl, float *f107a, float *f107, float *ap, float *w);
void dwm07_(int *iyd, float *sec, float *alt, float *glat, float *glon, float *ap, float *w);
void dwm07b_(float* mlt, float* mlat, float* kp, float* mmpwind, float* mzpwind);

}

namespace meteo {
namespace hwm14 {

struct WindPoint
{
  float u;
  float v;
  float dd;
  float ff;
  float lat;
  float lon;
  float alt;
};

enum CalcFlag {
  kNoFlag           = 0x00,
  kConvertUV2DDFF   = 0x01,
};

typedef QList<WindPoint> WindPointList;

void uvWind(float* u, float* v, float latDeg, float lonDeg, float altKm, const QDateTime& dt, float ap = -1.0);

QList<WindPoint> heightProfile(float latDeg,
                               float lonDeg,
                 const QList<float>& altKmList,
                    const QDateTime& dt,
                            CalcFlag flag = kNoFlag,
                               float ap = -1.0);


} // hwm14
} // meteo


#endif // METEO_COMMONS_MODELS_HWM14_FUNCS_H
