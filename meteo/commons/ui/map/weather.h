#ifndef METEO_COMMONS_UI_MAP_WEATHER_H
#define METEO_COMMONS_UI_MAP_WEATHER_H

#include <qstring.h>
#include <qmap.h>
#include <qpair.h>
#include <qdatetime.h>

#include <sql/nosql/document.h>

#include <meteo/commons/proto/puanson.pb.h>
#include <meteo/commons/proto/meteo.pb.h>
#include <meteo/commons/proto/map_isoline.pb.h>
#include <meteo/commons/proto/field.pb.h>
#include <meteo/commons/proto/weather.pb.h>

class QDateTime;

namespace rpc {
  class Channel;
}

namespace zond {
  class PlaceData;
}

namespace meteo {
namespace sprinf {
class StationFullInfo;
}
namespace map {

class VProfileDoc;
class Document;
class Layer;
class AeroDocument;
class FormalDoc;

class Weather
{
  public:
    Weather();
    ~Weather();

    proto::MapList getAvailableMaps( const proto::Map& map, const QDateTime& dtbeg, const QDateTime& dtend );
    proto::MapList getAvailableDocuments( const proto::Map& map, const QDateTime& dtbeg, const QDateTime& dtend );

    bool createMap( Document* doc, const proto::Map& info );

    Layer* buildLayer( Document* doc, const proto::WeatherLayer& info, int spline_koef );
    Layer* buildPuansonLayer( Document* doc, const proto::WeatherLayer& info );
    Layer* buildIsoLayer( Document* doc, const proto::WeatherLayer& info, int spline_koef );
    Layer* buildIsoLayer( Document* doc, const proto::WeatherLayer& info, int spline_koef, const proto::FieldColor& clr );
    Layer* buildMrlLayer( Document* doc, const proto::WeatherLayer& info );
    Layer* buildSigwxLayer( Document* doc, const proto::WeatherLayer& info );
    Layer* buildEmptyLayer( Document* doc, const proto::WeatherLayer& info );
    bool   buildAD( AeroDocument* doc, const proto::Map& map, QString *error = nullptr ) const;
    bool   buildVProfile(VProfileDoc *doc, const proto::Map& map ) const;
    bool   buildFormalDocument(FormalDoc* doc, const proto::Map& map);

    static QString keyFromParameters( const QDateTime& dt,
                                      const QString& datakey,
                                      int center,
                                      int model,
                                      int level,
                                      int leveltype,
                                      int hour );

    static bool parametersFromKey( const QString& key,
                                   QDateTime* dt,
                                   QString* datakey,
                                   int* center,
                                   int* model,
                                   int* level,
                                   int* leveltype,
                                   int* hour );

    static proto::DataSource sourceFromViewModeAndSource( proto::DataSource source, proto::ViewMode mode );

    static surf::SigwxDesc layer2sigwx( const proto::WeatherLayer& info  );
    static proto::WeatherLayer sigwx2layer( const surf::SigwxDesc& sigwx );

    /*!
     * \brief Создать виртуальный документ из proto-структуры doc
     * \return Указатель на созданный документ
     */
    static Document* documentFromProto( const proto::Document* doc );


  private:
    QMap<QString, proto::FieldColor> isoparams_;
    QMap<QString, puanson::proto::Puanson> punchparams_;

    proto::Map mapFromQuery(const proto::Map& map, const DbiEntry &doc );

    meteo::surf::DataReply* sendGribDataRequest( const proto::WeatherLayer& info );
    meteo::surf::DataReply* sendSurfaceDataRequest( const proto::WeatherLayer& info );
    meteo::surf::DataReply* sendFieldDataRequest( const proto::WeatherLayer& info );

    void setLayerFromMap( const proto::Map& info, proto::WeatherLayer* layer );
    bool buildADPoSurf(zond::PlaceData *place, const proto::Map& map , QString *error) const;
    bool buildADPoField(zond::PlaceData *place, const proto::Map& map , QString *error) const;
    bool buildADPoSat(zond::PlaceData *place, const proto::Map& map , QString *error) const;
    bool createAd( Document* doc, const proto::Map& info )const ;
    bool createVC( Document* doc, const proto::Map& info )const;

};

}
}

#endif
