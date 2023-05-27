#include "removetask.h"
#include "funcs.h"

#include <qdatetime.h>
#include <qdir.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qregexp.h>

#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/qobjectdeleter.h>

#include <sql/nspgbase/tsqlquery.h>

//#include <mappi/global/log.h>
#include <mappi/global/global.h>
#include <mappi/proto/cleaner.pb.h>


namespace mappi {
namespace cleaner {

RemoveTask::RemoveTask(QObject* parent)
 : PlannerTask(parent)
{
  lifeTime_ = 0;
}

void RemoveTask::run()
{
  DbIntervalOption opt = ::meteo::mappi::TMeteoSettings::instance()->dbClearConf("db_mappi");
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

  if ( lifeTime_ < 0 ) {
    debug_log << tr("Удаление устаревших записей отключено.");
    return;
  }

  if ( !execQuery() ) {
    debug_log << tr("Не удалось удалить таблицы в БД.");
    return;
  }

  foreach ( const QString& path, _paths ) {
    QDir dir(path);
    if (dir.exists() == false) {
      warning_log << QString::fromUtf8("Ошибка. Директория '%1' не существует").arg(path);
      continue;
    }

    foreach (const QFileInfo& item, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs)) {
      QString fileName = item.absoluteFilePath();
      if (needRemove(fileName) == true) {
        if (item.isDir() == true) {
          removeDir(fileName);
        }
        else if (item.isFile() == true) {
          removeFile(fileName);
        }
      }
    }
  }
}

bool RemoveTask::needRemove(const QString& fileName) const
{
  QRegExp rx("(\\d){4}[_-](\\d){2}[_-](\\d){2}");
  const int rxLength = 10;

  int pos = rx.indexIn(fileName);
  if (pos > -1) {
    QString str = fileName.mid(pos, rxLength);
    QDate fromFileNameDate = QDate::fromString(str.replace("_", "-"), QString("yyyy-MM-dd"));
    QDate checkDate = QDate::currentDate().addDays(-lifeTime_);
    return fromFileNameDate < checkDate;
  }
  return false;
}

void RemoveTask::removeDir(const QString& dirName) const
{
  if (dirName.isEmpty() == true) {
    return;
  }

  QDir dir(dirName);
  if (dir.exists() == false) {
    return;
  }

  foreach (const QFileInfo& item, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs)) {
    if (item.isDir() == true) {
      removeDir(item.absoluteFilePath());
    }
    else if (item.isFile() == true) {
      removeFile(item.absoluteFilePath());
    }
  }

  if (dir.rmdir(dirName) == false) {
    error_log << QString::fromUtf8("Ошибка при удалении директории '%1'").arg(dirName);
  }
}

void RemoveTask::removeFile(const QString& fileName) const
{
  if (fileName.isEmpty() == true || QFile::exists(fileName) == false) {
    return;
  }

  QFile file(fileName);
  if (file.remove() == false) {
    error_log << QString::fromUtf8("Ошибка при удалении файла '%1': %2")
                 .arg(file.fileName())
                 .arg(file.errorString());
  }
}

bool RemoveTask::execQuery() const
{
//  if (query_.isEmpty() ) { return true; }

//  QString sql = prepareSql(query_, "db_ptkpp");

//  NS_PGBase* db = new NS_PGBase(global::dbConfTelegram());
//  QObjectDeleter deleter(db);

//  for ( int n=0; n<30; ++n ) {
//    debug_log << "try clear...";

//    if ( !db->StartTransaction() ) { continue; }

//    if ( db->ExecQuery(sql) ) {
//      db->Commit();
//      return true;
//    }

//    db->Rollback();

//    msleep(1000 + n * 100);
//  }

  return false;
}

} // cleaner
} // mappi
