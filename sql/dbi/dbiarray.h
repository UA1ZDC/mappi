#ifndef SQL_DBI_DBIARRAY_H
#define SQL_DBI_DBIARRAY_H

#include <commons/geobasis/geopoint.h>
#include <sql/nosql/libmongoc/libbson/bson/bson.h>

namespace meteo {

class DbiEntry;

class DbiArray
{
  public:
    virtual ~DbiArray();

    bool initIterator();
    bool next();

    virtual int32_t valueInt32( bool* res = nullptr ) = 0;
    virtual int64_t valueInt64( bool* res = nullptr ) = 0;
    virtual double valueDouble( bool* res = nullptr ) = 0;
    virtual QString valueString( bool* res = nullptr ) = 0;
    virtual meteo::GeoPoint valueGeo( bool* res = nullptr ) = 0;
    virtual QDateTime valueDt( bool* res = nullptr ) = 0;
    virtual QString valueOid( bool* ok = nullptr ) = 0;
    virtual bool valueDocument( DbiEntry* doc ) = 0;
    virtual bool valueBool( bool* res = nullptr ) = 0;
};

}

#endif
