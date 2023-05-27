#ifndef METEO_COMMONS_SERVICES_INDEXES_INDEXESSAVER_H
#define METEO_COMMONS_SERVICES_INDEXES_INDEXESSAVER_H

#include <qdatetime.h>
#include <qmap.h>
#include <meteo/commons/proto/sprinf.pb.h>

class TMeteoData;

namespace zond {
  class Zond;
}

namespace meteo {

namespace surf {
class MeteoDataProto;
}

  class IndexesSaver {
  public:
    IndexesSaver();
    ~IndexesSaver();
    
    bool calcVolatileIndexes(const QDateTime& dt);
    bool calcHydroLevels(const QDateTime& dt);
    
  private:

    bool getAero(const std::string& station, const QDateTime& dt, zond::Zond* zond);
    bool calcIndexes(const zond::Zond& zond, TMeteoData* md);
    bool saveIndexes(TMeteoData* md, int categ, const QDateTime& dt);

    bool getStationsWithHydroLevels();

    QMap<int, sprinf::Station> stations_;
//    sprinf::Stations stations_;
    
  };
}

#endif
