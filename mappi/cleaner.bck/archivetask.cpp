#include "archivetask.h"

#include <qdir.h>
#include <qprocess.h>
#include <qelapsedtimer.h>

#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/qobjectdeleter.h>

#include <sql/nspgbase/tsqlquery.h>

#include <mappi/global/global.h>
#include <mappi/proto/cleaner.pb.h>
#include <mappi/settings/tmeteosettings.h>

namespace mappi {
namespace cleaner {

ArchiveTask::ArchiveTask(QObject* parent) :
  PlannerTask(parent),
  olderThan_(1)
{
}

void ArchiveTask::run()
{
////  DbIntervalOption opt = ::meteo::ukaz::Settings::instance()->dbArchiveConf("db_ptkpp");
//  DbIntervalOption opt = ::meteo::mappi::TMeteoSettings::instance()->dbArchiveConf("db_mappi");


//  NS_PGBase* db = new NS_PGBase(global::dbConfTelegram());
//  QObjectDeleter deleter(db);

//  if ( !opt.IsInitialized() ) {
//    debug_log << tr("Параметры архивации не заданы.");
//    return;
//  }
//  else {
//    if ( kDays == opt.unit() ) {
//      olderThan_ = opt.value();
//    }
//    else {
//      olderThan_ = 1;
//      debug_log << tr("Интервал должен задаваться в днях.");
//    }
//  }

//  if ( path_.isEmpty() ) {
//    error_log << tr("Не задана директория размещения архивов.");
//    return;
//  }

//  QDir dir(path_);
//  if ( !dir.exists() && !dir.mkpath(path_) ) {
//    error_log << tr("Ошибка при создании директории '%1'").arg(path_);
//    return;
//  }

//  QElapsedTimer timer;
//  timer.start();

//  int dayOffset = olderThan_;
//  while ( true ) {
//    // проверяем наличие телеграмм подлежащих архивации
//    QString checkSql = "SELECT id FROM telegrams "
//                       "WHERE converted_dt IS NOT NULL AND message_path NOT LIKE 'tar:%' "
//                       "AND converted_dt < TIMESTAMP 'today' - interval '%1 days' LIMIT 1";
//    checkSql = checkSql.arg(dayOffset);
//    TSqlQuery q(db);
//    if ( !q.exec(checkSql) ) {
//      error_log << tr("Ошибка при взаимодействии с БД");
//      debug_log << q.errorMessage();
//      debug_log << q.query();
//      return;
//    }
//    if ( q.size() == 0 ) {
//      break;
//    }

//    timer.restart();

//    // архивируем телеграммы за один день
//    QString selectSql = "SELECT id,message_path,converted_dt FROM telegrams "
//                        "WHERE converted_dt IS NOT NULL AND message_path NOT LIKE 'tar:%' "
//                        "AND converted_dt >= TIMESTAMP 'today' - interval '%1 days' "
//                        "AND converted_dt < TIMESTAMP 'today' - interval '%2 days' ";
//    selectSql = selectSql.arg(dayOffset+1).arg(dayOffset);
//    if ( !q.exec(selectSql) ) {
//      error_log << tr("Ошибка при взаимодействии с БД");
//      debug_log << q.errorMessage();
//      debug_log << q.query();
//      return;
//    }
//    if ( q.size() == 0 ) {
//      dayOffset += 1;
//      continue;
//    }

//    QString createDate = q.value(0,"converted_dt").left(10);

//    debug_log << "archive" << q.size() << "telegrams for day" << createDate;
//    debug_log << "select query:" << double(timer.elapsed() + 1)/1000 << "sec.";

//    QString tarFilePath = path_ + "/" + createDate + ".tar";
//    for ( int n=1; QFile::exists(tarFilePath); ++n ) {
//      tarFilePath = path_ + "/" + createDate + "_u" + QString::number(n) + ".tar";
//    }

//    QStringList args = QStringList() << "--absolute-names"
//                                     << "-cf"
//                                     << tarFilePath
//                                     << "-T"
//                                     << "-";
//    QProcess tar;
//    tar.start("tar", args);
//    if ( !tar.waitForStarted() ) {
//      error_log << tr("Ошибка при запуске tar");
//      return;
//    }

//    timer.restart();

//    QStringList updateIds;
//    QStringList pathList; // список путей к файлам телеграмм для удаления
//    for ( int i=0,isz=q.size(); i<isz; ++i ) {
//      if ( !QFile::exists(q.value(i, "message_path")) ) {
//        continue;
//      }

//      if ( -1 == tar.write(q.value(i, "message_path").toLocal8Bit() + '\n') ) {
//        error_log << tr("Ошибка при взаимодействии с tar");
//        return;
//      }

//      updateIds << q.value(i, "id");
//      pathList << q.value(i, "message_path");
//    }

//    tar.closeWriteChannel();
//    tar.waitForFinished(-1);
//    if ( tar.exitCode() != 0 ) {
//      error_log << tr("Ошибка при архивировании телеграмм '%1'").arg(tarFilePath);
//      debug_log << "exit code:" << tar.exitCode();
//      debug_log << "tar errors:" << tar.readAllStandardError();
//      return;
//    }

//    debug_log << "archive:" << double(timer.restart())/1000 << "sec.";

//    int sz = updateIds.size();
//    int updated = 0;
//    TSqlQuery updateQ(q.db());
//    while ( updateIds.size() > 0 ) {
//      QStringList ids;
//      for ( int i=0; i<5000 && updateIds.size() > 0; ++i, ++updated ) {
//        ids << updateIds.takeFirst();
//      }

//      QString updateSql = "UPDATE telegrams SET message_path='tar://"+tarFilePath+":' || message_path WHERE id IN (%1)";
//      updateSql = updateSql.arg( ids.join(",") );
//      if ( !updateQ.exec(updateSql) ) {
//        error_log << tr("Ошибка при взаимодействии с БД");
//        debug_log << updateQ.errorMessage();
//        debug_log << updateQ.query();
//        return;
//      }

//      debug_log << "updated" << updated << "of" << sz << double(timer.elapsed())/1000 << "sec.";
//    }

//    debug_log << "update query:" << double(timer.restart())/1000 << "sec.";

//    debug_log << "delete telegram files...";
//    while ( pathList.size() > 0 ) {
//      QFile file(pathList.takeFirst());
//      if ( !file.remove() ) {
//        debug_log << "can't remove file" << file.fileName();
//        debug_log  << file.errorString();
//      }
//    }
//    debug_log << "files deleted (" << double(timer.restart())/1000 << "sec.)";

//    dayOffset += 1;
//  }
}

bool ArchiveTask::testTar(const QString& filePath) const
{
  QProcess tar;
  tar.start("tar", QStringList() << "-tf" << filePath );
  tar.waitForFinished(-1);

  QString err = tar.readAllStandardError();
  if ( tar.exitCode() != 0 || err.contains("Error", Qt::CaseInsensitive) ) {
    return false;
  }

  return true;
}

} // cleaner
} // mappi

