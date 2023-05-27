#ifndef METEO_COMMONS_GLOBAL_WEATHERLOADER_H
#define METEO_COMMONS_GLOBAL_WEATHERLOADER_H

#include <qmap.h>
#include <qstring.h>

#include <cross-commons/singleton/tsingleton.h>
#include <meteo/commons/proto/weather.pb.h>
#include <meteo/commons/proto/map_isoline.pb.h>
#include <meteo/commons/proto/map_radar.pb.h>
#include <meteo/commons/proto/puanson.pb.h>
#include <meteo/commons/proto/map_ornament.pb.h>

namespace meteo {
namespace map {
namespace internal {

class WeatherLoader
{
  private:
    WeatherLoader();
    ~WeatherLoader();

  public:

    bool reloadTypes();

    void addPunchToLibrary( const meteo::puanson::proto::Puanson& punch );
    void rmPunchFromLibrary( const meteo::puanson::proto::Puanson& punch );
    const QMap< QString, meteo::puanson::proto::Puanson >& punchlibrary() const { return punchlibrary_; }
    const QMap< QString, meteo::puanson::proto::Puanson >& punchlibraryspecial() const { return punchlibraryspecial_; }

    const QMap<QString, meteo::map::proto::FieldColor>& isolibrary() const { return  isolinelibrary_; }
    const QMap<QString, meteo::map::proto::RadarColor>& radarlibrary() const { return radarlibrary_; }

    const QMap<QString, proto::Ornament>& ornamentlibrary() const { return ornamentlibrary_; }

    const QMap< QString, proto::Map >& punchmaps() const { return punchmaps_; }

    const QMap< QString, proto::Map >& weathermaps() const { return weathermaps_; }
    const QMap< QString, proto::Map >& formals() const { return formals_; }

    const QMap< QString, proto::Job >& mapjobs() const { return mapjobs_; }
    const QMap< QString, proto::Job >& docjobs() const { return docjobs_; }

    void addJobToLibrary( const meteo::map::proto::Job& job );
    void rmJobFromLibrary( const meteo::map::proto::Job& job );
    bool updateJobLibrary( const meteo::map::proto::JobList& jobs );

    void addMapToLibrary( const proto::Map& map );
    void rmMapFromLibrary( const proto::Map& map );

    meteo::map::proto::FieldColor isoparams( const QString& templatename, bool* ok = 0 ) const ;
    meteo::map::proto::RadarColor radarparams( const QString& templatename, bool* ok = 0 ) const ;
    meteo::puanson::proto::Puanson punchparams( const QString& templatename, bool* ok = 0 ) const ;

    void loadPunchLibrary();


  private:
    QMap< QString, proto::Map > types_;
    QMap<QString, meteo::puanson::proto::Puanson> punchlibrary_;
    QMap<QString, meteo::puanson::proto::Puanson> punchlibraryspecial_;
    QMap<QString, meteo::map::proto::FieldColor> isolinelibrary_;
    QMap<QString, meteo::map::proto::RadarColor> radarlibrary_;
    QMap< QString, proto::Map > punchmaps_;
    QMap< QString, proto::Map > weathermaps_;
    QMap< QString, proto::Map > formals_;
    QMap< QString, proto::Ornament > ornamentlibrary_;
    QMap< QString, proto::Job > mapjobs_;
    QMap< QString, proto::Job > docjobs_;

    meteo::map::proto::FieldColor emptyiso_;
    meteo::map::proto::RadarColor emptyradar_;
    meteo::puanson::proto::Puanson emptypunch_;

    void loadIsolineLibrary();
    void loadRadarLibrary();
    void loadPuncmaps();
    void loadWeathermaps();
    void loadFormals();
    void loadOrnamentLibrary();
    void loadMapJobs();
    void loadDocJobs();

  friend class TSingleton<internal::WeatherLoader>;
};

}

typedef TSingleton<internal::WeatherLoader> WeatherLoader;

}
}

#endif
