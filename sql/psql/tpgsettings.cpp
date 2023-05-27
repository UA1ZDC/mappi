#include "tpgsettings.h"

#include <qfile.h>
#include <qdebug.h>

#include <cross-commons/app/paths.h>
#include <cross-commons/includes/tcommoninclude.h>
#include <cross-commons/debug/tlog.h>

namespace meteo {

ConnectProp dbparams( const QString& appname, bool* ok, const QString& filename )
{
  if ( 0 != ok ) {
    *ok = false;
  }
  QString path = MnCommon::etcPath(appname) + "/" + filename;

  
  ConnectProp prop;
  if ( false == QFile::exists(path) ) {
    qDebug() << __PRETTY_FUNCTION__ << "can't find file =" << path;
    return prop;
  }
  QFile file(path);
  if ( false == file.open( TREAD_ONLY ) ) {
    qDebug() << __PRETTY_FUNCTION__ << "can't read file =" << path;
    return prop;
  }
  char* readline = new char[1024 + 1];
  while ( false == file.atEnd() ) {
    qint64 res = file.readLine( readline, 1024 );
    if ( 0 >= res ) {
      continue;
    }
    QString line(readline);
    line.replace( "\n", "" );
    QStringList list = QSTRINGLIST_FROM_QSTRING( line, COMMON_SPLIT_REGEXP );
    if ( 2 != list.size() ) {
      continue;
    }
    if ( "dbhost" == list[0] ) {
      prop.setHost(list[1]);
    }
    else if ( "dbname" == list[0] ) {
      prop.setName(list[1]);
    }
    else if ( "dblogin" == list[0] ) {
      prop.setLogin(list[1]);
    }
    else if ( "dbpass" == list[0] ) {
      prop.setPass(list[1]);
    }
    else if ( "dbport" == list[0] ) {
      prop.setPort( list[1].toUShort() );
    }
  }
  delete[] readline;

  if ( 0 != ok ) {
    *ok = true;
  }
  return prop;
}

}
