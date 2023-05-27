#ifndef TESTDB_H
#define TESTDB_H

#include <qobject.h>

#include <libpq-fe.h>
#include <cross-commons/includes/tcommoninclude.h>


namespace meteo {
class Psql;
}

class TestDb : public QObject {
  Q_OBJECT
  public:
    TestDb( QObject* parent = 0 );
    ~TestDb();

    bool connectToDb( const QString& host, const QString& name,
       const QString& login, const QString& passwd );

  private:
    TTimer* timer_;
    meteo::Psql* base_;

  private slots:
    void slotConnected();
    void slotFailed();
    void slotStatusChanged( ConnStatusType type );
    void slotTimeout();
};

#endif
