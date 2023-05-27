#ifndef METEO_MAP_PTKPPGEO_H
#define METEO_MAP_PTKPPGEO_H

#include <stdint.h>
#include <stdio.h>

#include <utility>
#include <string>

namespace meteo {
namespace map {

class Document;

namespace ptkppgeo {

bool loadGeoData( Document* doc );

}
}
}

#endif
