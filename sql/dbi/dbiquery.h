#ifndef SQL_DBI_DBIQUERY_H
#define SQL_DBI_DBIQUERY_H

#include <google/protobuf/repeated_field.h>

#include <qstring.h>
#include <qdatetime.h>
#include <qmap.h>
#include <qlist.h>
#include <qregularexpression.h>

#include <sql/rapidjson/document.h>
#include <sql/nosql/libmongoc/libbson/bson/bson.h>
#include <commons/geobasis/geopoint.h>
#include <commons/geobasis/geovector.h>

namespace meteo {

class Dbi;
class DbiEntry;
class DbiArray;

class DbiQuery
{
  public:
    DbiQuery( );
    DbiQuery( Dbi* db );
    DbiQuery( const QString& query, Dbi* db );
    virtual ~DbiQuery();

    virtual bool exec();
    virtual bool exec( const QString& query );
    bool execInit( QString* error );

    Dbi* db() const { return db_; }

    const QString& query() const { return query_; }
    DbiQuery& setQuery( const QString& q );

    virtual DbiQuery& arg( const QString& paramname, bool param ) = 0;
    virtual DbiQuery& arg( const QString& paramname, int16_t param ) = 0;
    virtual DbiQuery& arg( const QString& paramname, int32_t param ) = 0;
    virtual DbiQuery& arg( const QString& paramname, int64_t param ) = 0;
    virtual DbiQuery& arg( const QString& paramname, uint64_t param ) = 0;
    virtual DbiQuery& arg( const QString& paramname, float param ) = 0;
    virtual DbiQuery& arg( const QString& paramname, double param ) = 0;
    virtual DbiQuery& arg( const QString& paramname, const char* param ) = 0;
    virtual DbiQuery& arg( const QString& paramname, const QString& param ) = 0;
    virtual DbiQuery& argWithoutQuotes( const QString& paramname, const QString& param ) = 0;
    virtual DbiQuery& arg( const QString& paramname, const std::string& param ) = 0;
    virtual DbiQuery& arg( const QString& paramname, const QDateTime& param ) = 0;
    virtual DbiQuery& arg( const QString& paramname, const QDate& param ) = 0;
    virtual DbiQuery& arg( const QString& paramname, const QStringList& param ) = 0;
    virtual DbiQuery& argFieldName(const QString& paramname, const QString& fieldname) = 0;
    virtual DbiQuery& argIpAddress( const QString& paramname, const QString& param ) = 0;

    virtual DbiQuery& arg( const QString& paramname, int16_t param[], int size ) = 0;
    virtual DbiQuery& arg( const QString& paramname, int32_t param[], int size ) = 0;
    virtual DbiQuery& arg( const QString& paramname, double param[], int size ) = 0;
    virtual DbiQuery& arg( const QString& paramname, const google::protobuf::RepeatedPtrField<std::string>& list ) = 0;
    virtual DbiQuery& arg( const QString& paramname, const QList<int32_t>& list ) = 0;
    virtual DbiQuery& arg( const QString& paramname, const QList<long unsigned int>& list ) = 0;
    virtual DbiQuery& arg( const QString& paramname, const QList<long long int>& list ) = 0;
    virtual DbiQuery& arg( const QString& paramname, const QList<double>& list ) = 0;
    virtual DbiQuery& arg( const QString& paramname, const google::protobuf::RepeatedField<int32_t>& list ) = 0;
    virtual DbiQuery& arg( const QString& paramname, const google::protobuf::RepeatedField<long unsigned int>& list ) = 0;
    virtual DbiQuery& arg( const QString& paramname, const google::protobuf::RepeatedField<long int>& list ) = 0;
    virtual DbiQuery& arg( const QString& paramname, const google::protobuf::RepeatedField<long long int>& list ) = 0;
    virtual DbiQuery& arg( const QString& paramname, const google::protobuf::RepeatedField<double>& list ) = 0;

    virtual DbiQuery& arg( const QString& paramname, const meteo::GeoPoint& point ) = 0;
    virtual DbiQuery& arg( const QString& paramname, const meteo::GeoVector& vector ) = 0;

    virtual DbiQuery& argDt( const QString& paramname, const QString& param ) = 0;
    virtual DbiQuery& argDt( const QString& paramname, const std::string& param ) = 0;
    virtual DbiQuery& argDt( const QString& paramname, const QStringList& param ) = 0;
    virtual DbiQuery& argDt( const QString& paramname, const google::protobuf::RepeatedPtrField<std::string>& list ) = 0;

    virtual DbiQuery& argOid( const QString& paramname, const QString& param ) = 0;
    virtual DbiQuery& argOid( const QString& paramname, const std::string& param ) = 0;
    virtual DbiQuery& argOid( const QString& paramname, const QStringList& list ) = 0;
    virtual DbiQuery& argOid( const QString& paramname, const google::protobuf::RepeatedPtrField<std::string>& param ) = 0;

    virtual DbiQuery& argJson( const QString& paramname, const QString& json ) = 0;
    virtual DbiQuery& argJson( const QString& paramname, const QStringList& jsons ) = 0;
    virtual DbiQuery& argJsFunc( const QString& paramname, const QString& func ) = 0;

    virtual DbiQuery& argMd5( const QString& paramname, const QByteArray& md5 ) = 0;
    virtual DbiQuery& argMd5Base64( const QString& paramname, const QString& md5 ) = 0;

    virtual DbiQuery& argGeoWithin( const QString& paramname, const meteo::GeoVector& gv ) = 0;
    virtual DbiQuery& argGeoCircle(const QString& paramname, const meteo::GeoPoint& gv, int radiusmeter )=0;

    virtual DbiQuery& removeArg( const QString& argname ) { Q_UNUSED(argname); return *this; }

    const QString& getNULL() const { return null_; }

    virtual QString errorMessage() const = 0;

    virtual bool initIterator() = 0;
    virtual bool next() = 0;
    virtual const DbiEntry& entry() const = 0;
    virtual const DbiEntry& result() const = 0;

    virtual QString escapeString(const QString& originalString) = 0;

    QString fullname( const QString& paramname ) const ;

    static QDateTime datetimeFromString( const QString& str );
    static QDateTime datetimeFromString( const std::string& str );

    static QDate dateFromString( const QString& str );
    static QDate dateFromString( const std::string& str );

    static QString geopoint2json( const meteo::GeoPoint& gp );
    static meteo::GeoPoint json2geopoint( const QString& json, bool* ok = 0 );

    static QString geovector2json( const meteo::GeoVector& gv );
    static QList<meteo::GeoVector> json2geovector( const QString& json, bool* ok = 0);

    static bool find_doc_field( const QString& field, bson_iter_t* iter, const bson_t* doc );

  protected:
    QString query_;
    Dbi* db_;
    QString null_;
    QRegularExpression rx_ = QRegularExpression("@.*?@");
    QMap< QString, int > uninstalledargs_;

  protected:
    static rapidjson::Value& jsonValue( rapidjson::Value* doc, const QString& fieldname );

  private:
    void parseUninstalledArgsInQueryString();
};

}

#endif
