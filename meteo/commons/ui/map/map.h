#ifndef METEO_COMMONS_UI_MAP_MAP_H
#define METEO_COMMONS_UI_MAP_MAP_H

#include <stdlib.h>

#include <qhash.h>
#include <qstring.h>

#include <cross-commons/includes/tcommoninclude.h>
#include <cross-commons/singleton/tsingleton.h>
#include <commons/geobasis/projection.h>
#include <meteo/commons/proto/map_document.pb.h>

class QTransform;

namespace meteo {

namespace map {

class Document;
class Ramka;
class GeoPolygon;
class Layer;

namespace proto {
class Map;
}

bool loadGeoLayers( Document* doc );

}
}

#endif
