#ifndef COMMONS_LANDMASK_ELEVATION_H
#define COMMONS_LANDMASK_ELEVATION_H

#include <qfile.h>

namespace meteo {

struct TileInfo {
  int columns;
  int rows;
  double begLat;
  double endLat;
  double begLon;
  double endLon;
  int min;
  int max;
  char fileName[5];
};

class Elevation
{
public:
  static constexpr double kLonStep = 90.0 / 10800;
  static constexpr double kLatStep = 50.0 / 6000;

  const TileInfo kTileInfo[16] = {
    { 10800, 4800,  50,  90, -180,  -90,     1, 6098, "a11g" },
    { 10800, 4800,  50,  90,  -90,    0,     1, 3940, "b10g" },
    { 10800, 4800,  50,  90,    0,   90,   -30, 4010, "c10g" },
    { 10800, 4800,  50,  90,   90,  180,     1, 4588, "d10g" },
    { 10800, 6000,   0,  50, -180,  -90,   -84, 5443, "e10g" },
    { 10800, 6000,   0,  50,  -90,    0,   -40, 6085, "f10g" },
    { 10800, 6000,   0,  50,    0,   90,  -407, 8752, "g10g" },
    { 10800, 6000,   0,  50,   90,  180,   -63, 7491, "h10g" },
    { 10800, 6000, -50,   0, -180,  -90,     1, 2732, "i10g" },
    { 10800, 6000, -50,   0,  -90,    0,  -127, 6798, "j10g" },
    { 10800, 6000, -50,   0,    0,   90,     1, 5825, "k10g" },
    { 10800, 6000, -50,   0,   90,  180,     1, 5179, "l10g" },
    { 10800, 4800, -90, -50, -180,  -90,     1, 4009, "m10g" },
    { 10800, 4800, -90, -50,  -90,    0,     1, 4743, "n10g" },
    { 10800, 4800, -90, -50,    0,   90,     1, 4039, "o10g" },
    { 10800, 4800, -90, -50,   90,  180,     1, 4363, "p10g" },
  };

public:
  Elevation(){}

  bool init();

  int value(double lat, double lon) const;

private:
  mutable uint   pos_[16];
  mutable qint16 value_[16];
  mutable QFile  tiles_[16];
};

} // meteo

#endif // COMMONS_LANDMASK_ELEVATION_H
