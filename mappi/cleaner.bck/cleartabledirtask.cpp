#include "cleartabledirtask.h"
#include "funcs.h"

#include <qprocess.h>
#include <qelapsedtimer.h>

#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/qobjectdeleter.h>

#include <sql/nspgbase/tsqlquery.h>

namespace mappi {
namespace cleaner {

void ClearTableDirTask::run()
{
  NS_PGBase* db = new NS_PGBase(getDbConf(connectionName_));
  QObjectDeleter deleter(db);

  TSqlQuery q(db);
  foreach ( const QString& query, _queries ) {
    QElapsedTimer t;
    t.start();

    QString sql = prepareSql(query, connectionName_);
    if ( !q.exec(sql) ) {
      info_log << tr("[ ошибка ] \"") << sql << "\" (" << double(t.restart())/1000 << "sec.)";
      error_log << tr("Ошибка при взаимодействии с БД");
      debug_log << q.errorMessage();
      debug_log << q.query();
      return;
    }

    info_log << tr("[ успешно ] ") << '"'+sql+'"' << " (" << double(t.restart())/1000 << "sec.)";
  }

  levelUp();

  QDateTime dt = QDateTime::fromString(q.value(0,0), Qt::ISODate);
  if ( !dt.isValid() ) {
    error_log << tr("Невозможно удалить устаревшие файлы, получено некорректное значение времени");
    debug_log << q.query();
    levelDown();
    return;
  }

  int depth = _dtFormat.split("/", QString::SkipEmptyParts).size();
  QStringList paths = pathListByFilter(paths_);

  QRegExp rx("(" + paths.join("|") + ")[/]?");

  paths = dirList(paths, depth);

  foreach ( const QString& p, paths ) {
    QString path = p;
    path.remove(rx);

    QDateTime dirDt = QDateTime::fromString(path, _dtFormat);

    if ( !dirDt.isValid() ) {
      warning_log << tr("Не удалось определить дату/время.");
      var(_dtFormat);
      var(path);
      continue;
    }


    if ( dirDt < dt ) {
      if ( rmPath(p) ) {
        warning_log << tr("Не удалось удалить директорию %1").arg(p);
      }
      else {
        debug_log << "DELETED:" << p;
      }
    }
  }

  foreach ( const QString& path, paths_ ) {
    rmEmptyDir(path);
  }

  foreach ( const QString& templ, fileTemplateList_ ) {
    QFileInfo fi(templ);
    QString name = fi.baseName();
    QRegExp rx("(.*)(\\$\\{.*\\})(.*)");
    rx.indexIn(name);
    QString format = "'" + rx.cap(1) + "'" + rx.cap(2) + "'" + rx.cap(3) + "'";
    format.remove("${").remove("}");

    QDir dir(fi.path());
    QStringList files = dir.entryList(QDir::Files);
    foreach ( const QString& file, files ) {
      QDateTime fileDt = QDateTime::fromString(file, format);
      if ( !fileDt.isValid() ) {
        continue;
      }
      if ( fileDt < dt ) {
        if ( -1 == mac_ ) {
          QFile::remove(dir.absoluteFilePath(file));
        }
      }
    }
  }

  levelDown();
}

bool ClearTableDirTask::rmPath(const QString& path) const
{
  QDir dir(path);
  if ( !dir.exists() ) { return true; }

  QStringList dirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
  foreach ( const QString& dirName, dirs ) {
    if ( !rmPath(dir.absoluteFilePath(dirName)) ) {
      return false;
    }
  }

  QStringList files = dir.entryList(QDir::Files);
  foreach ( const QString& fileName, files ) {
    QFile file(dir.absoluteFilePath(fileName));
    debug_log << "rm file:" << dir.absoluteFilePath(fileName);
    if ( !file.remove(dir.absoluteFilePath(fileName)) ) {
      debug_log << file.errorString();
      return false;
    }
  }

  debug_log << "rm dir:" << path;
  if ( !dir.rmdir(path) ) {
    return false;
  }

  return true;
}

void ClearTableDirTask::rmEmptyDir(const QString& path) const
{
  QDir dir(path);

  if ( !dir.exists() ) { return; }

  QStringList dirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
  foreach ( const QString& dirName, dirs ) {
    rmEmptyDir(dir.absoluteFilePath(dirName));
  }

  QStringList files = dir.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
  if ( files.isEmpty() ) {
    dir.rmdir(path);
  }
}

QStringList ClearTableDirTask::dirList(const QStringList& paths, int maxDepth) const
{
  if ( paths.isEmpty() || 0 == maxDepth ) { return QStringList(); }

  QStringList list;

  foreach ( const QString& path, paths ) {
    QDir dir(path);

    if ( !dir.exists() ) { continue; }

    QStringList dirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    foreach ( const QString& dirName, dirs ) {
      list += dir.absoluteFilePath(dirName);
    }
    if ( dirs.isEmpty() ) {
      list << path;
    }
  }

  if ( paths == list ) {
    return list;
  }

  QStringList l = dirList(list, maxDepth - 1);

  if ( !l.isEmpty() ) { return l; }

  return list;
}

QStringList ClearTableDirTask::pathListByFilter(const QStringList& patterns)
{
  QStringList resList;
  foreach ( const QString& p, patterns ) {
    QStringList dirNames = p.split("/", QString::SkipEmptyParts);

    QStringList paths("/");
    foreach ( const QString& n, dirNames ) {
      if ( n.contains("*") ) {
        QStringList tmp = paths;
        paths.clear();

        foreach ( const QString& path, tmp ) {
          QDir dir(path);
          QStringList dirList = dir.entryList(QStringList(n), QDir::Dirs | QDir::NoDotAndDotDot);
          foreach ( const QString& d, dirList ) {
            paths << dir.absoluteFilePath(d);
          }
        }
        continue;
      }

      QStringList tmp = paths;
      paths.clear();

      foreach ( const QString& path, tmp ) {
        paths << QDir::cleanPath(path + "/" + n);
      }
    }
    resList += paths;
  }

  return resList;
}

void ClearTableDirTask::levelUp()
{

}

void ClearTableDirTask::levelDown()
{

}

} // cleaner
} // mappi

