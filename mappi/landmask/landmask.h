#ifndef MAPPI_LANDMASK_LANDMASK_H
#define MAPPI_LANDMASK_LANDMASK_H

#include <qpoint.h>
#include <qdatetime.h>

#include <commons/geobasis/geopoint.h>
#include <commons/landmask/landmask.h>
#include <commons/landmask/elevation.h>

#include <meteo/commons/astrodata/astrodata.h>

namespace mappi {


class LandMask
{
public:
  enum surfType { SEAMASK = 0, LANDMASK = 1  };//1 for land mask , 0 for sea mask

  explicit LandMask(){}

  bool initLandSea();
  bool initElevation();
  bool initDayNight(const QDateTime& dt);//хреновая реализация.
  bool initSst(int month);
  bool initAlbedo(int month);
  bool initHumidity(int month);

  //! Возвращаемые значения: 1 - суша, 0 - вода, -1 - ошибка.
  surfType land(const ::meteo::GeoPoint& coord);
  //! Возвращает высоту в метрах над уровнем моря (-500 - вода).
  int elevation(const ::meteo::GeoPoint& coord) const;
  //! Возвращаемые значения: 1 - день, 0 - ночь.
  int day(const ::meteo::GeoPoint& coord) const;
  int day(const ::meteo::GeoPoint& coord, const QDateTime& dt);
  //! Возращает температуру поверхности моря в градусах Цельсия * 100.
  int sst(const ::meteo::GeoPoint& coord);
  //! Возращает альбедо в процентах * 100.
  int albedo(const ::meteo::GeoPoint& coord);
  //! Возвращает влажность в гр/кг*100.
  int waterVapor(const ::meteo::GeoPoint& coord);

private:
  ::astro::AstroDataCalendar calendar_;
  QDateTime dt_;

  ::meteo::Elevation elevation_;
  ::meteo::map::internal::LandMask landSea_;
  ::meteo::map::internal::LandMask sst_;
  ::meteo::map::internal::LandMask albedo_;
  ::meteo::map::internal::LandMask humidity_;
};

} // mappi

#endif // MAPPI_LANDMASK_LANDMASK_H
