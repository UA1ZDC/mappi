#include "fs.h"

#include <qfileinfo.h>
#include <qdir.h>
#include <qobject.h>

#include <cross-commons/debug/tlog.h>

namespace meteo {

QStringList dirList(const QStringList& paths, int maxDepth)
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

QStringList fileList(const QString& path, const QStringList& extensions )
{
  if ( true == path.isEmpty() ) {
    return QStringList();
  }
  QFileInfo fi(path);
  if ( false == fi.exists() ) {
    error_log << QObject::tr("Директория '%1' не существует")
      .arg(path);
    return QStringList();
  }
  if ( false == fi.isDir() ) {
    error_log << QObject::tr("Файл '%1' не директория");
    return QStringList();
  }
  int filtersize = extensions.size();
  QStringList files;
  QDir dir(path);
  QFileInfoList list = dir.entryInfoList( QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot );
  for ( auto fi : list ) {
    if ( true == fi.isDir() ) {
      QStringList flist = fileList( fi.absoluteFilePath(), extensions );
      if ( true == flist.isEmpty() ) {
        continue;
      }
      files.append(flist);
    }
    else {
      if ( 0 != filtersize && false  == extensions.contains( fi.completeSuffix() ) ) {
        continue;
      }
      files.append( fi.absoluteFilePath() );
    }
  }
  return files;
}

QStringList fileFilteredList(const QString& path, const QString& wildcard )
{

  if ( true == path.isEmpty() ) {
    return QStringList();
  }
  QFileInfo fi(path);
  if ( false == fi.exists() ) {
    error_log << QObject::tr("Директория '%1' не существует")
      .arg(path);
    return QStringList();
  }
  if ( false == fi.isDir() ) {
    error_log << QObject::tr("Файл '%1' не директория");
    return QStringList();
  }

  QStringList files;
  QDir dir(path);
  QFileInfoList list = dir.entryInfoList( QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot );
  for ( auto fi : list ) {
    if ( true == fi.isDir() ) {
      QStringList flist = fileFilteredList( fi.absoluteFilePath(), wildcard );
      if ( true == flist.isEmpty() ) {
        continue;
      }
      files.append(flist);
    }
    else {
      QRegExp rexp(wildcard, Qt::CaseInsensitive, QRegExp::PatternSyntax::Wildcard);

      if ( false == rexp.exactMatch(fi.fileName()) ) {
        continue;
      }
      files.append( fi.absoluteFilePath() );
    }
  }
  return files;
}

QStringList fileList(const QString& path, const QString& extension )
{
  QStringList extlist;
  if ( 0 != extension.size() ) {
    extlist << extension;
  }
  return fileList( path, extlist );
}

}
