#ifndef SQL_PSQL_PSQLENTRY_H
#define SQL_PSQL_PSQLENTRY_H

#include <sql/dbi/dbientry.h>
#include <sql/nosql/document.h>

class pg_result;

namespace meteo {

class PsqlQuery;

class PsqlEntry : public DbiEntry
{
  public:
    PsqlEntry( PsqlQuery* query );
    PsqlEntry( const QHash<QString, QString>& values );
    PsqlEntry();
    PsqlEntry( const PsqlEntry& entry );
    ~PsqlEntry();

    int32_t valueInt32( const QString& field, bool* res = nullptr ) const;
    int64_t valueInt64( const QString& field, bool* res = nullptr ) const;
    double valueDouble( const QString& field, bool* res = nullptr ) const;
    QString valueString( const QString& field, bool* res = nullptr ) const;
    meteo::GeoPoint valueGeo( const QString& field, bool* res = nullptr ) const;
    QList<meteo::GeoVector> valueGeoVector( const QString& field, bool* res = nullptr ) const;
    QDateTime valueDt( const QString& field, bool* res = nullptr ) const;
    QString valueOid( const QString& field, bool* res = nullptr ) const;
    bool valueBool( const QString& field, bool* res = nullptr ) const;
    const Document& valueDocument( const QString& field, bool* res = nullptr ) const;
    const Array& valueArray( const QString& field, bool* res = nullptr ) const;

    bool hasField( const QString& field ) const;
    bson_type_t typeField( const QString& field ) const;
    void removeField( const QString& field );

    QString jsonExtendedString() const ;

  private:
    pg_result* result_ = nullptr;
    int rowindex_ = -1;
    QHash< QString, QString > values_;
    mutable Document locdoc_;
    mutable Array locarr_;
};

};

#endif
