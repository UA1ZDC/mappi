#ifndef METEO_COMMONS_UI_MAP_DATAEXCHANGE_PASSPORTBASIS_H
#define METEO_COMMONS_UI_MAP_DATAEXCHANGE_PASSPORTBASIS_H

#include <commons/geobasis/geopoint.h>

namespace meteo {
namespace sxf {

struct PassportBasis
{
  int scale;           //!< Знаменатель масштаба
  int projection;      //!< Тип проекции (значение согласно документации к формату SXF)
  //!< Координаты углов листа:
  GeoPoint southWest;
  GeoPoint northWest;
  GeoPoint northEast;
  GeoPoint southEast;
  //!< Постоянные проекции:
  GeoPoint mainPoint;          //!< (Параллель главной точки, Осевой меридиан)

  double firstMajorParallel;   //!< Первая главная параллель (в радианах)
  double secondMajorParallel;  //!< Вторая главная параллель (в радианах)

  double northOffset;          //!< Смещение на север (в метрах)
  double eastOffset;           //!< Смещение на восток (в метрах)

  QRect band;
};

} // sxf
} // meteo

#endif // METEO_COMMONS_UI_MAP_DATAEXCHANGE_PASSPORTBASIS_H
