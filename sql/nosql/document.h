#ifndef SQL_NOSQL_DOCUMENT_H
#define SQL_NOSQL_DOCUMENT_H

#include <qstring.h>

#include <sql/dbi/dbientry.h>
#include <sql/nosql/libmongoc/libbson/bson/bson.h>
#include <sql/nosql/array.h>
#include <commons/geobasis/geopoint.h>
#include <commons/geobasis/geovector.h>

#include <sql/rapidjson/document.h>

namespace meteo {

class Document : public DbiEntry
{
  public:
    Document();
    Document( const Document& doc );
    Document( bson_t* bson );
    Document( const bson_t* bson );
    Document( bson_iter_t* iterator );
    Document( const QString& json );
    Document( const rapidjson::Document& doc );

    ~Document();

    void setDocument( bson_t* doc );
    void setDocument( const bson_t* doc );

    int32_t valueInt32( const QString& field, bool* res = nullptr ) const;
    int64_t valueInt64( const QString& field, bool* res = nullptr ) const;
    double valueDouble( const QString& field, bool* res = nullptr ) const;
    QString valueString( const QString& field, bool* res = nullptr ) const;
    meteo::GeoPoint valueGeo( const QString& field, bool* res = nullptr ) const;
    QList<meteo::GeoVector> valueGeoVector( const QString& field, bool* res = nullptr ) const;
    QDateTime valueDt( const QString& field, bool* res = nullptr ) const;
    QString valueOid( const QString& field, bool* ok = nullptr ) const;
    const Document& valueDocument( const QString& field, bool* res = nullptr ) const;
    bool valueBool( const QString& field, bool* res = nullptr ) const;
    const Array& valueArray( const QString& field, bool* ok = nullptr ) const;

    bool hasField( const QString& field ) const ;
    bson_type_t typeField( const QString& field ) const ;
    void removeField( const QString& field );

    bool isEmpty() const ;

    QString jsonString() const ;
    QString jsonExtendedString() const ;

    QString jsonPostgresql() const ;

    rapidjson::Document jsonRapid() const ;

    Document& operator=( const Document& doc );

  public:
    static bool find_doc_field( const QString& field, bson_iter_t* iter, const bson_t* doc );
    static QString rapid2string( const rapidjson::Document& doc );

  private:
    bson_t* own_ = nullptr;
    const bson_t* alien_ = nullptr;
    int32_t* refcnt_ = nullptr;
    mutable Document* locdoc_ = nullptr;
    mutable Array locarr_;
};

}

#endif
