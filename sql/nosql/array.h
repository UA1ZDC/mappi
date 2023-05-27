#ifndef SQL_NOSQL_ARRAY_H
#define SQL_NOSQL_ARRAY_H

#include <commons/geobasis/geopoint.h>
#include <sql/nosql/libmongoc/libbson/bson/bson.h>
#include <sql/rapidjson/document.h>

namespace meteo {

class Document;

class Array
{
  public:
    Array();
    Array( const QString& json );
    Array( bson_t* array );
    Array( const Array& array );
    Array( const rapidjson::Document& doc );
    ~Array();

    Array& operator=( const Array& array );

    bool initIterator();
    bool next();

    bool isEmpty() const ;

    void setArray( bson_t* doc );

    int32_t valueInt32( bool* res = nullptr );
    int64_t valueInt64( bool* res = nullptr );
    double valueDouble( bool* res = nullptr );
    QString valueString( bool* res = nullptr );
    meteo::GeoPoint valueGeo( bool* res = nullptr );
    QDateTime valueDt( bool* res = nullptr );
    QString valueOid( bool* ok = nullptr );
    bool valueDocument( Document* doc );
    bool valueArray( Array* doc );
    bool valueBool( bool* res = nullptr );

    QString jsonString() const ;
    QString jsonExtendedString() const ;
    QString jsonPostgresql() const ;

    rapidjson::Document jsonRapid() const ;

    static QString postgresqlValue( bson_iter_t* it );

  private:
    bson_t* array_ = nullptr;
    bson_iter_t iterator_;
    int32_t* refcnt_ = nullptr;
};
}

#endif
