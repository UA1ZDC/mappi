#include "dbi.h"

#include <qfile.h>
#include <qfileinfo.h>

#include <cross-commons/debug/tlog.h>
#include <commons/funcs/fs.h>

namespace meteo {

namespace global {

QMap< QString, QString > loadMongoQueriesNew()
{
  QMap< QString, QString > list;
  QStringList files = meteo::fileList( MnCommon::sharePath("meteo") + "/mongo.d", "json" );
  for ( auto f : files ) {
    QFile file(f);
    if ( false == file.open(QIODevice::ReadOnly) ) {
      error_log << QObject::tr("Не удалось открыть файл '%1' для чтения. Ошибка = %2")
        .arg(f)
        .arg( file.errorString() );
      continue;
    }
    QString str = QString::fromUtf8( file.readAll() );
    list.insert( QFileInfo(file).baseName(), str );
  }
  return list;
}

QMap< QString, QString > loadSqlQueries()
{
  QMap< QString, QString > list;
  QStringList files = meteo::fileList( MnCommon::sharePath("meteo") + "/sql.d", "sql" );
  for ( auto f : files ) {
    QFile file(f);
    if ( false == file.open(QIODevice::ReadOnly) ) {
      error_log << QObject::tr("Не удалось открыть файл '%1' для чтения. Ошибка = %2")
        .arg(f)
        .arg( file.errorString() );
      continue;
    }
    auto arr = file.readAll();
    list.insert( QFileInfo(file).baseName(), QString::fromUtf8(arr) );
  }
  return list;
}
}

Dbi::Dbi( const QString& host, uint16_t port, const QString& database, const QString& login, const QString& pass )
  : QObject()
{
  params_.setHost(host);
  params_.setPort(port);
  params_.setName(database);
  params_.setLogin(login);
  params_.setPass(pass);
}

Dbi::Dbi( const ConnectProp& params )
  : QObject(),
  params_(params)
{
}

Dbi::Dbi()
  : QObject()
{
}

Dbi::~Dbi()
{
}

bool Dbi::connect( const ConnectProp& params )
{
  setParams(params);
  return connect();
}

bool Dbi::connect( const QString& host, uint16_t port, const QString& database, const QString& login, const QString& pass )
{
  return connect( ConnectProp( host, database, login, pass, port ) );
}

}
