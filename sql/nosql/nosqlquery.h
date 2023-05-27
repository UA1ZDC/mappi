#ifndef SQL_NOSQL_NOSQLQUERY_H
#define SQL_NOSQL_NOSQLQUERY_H

#include <stdint.h>

#include <qstring.h>
#include <qstringlist.h>
#include <qglobal.h>
#include <qregexp.h>
#include <qregularexpression.h>

#include <cross-commons/debug/tlog.h>

#include <sql/nosql/libmongoc/libbson/bson/bson.h>
#include <sql/dbi/dbiquery.h>
#include <sql/nosql/document.h>
#include <sql/rapidjson/rapidjson.h>

namespace meteo {

static const QRegularExpression kInvalidDogs("(?<!\")(@.*?@)(?!\")");
static const QRegExp kDogs("@.*@");


class NosqlQuery : public DbiQuery
{
  public:
    NosqlQuery();
    ~NosqlQuery();

    bool exec();
    bool exec( const QString& query );

    bool initIterator();
    bool next();
    const DbiEntry& entry() const { return entry_; }
    const DbiEntry& result() const ;

    int32_t querySize() const ;

    QString errorMessage() const { return error_; }


    DbiQuery& arg( const QString& paramname, bool param );
    DbiQuery& arg( const QString& paramname, int16_t param );
    DbiQuery& arg( const QString& paramname, int32_t param );
    DbiQuery& arg( const QString& paramname, int64_t param );
    DbiQuery& arg( const QString& paramname, uint64_t param );
    DbiQuery& arg( const QString& paramname, float param );
    DbiQuery& arg( const QString& paramname, double param );
    DbiQuery& arg( const QString& paramname, const char* param );
    DbiQuery& arg( const QString& paramname, const QString& param );
    DbiQuery& argWithoutQuotes( const QString& paramname, const QString& param );
    DbiQuery& arg( const QString& paramname, const std::string& param );
    DbiQuery& arg( const QString& paramname, const QDateTime& param );
    DbiQuery& arg( const QString& paramname, const QDate& param );
    DbiQuery& arg( const QString& paramname, const QStringList& param );

    //для массивов
    DbiQuery& arg( const QString& paramname, int16_t param[], int size );
    DbiQuery& arg( const QString& paramname, int32_t param[], int size );
    DbiQuery& arg( const QString& paramname, double param[], int size );
    DbiQuery& arg( const QString& paramname, const google::protobuf::RepeatedPtrField<std::string>& list );
    DbiQuery& arg( const QString& paramname, const google::protobuf::RepeatedField<int32_t>& list );
    DbiQuery& arg( const QString& paramname, const google::protobuf::RepeatedField<long unsigned int>& list );
    DbiQuery& arg( const QString& paramname, const google::protobuf::RepeatedField<long int>& list );
    DbiQuery& arg( const QString& paramname, const google::protobuf::RepeatedField<long long int>& list );
    DbiQuery& arg( const QString& paramname, const QList<int32_t>& list );
    DbiQuery& arg( const QString& paramname, const QList<long unsigned int>& list );
    DbiQuery& arg( const QString& paramname, const QList<long long int>& list );
    DbiQuery& arg( const QString& paramname, const QList<double>& list );
    DbiQuery& arg( const QString& paramname, const google::protobuf::RepeatedField<double>& list );
    DbiQuery& argFieldName(const QString& paramname, const QString& fieldname) override;
    DbiQuery& argIpAddress( const QString& paramname, const QString& param );

    DbiQuery& arg( const QString& paramname, const meteo::GeoPoint& point );
    DbiQuery& arg( const QString& paramname, const meteo::GeoVector& vector );

    DbiQuery& argDt( const QString& paramname, const QString& param );
    DbiQuery& argDt( const QString& paramname, const std::string& param );
    DbiQuery& argDt( const QString& paramname, const QStringList& param );
    DbiQuery& argDt( const QString& paramname, const google::protobuf::RepeatedPtrField<std::string>& list );

    DbiQuery& argOid( const QString& paramname, const QString& param );
    DbiQuery& argOid( const QString& paramname, const std::string& param );
    DbiQuery& argOid( const QString& paramname, const QStringList& list );
    DbiQuery& argOid( const QString& paramname, const google::protobuf::RepeatedPtrField<std::string>& param );

    DbiQuery& argJson( const QString& paramname, const QString& json );
    DbiQuery& argJson( const QString& paramname, const QStringList& jsons );
    DbiQuery& argJsFunc( const QString& paramname, const QString& func );

    DbiQuery& argMd5( const QString& paramname, const QByteArray& md5 );
    DbiQuery& argMd5Base64( const QString& paramname, const QString& md5 );

    DbiQuery& argGeoWithin( const QString& paramname, const meteo::GeoVector& gv );
    DbiQuery& argGeoCircle(const QString& paramname, const meteo::GeoPoint& gv, int radiusmeter );

    DbiQuery& removeArg( const QString& name, bool* fl = nullptr );
    DbiQuery& removeArg( const QString& name, int num, const QString& arrname, bool* fl = nullptr );

    QString escapeString(const QString& originalString);

    static bool isObjectId(const std::string& oid);
    static bool isObjectId(const QString& oid);

    static QString  generateOid();

    void removeDogs();

  private:
    QString error_;
    Document entry_;
    Document docentry_;
    bson_t document_result_;
    const bson_t* cursor_doc_ = nullptr;
    bool hasresult_ = false;

    void removeDogsRecursive( rapidjson::Value* json );

  friend class NoSql;

};

}

#endif
