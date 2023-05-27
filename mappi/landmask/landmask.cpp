#include "landmask.h"

#include <meteo/commons/astrodata/astrodata.h>
#include <meteo/commons/astrodata/observerpoint.h>

namespace mappi {

bool LandMask::initLandSea()
{
  int r = landSea_.init();
  if ( ERR_NOERR != r ) {
    return false;
  }

  return true;
}

bool LandMask::initElevation()
{
  return elevation_.init();
}

bool LandMask::initDayNight(const QDateTime& dt)
{
  dt_ = dt;
  return true;
}

bool LandMask::initSst(int month)
{
  if ( month < 1 || month > 12 ) {
    return false;
  }

  int r = sst_.init_sst(month);
  if ( ERR_NOERR != r ) {
    return false;
  }

  return true;
}

bool LandMask::initAlbedo(int month)
{
  if ( month < 1 || month > 12 ) {
    return false;
  }

  int r = albedo_.init_albedo(month);
  if ( ERR_NOERR != r ) {
    return false;
  }

  return true;
}

bool LandMask::initHumidity(int month)
{
  if ( month < 1 || month > 12 ) {
    return false;
  }

  int r = humidity_.init_wv(month);
  if ( ERR_NOERR != r ) {
    return false;
  }

  return true;
}

LandMask::surfType LandMask::land(const ::meteo::GeoPoint& coord)
{
  return static_cast<LandMask::surfType>(landSea_.get(coord.latDeg(),coord.lonDeg()));
}

int LandMask::elevation(const meteo::GeoPoint& coord) const
{
  return elevation_.value(coord.latDeg(),coord.lonDeg());
}


int LandMask::day(const meteo::GeoPoint& coord, const QDateTime& dt)
{
  dt_ = dt;
  return day(coord);
}


int LandMask::day(const meteo::GeoPoint& coord) const
{
  ::astro::AstroDataCalendar calendar;
  ::astro::ObserverPoint& p = calendar.observerPoint();
  p.setLatitude(coord.latDeg());
  p.setLongitude(coord.lonDeg());
  p.setHeight(coord.alt());

  p.setFirstDateTime(QDateTime(dt_.date(), QTime()));
  p.setSecondDateTime(QDateTime(dt_.date(), QTime()));

  calendar.clear();
  calendar.makeUp(::astro::AstroDataCalendar::kMakeSun);

  QTime t = dt_.time();
  QTime rise = calendar.day(0).getSunRise();
  QTime set = calendar.day(0).getSunSet();
  if ( t >= rise && t < set ) {
    return 1;
  }

  return 0;
}

int LandMask::sst(const meteo::GeoPoint& coord)
{
  return sst_.get_atlas_data(coord.latDeg(),coord.lonDeg());
}

int LandMask::albedo(const meteo::GeoPoint& coord)
{
  return albedo_.get_atlas_data(coord.latDeg(),coord.lonDeg());
}

int LandMask::waterVapor(const meteo::GeoPoint& coord)
{
  return humidity_.get_wv_data(coord.latDeg(),coord.lonDeg(),coord.alt());
}

} // mappi
