#include "dbtask.h"
#include "funcs.h"

#include <qelapsedtimer.h>

#include <cross-commons/app/qobjectdeleter.h>
#include <cross-commons/debug/tlog.h>

#include <sql/nspgbase/tsqlquery.h>

#include <mappi/global/global.h>
#include <mappi/settings/tmeteosettings.h>

namespace mappi {
namespace cleaner {

void DbTask::run()
{
  NS_PGBase* db = new NS_PGBase(getDbConf(connectionName_));
  QObjectDeleter deleter(db);

  foreach ( const QString& query, queries_ ) {
    TSqlQuery q(db);
    QElapsedTimer t;
    t.start();

    QString sql = prepareSql(query, connectionName_);
    var(sql);
    if ( !q.exec(sql) ) {
      error_log << tr("[ ошибка ] \"%1\" %2 сек.").arg(q.query()).arg(double(t.restart())/1000);
      error_log << tr("Ошибка при взаимодействии с БД");
      debug_log << q.errorMessage();
      debug_log << q.query();
      return;
    }

    info_log << tr("[ успешно ] \"%1\" %2 сек.").arg(q.query()).arg(double(t.restart())/1000);
  }

  foreach ( const QString& sql, reindexQueries_ ) {
    TSqlQuery q(db);
    if ( !q.exec(sql) ) {
      error_log << tr("Ошибка при взаимодействии с БД");
      debug_log << q.errorMessage();
      debug_log << q.query();
      return;
    }

    QString reindexSql("REINDEX TABLE %1;");
    for ( int i=0,isz=q.size(); i<isz; ++i ) {
      QElapsedTimer t;
      t.start();

      QString tblName = QString(q.value(i,0));

      // try lock
      bool locked = false;
      for ( int n=0; n<30; ++n) {
        debug_log << "try lock" << tblName << "...";

        if ( !db->StartTransaction() ) { continue; }

        QString sql = QString("LOCK TABLE %1 IN ACCESS EXCLUSIVE MODE NOWAIT;").arg(tblName);
        if ( db->ExecQuery(sql) ) {
          locked = true;
          break;
        }

        db->Rollback();

        msleep(1000 + n * 100);
      }

      if ( !locked ) {
        debug_log << "skip table" << tblName << "becouse can't lock";
        continue;
      }

      TSqlQuery reindexQ(db);
      if ( !reindexQ.exec(reindexSql.arg(tblName)) ) {
        db->Rollback();
        error_log << tr("[ ошибка ] \"%1\" %2 сек.").arg(reindexQ.query()).arg(double(t.restart())/1000);
        debug_log << reindexQ.errorMessage();
        continue;
      }
      else {
        db->Commit();
      }

      info_log << tr("[ успешно ] \"%1\" %2 сек.").arg(reindexQ.query()).arg(double(t.restart())/1000);
    }
  }

  foreach ( const QString& sql, vacuumQueries_ ) {
    TSqlQuery tblQ(db);
    if ( !tblQ.exec(sql) ) {
      error_log << tr("Ошибка при взаимодействии с БД");
      debug_log << tblQ.errorMessage();
      debug_log << tblQ.query();
      return;
    }

    QString reindexSql("VACUUM ANALYZE %1;");
    for ( int i=0,isz=tblQ.size(); i<isz; ++i ) {
      QElapsedTimer t;
      t.start();

      TSqlQuery q(db);
      if ( !q.exec(reindexSql.arg(QString(tblQ.value(i,0)))) ) {
        error_log << tr("[ ошибка ] \"%1\" %2 мсек.").arg(q.query()).arg(t.restart());
        debug_log << q.errorMessage();
        continue;
      }

      info_log << tr("[ успешно ] \"%1\" %2 мсек.").arg(q.query()).arg(t.restart());
    }
  }

  foreach ( const QString& sql, analyzeQueries_ ) {
    TSqlQuery tblQ(db);
    if ( !tblQ.exec(sql) ) {
      error_log << tr("Ошибка при взаимодействии с БД");
      debug_log << tblQ.errorMessage();
      debug_log << tblQ.query();
      return;
    }

    QString sql2("ANALYZE %1;");
    for ( int i=0,isz=tblQ.size(); i<isz; ++i ) {
      QElapsedTimer t;
      t.start();

      TSqlQuery q(db);
      if ( !q.exec(sql2.arg(QString(tblQ.value(i,0)))) ) {
        error_log << tr("[ ошибка ] \"%1\" %2 мсек.").arg(q.query()).arg(t.restart());
        debug_log << q.errorMessage();
        continue;
      }

      info_log << tr("[ успешно ] \"%1\" %2 мсек.").arg(q.query()).arg(t.restart());
    }
  }
}

} // cleaner
} // mappi

