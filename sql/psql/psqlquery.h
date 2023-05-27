#ifndef SQL_PSQL_PSQLQUERY_H
#define SQL_PSQL_PSQLQUERY_H

#include <stdint.h>

#include <qstring.h>
#include <qstringlist.h>
#include <qglobal.h>

#include <cross-commons/debug/tlog.h>
#include <commons/geobasis/geopoint.h>

#include <sql/dbi/dbiquery.h>
#include <sql/psql/psql.h>
#include <sql/psql/psqlentry.h>

class pg_result;

namespace meteo {

class Psql;
class Document;
class Array;

class PsqlQuery : public DbiQuery
{
  public:
    PsqlQuery( Psql* db );
    PsqlQuery();
    PsqlQuery( const QString& query, Psql* db );
    ~PsqlQuery();

    bool exec();
    bool exec( const QString& query );

    bool initIterator();
    bool next();
    const DbiEntry& entry() const { return entry_; }
    const DbiEntry& result() const ;

    DbiQuery& argOid(const QString& paramname, const QString& param );
    DbiQuery& argOid(const QString& paramname, const std::string& param );
    DbiQuery& argOid(const QString& paramname, const QStringList& list );
    DbiQuery& argOid(const QString& paramname,
                     const google::protobuf::RepeatedPtrField<std::string>& param );

    DbiQuery& argJsFunc(const QString& paramname, const QString& func );
    DbiQuery& argGeoWithin(const QString& paramname, const meteo::GeoVector& gv );

    DbiQuery& argGeoCircle(const QString& paramname, const meteo::GeoPoint& gv, int radiusmeter );

    DbiQuery& argMd5(const QString& paramname, const QByteArray& md5 ); //!FIXME поменять формат jsonb на более подходящий
    DbiQuery& argMd5Base64(const QString& paramname, const QString& md5 ); //!FIXME



    //Набор функций для подстановки значений параметров с указанием имени аргумента
    DbiQuery& arg( const QString& paramname, bool param );
    DbiQuery& arg( const QString& paramname, int16_t param );
    DbiQuery& arg( const QString& paramname, int32_t param );
    DbiQuery& arg( const QString& paramname, int64_t param );
    DbiQuery& arg( const QString& paramname, uint64_t param ); //WARN хранятся как !!!!BIGINT!!!!
    DbiQuery& arg( const QString& paramname, float param );
    DbiQuery& arg( const QString& paramname, double param );
    DbiQuery& arg( const QString& paramname, const char* param );
    DbiQuery& arg( const QString& paramname, const QString& param );
    DbiQuery& argWithoutQuotes( const QString& paramname, const QString& param );
    DbiQuery& arg( const QString& paramname, const QDateTime& param );
    DbiQuery& arg( const QString& paramname, const QDate& param );
    DbiQuery& arg( const QString& paramname, const QStringList& param );
    DbiQuery& arg( const QString& paramname, int16_t param[], int size );
    DbiQuery& arg( const QString& paramname, int32_t param[], int size );
    DbiQuery& arg( const QString& paramname, double param[], int size );
    DbiQuery& arg( const QString& paramname, const GeoPoint& gp );
    DbiQuery& arg( const QString& paramname, const Document& param );
    DbiQuery& arg( const QString& paramname, const Array& param );
    DbiQuery& arg( const QString& paramname, const QList<Document>& param );
    DbiQuery& arg(const QString& paramname, const meteo::GeoVector& vector );
    DbiQuery& argFieldName(const QString& paramna, const QString& fieldName);
    DbiQuery& argIpAddress(const QString& paramna, const QString& ip);

    DbiQuery& arg(const QString& paramname,
                  const google::protobuf::RepeatedField<double>& list );
    DbiQuery& arg(const QString& paramname,
                  const google::protobuf::RepeatedField<int32_t>& list );
    DbiQuery& arg(const QString& paramname,
                  const google::protobuf::RepeatedField<long unsigned int>& list );
    DbiQuery& arg(const QString& paramname,
                  const google::protobuf::RepeatedField<long int>& list );
    DbiQuery& arg(const QString& paramname,
                  const google::protobuf::RepeatedField<long long int>& list );
    DbiQuery& arg(const QString& paramname,
                  const QList<int32_t>& list );
    DbiQuery& arg(const QString& paramname,
                  const QList<long unsigned int>& list );
    DbiQuery& arg(const QString& paramname,
                  const QList<long long int>& list );
    DbiQuery& arg(const QString& paramname,
                  const QList<double>& list );
    DbiQuery& arg(const QString& paramname, const std::string& param );
    DbiQuery& arg(const QString& paramname,
                  const google::protobuf::RepeatedPtrField<std::string>& list );

    DbiQuery& argDt(const QString& paramname, const QString& param );
    DbiQuery& argDt(const QString& paramname, const std::string& param );
    DbiQuery& argDt(const QString& paramname, const QStringList& param );
    DbiQuery& argDt(const QString& paramname,
                    const google::protobuf::RepeatedPtrField<std::string>& list );

    DbiQuery& argJson( const QString& paramname, const QString& param );
    DbiQuery& argJson( const QString& paramname, const QStringList& params );

    QString errorMessage() const ;
    QString escapeString( const QString& str );

  private:
    pg_result* res_;

    int size_;
    Psql* psql_ = nullptr;

    QStringList columns_;
    int currentrow_ = -1;
    PsqlEntry entry_;
    PsqlEntry docresult_;

    friend class Psql;
    friend class PsqlEntry;
};

}

#endif
