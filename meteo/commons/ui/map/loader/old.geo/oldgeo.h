#ifndef METEO_MAP_OLDGEO_H
#define METEO_MAP_OLDGEO_H

#include <stdint.h>
#include <stdio.h>

#include <utility>
#include <string>

namespace meteo {
namespace map {

class Document;

namespace oldgeo {

bool loadGeoData( Document* doc );

}
}
}

#endif
