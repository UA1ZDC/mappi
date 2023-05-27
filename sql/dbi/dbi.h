#ifndef SQL_DBI_DBI_H
#define SQL_DBI_DBI_H

#include <memory>

#include <qobject.h>
#include <qmap.h>

#include <sql/psql/tpgsettings.h>

namespace meteo {
namespace global {

QMap< QString, QString > loadMongoQueriesNew();

static const QMap< QString, QString > kMongoQueriesNew = loadMongoQueriesNew();

QMap< QString, QString> loadSqlQueries();

static const QMap< QString, QString > kSqlQueries = loadSqlQueries();

}


class DbiQuery;

class Dbi : public QObject
{
  Q_OBJECT
  public:
    Dbi( const QString& host, uint16_t port, const QString& database, const QString& login = QString(), const QString& pass = QString() );
    Dbi( const ConnectProp& params );
    Dbi();
    virtual ~Dbi();

    virtual void reset() = 0;

    virtual bool connect() = 0;
    virtual void disconnect() = 0;
    virtual bool connected() const = 0;
    virtual bool execQuery( const QString& query ) = 0;
    virtual bool execQuery( DbiQuery* query ) = 0;

    virtual DbiQuery* query( const QString& q = QString() ) = 0;
    virtual std::unique_ptr<DbiQuery> queryptr( const QString& q = QString() ) = 0;

    virtual DbiQuery* queryByName( const QString& q ) = 0;
    virtual std::unique_ptr<DbiQuery> queryptrByName( const QString& q ) = 0;


    bool connect( const QString& host, uint16_t port, const QString& database, const QString& login = QString(), const QString& pass = QString() );
    bool connect( const ConnectProp& params );

    const ConnectProp& params() const { return params_; }
    const QString& hostname() const { return params_.host(); }
    const QString& dbname() const { return params_.name(); }
    uint16_t port() const { return params_.port(); }
    const QString& login() const { return params_.login(); }
    const QString& pass() const { return params_.pass(); }
    const QString& userdb() const { return params_.userDb(); }

    void setParams( const ConnectProp& params ) { params_ = params; reset(); }
    void setHostname( const QString& host ) { params_.setHost(host); reset(); }
    void setDbname( const QString& dbname ) { params_.setName(dbname); reset(); }
    void setPort( uint16_t port ) { params_.setPort(port); reset(); }
    void setLogin( const QString& login ) { params_.setLogin(login); reset(); }
    void setPass( const QString& pass ) { params_.setPass(pass); reset(); }
    void setUserdb( const QString& userdb ) { params_.setUserDb(userdb); reset(); }

  protected:
    ConnectProp params_;
};

}

#endif
