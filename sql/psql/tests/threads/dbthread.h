#ifndef SQL_NSPGBASE_TEST_THREADS_DBTHREAD_H
#define SQL_NSPGBASE_TEST_THREADS_DBTHREAD_H

#include <qthread.h>

#include <cross-commons/debug/tlog.h>
#include <sql/psql/psql.h>

#include <sql/psql/psqlquery.h>


class DbThread : public QThread
{
  Q_OBJECT
public:
  DbThread(NS_PGBase* db, const ConnectProp& prop, const QString& uuid) : db_(db)
  {
    uuid_ = uuid;

    if ( 0 == db ) {
      db_ = new NS_PGBase;
    }

    if ( !db_->Connect(prop) ) {
      error_log << QObject::tr("Ошибка при подключении к БД.");
    }
  }

public slots:
  void slotStartTest()
  {
    QString sql = QString("INSERT INTO %1(data) VALUES('%2');").arg(uuid_, uuid_.repeated(20000));

    TSqlQuery q(db_);

    for ( int i=0; i<100; ++i ) {
      q.exec(sql);
    }

    quit();
  }

private:
  NS_PGBase* db_;
  QString uuid_;
};


#endif // SQL_NSPGBASE_TEST_THREADS_DBTHREAD_H
