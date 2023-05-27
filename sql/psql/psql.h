#ifndef SQL_PSQL_PSQL_H
#define SQL_PSQL_PSQL_H

#include <postgres_ext.h>

#include <qmutex.h>
#include <qobject.h>
#include <qstringlist.h>

#include <cross-commons/includes/tcommoninclude.h>
#include <cross-commons/debug/tmap.h>

#include <sql/dbi/dbi.h>
#include <sql/psql/tpgsettings.h>


class QDateTime;
class QSocketNotifier;

class pg_conn;
class pg_result;

namespace meteo {

class PsqlQuery;

// Class for getting data from PGresult *
class PsqlResult
{
  public:
    PsqlResult()     { Empty = true; Data = 0; }
    bool isEmpty()    { return Empty; }
    bool setData( const char* data );
    void clear()      {Empty = true; Data = 0; }

    double asDouble();
    bool asBool();
    int	asInt();
    const char* asString();
    bool asDateTime( QDateTime* dt );

  private:
    const char* Data;
    bool Empty;
};

// Class for connecting to database with libpq
class Psql : public Dbi
{
  Q_OBJECT
  public:
    enum ConnectionType {
      BLOCKED_CONNECTION = 0,
      NONBLOCKED_CONNECTION = 1
    };

    enum FieldFormat {
      TEXT = 0,
      BINARY = 1
    };

    Psql( const QString& host, uint16_t port, const QString& db, const QString& login = QString(), const QString& pass = QString() );
    Psql( const ConnectProp& prop );
    Psql();
    ~Psql();

    bool connect();
    void disconnect();
    void reset();


    bool connectNonBlock();

    bool execQuery( const QString& query );

    DbiQuery* query( const QString& q = QString() );
    std::unique_ptr<DbiQuery> queryptr( const QString& q = QString() );

    DbiQuery* queryByName( const QString& q );
    std::unique_ptr<DbiQuery> queryptrByName( const QString& q );


    bool execSQL( const QString& query ); // query in transaction

    bool connected() const ;

    int recordCount() const ;
    int columnCount() const ;

    bool startTransaction();
    bool commit();
    bool rollback();

    bool addNotify(const QString& noti_name );
    void reconnectNotify();

    void sendQuery(const QString& query);
    void cancelQuery();

    const char* errorMessage(); //возвращает строку ошибки is absent in new

    PsqlResult* getCell( int row, int column);
    PsqlResult* getCell( int row, const char* column );
    PsqlResult* getNameCell(int);

    QString getValue( int row, int column, bool* ok = 0 ) const ;
    QString getValue( int row, const QString& column, bool* ok = 0 ) const ;

    int getColumnNum( const char* column ) const ;
    FieldFormat getColumnFormat( int column ) const;

    inline pg_result * getResult() const  { return pgResult_; }

    Oid insertDataIntoLO( void* data, int size );
    Oid insertDataIntoLO(const char* data, int data_size);
    int readDataFromLO( void* data, int size, Oid obj_id );
    int writeDataIntoLO( void* data, int size, Oid obj_id);
    int removeLO(Oid); //не включено в new

  public:
    static QString dt2string( const QDateTime& dt, bool* ok = 0 );
    static QDateTime string2dt( const QString& string, bool* ok = 0 );
    static const QString& dtFormat();

    static Dbi* createDatabase( const ConnectProp& params );

  public slots:
    void sqlNotify(int d);

  protected:
    bool execQuery( DbiQuery* query );

  private slots:
    void slotTimeout();
    void slotGetResult();

  signals:
    void notifyActivated( const char* name );
    void notifyActivated( const char* name, const char* extra );
    void connectionNonBlockFailed();
    void connectionNonBlockSucces();
    void connectionStatusChanged( int constatus );
    void queryDone();
    void queryError(const QString& error);

  private:
    void abort(const char*);


  private:
    mutable QMutex mutex;

    PsqlResult res_;
    pg_conn* connection_;
    pg_result* pgResult_;
    QSocketNotifier* sockNotif_;
    TTimer* timer_;
    TTimer* busy_timer_;
    int connStatus_;

    QStringList _notifyList;    //!< список notify, на которые подписан класса
    QString query_;
    bool conn_busy_;

    friend class PsqlQuery;
};

}

#endif
