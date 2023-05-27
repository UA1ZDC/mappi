#include "tlgfuncs.h"

#include <cross-commons/debug/tlog.h>

#include <qfile.h>
#include <qprocess.h>
#include <qstringlist.h>
#include <qsemaphore.h>

namespace meteo {

static QSemaphore gSem(25);

QByteArray readTelegram(const QString& fileName, bool* aOk, QString* aError)
{
  bool b;
  QString e;

  bool* ok = ( nullptr != aOk ? aOk : &b );
  QString* error = ( nullptr != aError ? aError : &e );

  QStringList list = fileName.split(":", QString::SkipEmptyParts);

  if ( list[0].startsWith("tar") ) {
    if ( list.size() < 3 ) {
      *ok = false;
      *error = QObject::tr("Некорректный путь к архиву");
      return QByteArray();
    }

    QString file = list[2];

    while(!gSem.tryAcquire(1, 20)){
    }

    QStringList args;
    args << "--to-stdout" << "-xf" << list[1].remove(0,2) << file;
    QProcess* tar = new QProcess;
    tar->start("tar", args);
    tar->waitForFinished();
    if ( tar->exitCode() != 0 ) {
      *ok = false;
      *error = QObject::tr("Не удалось получить телеграмму из архива '%1'").arg(list[1]);
      delete tar;
      gSem.release(1);
      return QByteArray();
    }
    QByteArray ba = tar->readAll();
    delete tar;
    gSem.release(1);

    *ok = true;
    return ba;
  }

  QFile file( fileName );
  if ( !file.open(QIODevice::ReadOnly) ) {
    *ok = false;
    *error = QObject::tr("Ошибка при открытии файла %1. %2.").arg(file.fileName()).arg(file.errorString());
    return QByteArray();
  }

  *ok = true;
  return file.readAll();
}

QDateTime convertedDateTime(const QDateTime& metaDt, const QString& yygggg)
{
  int tlgDay = yygggg.left(2).toInt();
  if ( tlgDay > 31 || tlgDay < 1 || !metaDt.isValid() ) {
    return QDateTime();
  }
  QTime tlgTime = QTime::fromString(yygggg.right(4), "hhmm");

  QDateTime dt = metaDt;
  while ( tlgDay > dt.date().day() ) {
    dt = dt.addDays( -1 );
  }

  if ( tlgDay != dt.date().day() ) {
    QDate date( dt.date().year(), dt.date().month(), tlgDay );
    dt.setDate( date );
  }

  dt.setTime( tlgTime );

  return dt;
}

} // meteo
