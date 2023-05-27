#ifndef METEO_COMMONS_UI_MAP_LOADER_SXF_H
#define METEO_COMMONS_UI_MAP_LOADER_SXF_H

namespace meteo {
  namespace map {

    class Document;
    
    namespace sxf {
      
      bool loadGeoData( Document* map );
      
    }
  }
}

#endif
