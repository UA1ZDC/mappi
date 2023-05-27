#ifndef METEO_COMMONS_ASTRODATA_ASTRODATAWRAP_ASTROWRAP_H
#define METEO_COMMONS_ASTRODATA_ASTRODATAWRAP_ASTROWRAP_H

#include <boost/python.hpp>
#include <meteo/commons/astrodata/astrodata.h>

class AstroWrap
{
public:
  AstroWrap();
  ~AstroWrap();
  void setStation( float lat, float lon, float height);
  void setDate( std::string begin, std::string end );
  void calcPoint();
  std::string getSunRise();
  std::string getSunSet();
  std::string getLunarRise();
  std::string getLunarSet();
  std::string getTwilightNauticalBegin();
  std::string getTwilightNauticalEnd();
  std::string getLunarQuartet();

private:
  astro::AstroDataCalendar* calendar_ = nullptr;
};

#endif // METEO_COMMONS_ASTRODATA_ASTRODATAWRAP_ASTROWRAP_H
