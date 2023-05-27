#include "cleartabletask.h"
#include "funcs.h"

#include <qset.h>
#include <qdir.h>
#include <qfile.h>
#include <qelapsedtimer.h>

#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/qobjectdeleter.h>

#include <sql/nspgbase/tsqlquery.h>

#include <mappi/global/global.h>
#include <mappi/settings/tmeteosettings.h>

namespace mappi {
namespace cleaner {

void ClearTableTask::run()
{
  NS_PGBase* db = new NS_PGBase(getDbConf(connectionName_));
  QObjectDeleter deleter(db);

  DbIntervalOption opt = ::meteo::mappi::TMeteoSettings::instance()->dbClearConf(connectionName_);
  if ( !opt.IsInitialized() ) {
    debug_log << tr("Удаление устаревших записей отключено.");
    return;
  }
  else {
    if ( kDays == opt.unit() ) {
      lifeTime_ = opt.value();
    }
    else {
      lifeTime_ = 1;
      debug_log << tr("Интервал должен задаваться в днях.");
    }
  }

  if ( tableName_.isEmpty() ) {
    error_log << tr("Не задано имя таблицы");
    return;
  }
  if ( dtField_.isEmpty() ) {
    error_log << tr("Не задано название колонки с датой/временем");
    return;
  }
  if ( pathField_.isEmpty() ) {
    error_log << tr("Не задано название колонки с расположением файла");
    return;
  }
  if ( -1 == lifeTime_ ) {
    error_log << tr("Не задано время хранения записей");
    return;
  }

  QString idSql = "SELECT a.attname FROM pg_index i "
                  "JOIN   pg_attribute a ON a.attrelid = i.indrelid AND a.attnum = ANY(i.indkey) "
                  "WHERE  i.indrelid = '" + tableName_ + "'::regclass "
                  "AND    i.indisprimary ";
  TSqlQuery q(db);
  if ( !q.exec(idSql) ) {
    error_log << tr("Ошибка при взаимодействии с БД");
    debug_log << q.errorMessage();
    debug_log << q.query();
    return;
  }
  if ( q.size() == 0 ) {
    error_log << tr("Не удалось определить первичный ключ для таблицы %1").arg(tableName_);
    debug_log << "sql:" << idSql;
    return;
  }
  idField_ = q.value(0,0);

  QElapsedTimer timer;
  timer.start();

  QString countSql = QString("SELECT count(*) FROM %1 WHERE %2 < timezone('utc'::text, now()) - interval '%3 days'")
                        .arg(tableName_)
                        .arg(dtField_)
                        .arg(lifeTime_);
  if ( !q.exec(countSql) ) {
    error_log << tr("Ошибка при взаимодействии с БД");
    debug_log << q.errorMessage();
    debug_log << q.query();
    return;
  }

  info_log << tr("Найдено устаревших записей") << QString(q.value(0,0)).toInt()
           << "(" << double(timer.restart())/1000 << "sec.)";

  int dayOffset = lifeTime_;
  while ( true ) {
    // проверяем наличие записей подлежащих удалению
    QString checkSql = QString("SELECT %1 FROM %2 WHERE %3 < timezone('utc'::text, now()) - interval '%4 days' LIMIT 1")
                          .arg(idField_, tableName_, dtField_)
                          .arg(dayOffset);
    if ( !q.exec(checkSql) ) {
      error_log << tr("Ошибка при взаимодействии с БД");
      debug_log << q.errorMessage();
      debug_log << q.query();
      return;
    }
    if ( q.size() == 0 ) {
      break;
    }

    QElapsedTimer t;
    t.start();

    QString selectSql = QString("SELECT %1,%2 FROM %3 WHERE "
                                "%4 BETWEEN timezone('utc'::text, now()) - interval '%5 days'"
                                "   AND timezone('utc'::text, now()) - interval '%6 days'")
                          .arg(idField_, pathField_, tableName_, dtField_)
                          .arg(dayOffset+1)
                          .arg(dayOffset);
    if ( !q.exec(selectSql) ) {
      error_log << tr("Ошибка при взаимодействии с БД");
      debug_log << q.errorMessage();
      debug_log << q.query();
      return;
    }
    if ( q.size() == 0 ) {
      dayOffset += 1;
      continue;
    }

    debug_log << "select" << q.size() << "records (" << double(t.restart())/1000 << "sec.)";

    QString deleteSql = QString("DELETE FROM %1 WHERE %2 IN (%3)").arg(tableName_, idField_);
    for ( int i=0,removed=0,isz=q.size(); i<isz; ) {
      QStringList ids;
      QSet<QString> dirs;
      QSet<QString> files;
      for ( int n=0; n<100 && i<isz; ++i, ++n ) {
        ids << q.value(i,0);
        QString filePath = q.value(i,1);
        files << filePath;
        QString path = filePath.left(filePath.lastIndexOf("/"));
        dirs << path;
      }

      TSqlQuery deleteQ(q.db());
      if ( !deleteQ.exec(deleteSql.arg(ids.join(","))) ) {
        error_log << tr("Ошибка при взаимодействии с БД");
        debug_log << deleteQ.errorMessage();
        debug_log << deleteQ.query();
        return;
      }

      foreach ( const QString& path, files ) {
        if ( !QFile::remove(path) ) {
          debug_log << "can't remove file" << path;
        }
      }

      foreach ( const QString& path, dirs ) {
        QDir dir(path);
        QString name = dir.dirName();
        dir.cdUp();
        dir.rmdir(name);
      }

      removed += ids.size();
      debug_log_if(removed%10000 == 0) << "delete" << removed << "of" << isz << double(t.elapsed())/1000 << "sec.";
    }
    debug_log << "delete" << q.size() << "of" << q.size() << double(t.elapsed())/1000 << "sec.";

    debug_log << "delete query:" << double(t.restart())/1000 << "sec.";
  }
}

} // cleaner
} // mappi
