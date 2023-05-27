#ifndef METEO_MAP_PH_H
#define METEO_MAP_PH_H

#include <stdint.h>
#include <stdio.h>

#include <utility>
#include <string>

namespace meteo {
namespace map {

class Map;

namespace ph {

bool loadGeoData( Map* map );

}
}
}

#endif
