#ifndef TGRIB_CALC_H
#define TGRIB_CALC_H

namespace grib {
  class TGribData;

  bool copyIdentificataion(const grib::TGribData& src, grib::TGribData* dst);

  //! скорость и направление ветра из векторных составляющих
  bool uv2ff(const grib::TGribData& u, const grib::TGribData& v,  grib::TGribData* ff, grib::TGribData* dd);
  //! температура точки росы в дефицит
  bool Td2D(const grib::TGribData& T, const grib::TGribData& Td, grib::TGribData* D);
  //! влажность в дефицит
  bool U2D(const grib::TGribData& T, const grib::TGribData& U,  grib::TGribData* D);
}

#endif
