#ifndef SQL_DBI_DBIENTRY_H
#define SQL_DBI_DBIENTRY_H

#include <qstring.h>

#include <sql/nosql/libmongoc/libbson/bson/bson.h>
#include <commons/geobasis/geopoint.h>
#include <commons/geobasis/geovector.h>

namespace meteo {

class Document;
class Array;

class DbiEntry
{
  public:
    virtual ~DbiEntry();

    virtual int32_t valueInt32( const QString& field, bool* res = nullptr ) const = 0;
    virtual int64_t valueInt64( const QString& field, bool* res = nullptr ) const = 0;
    virtual double valueDouble( const QString& field, bool* res = nullptr ) const = 0;
    virtual QString valueString( const QString& field, bool* res = nullptr ) const = 0;
    virtual meteo::GeoPoint valueGeo( const QString& field, bool* res = nullptr ) const = 0;
    virtual QList<meteo::GeoVector> valueGeoVector( const QString& field, bool* res = nullptr ) const = 0;
    virtual QDateTime valueDt( const QString& field, bool* res = nullptr ) const = 0;
    virtual QString valueOid( const QString& field, bool* ok = nullptr ) const = 0;
    virtual const Document& valueDocument( const QString& field, bool* res = nullptr ) const = 0;
    virtual bool valueBool( const QString& field, bool* res = nullptr ) const = 0;
    virtual const Array& valueArray( const QString& field, bool* ok = nullptr ) const = 0;

    virtual bool hasField( const QString& field ) const = 0;
    virtual bson_type_t typeField( const QString& field ) const = 0;
    virtual void removeField( const QString& field ) = 0;

    virtual QString jsonExtendedString() const = 0;
};

}

#endif
