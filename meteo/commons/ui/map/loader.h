#ifndef METEO_COMMONS_UI_MAP_LOADER_H
#define METEO_COMMONS_UI_MAP_LOADER_H

#include <qmap.h>
#include <qstring.h>
#include <qstringlist.h>

#include <cross-commons/singleton/tsingleton.h>

namespace meteo {
namespace map {

class Map;
class Document;

namespace internal
{

class Loader
{
  typedef bool (*FunctionType)(Document*);
  typedef bool (*FunctionGridType)(Document*);
  public:

    bool registerDataHandler( const QString& id, FunctionType func );
    bool registerGridHandler( const QString& id, FunctionGridType func );
    bool registerStationHandler( const QString& id, FunctionGridType func );
    bool registerCitiesHandler( const QString& id, FunctionGridType func );

    bool handleData( const QString& id, Document* map );
    bool handleGrid( const QString& id, Document* map );
    bool handleStation( const QString& id, Document* map );
    bool handleCities( const QString& id, Document* map );

    QStringList geoLoaders() const ;

  private:
    Loader();
    ~Loader();

    QMap< QString, FunctionType > handlers_;
    QMap< QString, FunctionGridType > gridhandlers_;
    QMap< QString, FunctionGridType > stationhandlers_;
    QMap< QString, FunctionGridType > cityHandlers_;

  friend class TSingleton<internal::Loader>;

};

}

typedef TSingleton<internal::Loader> Loader;

}
}

#endif
