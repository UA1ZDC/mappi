#include "processedfiles.h"

#include <qfile.h>
#include <qcryptographichash.h>

#include <cross-commons/debug/tlog.h>

namespace meteo {

ProcessedFiles::ProcessedFiles()
{
}

ProcessedFiles::~ProcessedFiles()
{
  save();
}

void ProcessedFiles::addProcessed(const QString& fileName)
{
  QByteArray ba = QCryptographicHash::hash(fileName.toUtf8(), QCryptographicHash::Md5);
  QDate d = QDate::currentDate();
  hash_[d].insert(ba);
  count_[d] += 1;
}

bool ProcessedFiles::isProcessed(const QString& fileName)
{
  QByteArray ba = QCryptographicHash::hash(fileName.toUtf8(), QCryptographicHash::Md5);

  foreach ( const QDate& d, hash_.keys() ) {
    if ( hash_[d].contains(ba) ) {
      count_[d] += 1;
      return true;
    }
  }

  return false;
}

void ProcessedFiles::removeOld()
{
  QDate cur = QDate::currentDate();

  foreach ( const QDate& d, count_.keys() ) {
    if ( cur != d && 0 == count_[d] ) {
      hash_.remove(d);
      count_.remove(d);
    }
  }

  save();

  foreach ( const QDate& d, count_.keys() ) {
    if ( cur != d ) {
      count_[d] = 0;
    }
  }
}

bool ProcessedFiles::load(const QString& fileName)
{
  historyFile_ = fileName;

  QFile file(historyFile_);
  if ( !file.open(QFile::ReadOnly)  ) {
    error_log << QObject::tr("Не удалось открыть файл %1.").arg(file.fileName())
              << file.errorString();
    return false;
  }

  QDate d = QDate::currentDate();
  while ( !file.atEnd() ) {
    QByteArray line = file.readLine().trimmed();

    if ( line.startsWith("@date") ) {
      d = QDate::fromString(QString(line).section("=", 1, 1), Qt::ISODate);
    }
    else if ( line.startsWith("@match") ) {
      count_[d] = QString(line).section("=", 1, 1).toInt();
    }
    else {
      hash_[d].insert(QByteArray::fromHex(line));
    }
  }

  return true;
}

bool ProcessedFiles::save()
{
  QFile file(historyFile_);
  if ( !file.open(QFile::WriteOnly | QFile::Truncate)  ) {
    error_log << QObject::tr("Не удалось открыть файл %1.").arg(file.fileName())
              << file.errorString();
    return false;
  }

  foreach ( const QDate& d, hash_.keys() ) {
    QString sDate = QString("@date=%1").arg(d.toString(Qt::ISODate));
    QString sMatch = QString("@match=%1").arg(count_[d]);
    file.write(sDate.toUtf8() + "\n");
    file.write(sMatch.toUtf8() + "\n");
    foreach ( const QByteArray& ba, hash_[d] ) {
      file.write(ba.toHex() + "\n");
    }
  }

  return true;
}

} // meteo
