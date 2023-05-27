#ifndef TQT4INCLUDE_H
#define TQT4INCLUDE_H

#include <qpoint.h>
#include <qlist.h>
#include <qmutex.h>

#include <qtimer.h>
typedef  QTimer TTimer;


#define QSTRINGLIST_FROM_QSTRING( str, sep )      QString(str).split( sep, QString::SkipEmptyParts )
#define ASCII_FROM_QSTRING(str) str.toUtf8().data()
#define FIND_INQREGEXP(exp,str,index) exp.indexIn(str,index)
#define FIND_INQREGEXPDEFAULT( exp, str ) exp.indexIn(str)

#define TWRITE_ONLY   QIODevice::WriteOnly
#define TREAD_ONLY    QIODevice::ReadOnly
#define TREAD_WRITE   QIODevice::ReadWrite

template<class T> class QValueList : public QList<T>{};
typedef QMutexLocker TMutexLocker;

template<class T> int findInQList( const T& value,  const QList<T>& list )
{
  return list.indexOf(value);
}
  
#endif
