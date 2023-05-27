#include "localfs.h"

#include "localfile.h"
#include "localfilelist.h"
#include "psql.h"
#include "psqlquery.h"

#include <qdir.h>
#include <qfile.h>
#include <qsavefile.h>
#include <qfileinfo.h>

#include <cross-commons/app/paths.h>
#include <cross-commons/debug/tlog.h>

#include <sql/rapidjson/document.h>
#include <sql/rapidjson/prettywriter.h>
#include <sql/rapidjson/stringbuffer.h>

namespace meteo {

LocalFs::LocalFs()
{
  error_ = GridFs::kNotInitialized;
}

LocalFs::~LocalFs()
{
  delete client_; client_ = nullptr;
}

bool LocalFs::connect( const meteo::ConnectProp& p )
{
  params_ = p;
  db_ = params_.name();
  if ( nullptr != client_ && ( client_->params() != params_ || false == client_->connected() ) ) {
    delete client_; client_ = nullptr;
  }
  client_ = Psql::createDatabase(params_);
  if ( nullptr == client_ ) {
    lastError_ = QObject::tr("Ошибка подключения к БД");
    error_ = GridFs::kNotInitialized;
    return false;
  }
  error_ = GridFs::kNoError;
  return true;
}

bool LocalFs::isConnected() const
{
  return ( nullptr != client_ && true == client_->connected() );
}

void LocalFs::use(const QString& db, const QString& prefix )
{
   db_ = db;
   prefix_ = prefix;
}

bool LocalFs::put( const QString& filename, const QByteArray& data, GridFile* gridfile, bool rewrite )
{
  if ( db_.isEmpty() ) {
    lastError_ = QObject::tr("Не задано имя БД.");
    error_ = GridFs::kErrorQuery;
    return false;
  }
  if ( prefix_.isEmpty() ) {
    lastError_ = QObject::tr("Не задан префикс.");
    error_ = GridFs::kErrorQuery;
    return false;
  }

  if ( nullptr == client_ ) {
    client_ = Psql::createDatabase(params_);
  }
  if ( nullptr == client_ ) {
    lastError_ = QObject::tr("Не удалось подключиться к БД! Параметры подключения = %1").arg( params_.print() );
    error_log << lastError_;
    error_ = GridFs::kErrorConnect;
    return false;
  }
  GridFile gfile;
  if ( true == rewrite ) {
    gfile = findOneByName(filename);
  }
  if ( true == rewrite && true == gfile.isValid() && true == gfile.hasFile() ) {
    int64_t res = gfile.write( data, true );
    if ( nullptr != gridfile ) {
      *gridfile = gfile;
    }
    if ( -2 == res ) {
      error_ = GridFs::kErrorDuplicate;
      return false;
    }
    return true;
  }
  LocalFile* lc = new LocalFile( filename, data, prefix_, client_ );
  bool dup = false;
  if ( false == lc->save(&dup) ) {
    if ( true == dup ) {
      lastError_ = QObject::tr("Ошибка ункальности файла по md5");
      error_ = GridFs::kErrorDuplicate;
      if ( nullptr != gridfile ) {
        *gridfile = GridFile(lc);
      }
    }
    else {
      lastError_ = QObject::tr("Ошибка сохранения файла = '%1'").arg(filename);
      error_ = GridFs::kErrorSave;
      delete lc;
    }
    return false;
  }

  if ( nullptr != gridfile ) {
    *gridfile = GridFile(lc);
  }
  else {
    delete lc;
  }
  
  return true;
}

bool LocalFs::removeFromDb( const QString& fileName )
{
  if ( nullptr == client_ ) {
    client_ = Psql::createDatabase(params_);
  }
  if ( nullptr == client_ ) {
    lastError_ = QObject::tr("Не удалось подключиться к БД! Параметры подключения = %1").arg( params_.print() );
    error_log << lastError_;
    error_ = GridFs::kErrorConnect;
    return false;
  }
  QString qname("remove_file_description");
  auto query = client_->queryptrByName(qname);
  if ( nullptr == query ) {
    lastError_ = QObject::tr("Не создан запрос = '%1'").arg(qname);
    error_log << lastError_;
    return false;
  }
  query->arg( "tablename", prefix_ );
  query->arg( "filename", fileName );
  if ( false == query->exec() ) {
    lastError_ = QObject::tr("Ошибка выполнения запроса = '%1'").arg( query->query() );
    return false;
  }
  return true;
}

int LocalFs::remove( const QString& filename, bool* ok )
{
  Q_UNUSED(ok);
  GridFileList list = findByName(filename);
  int cnt = 0;
  while ( true == list.next() ) {
    GridFile file = list.file();
    if ( true == file.remove() ) {
      ++cnt;
    }
  }
  return cnt;
}

GridFile LocalFs::findOne( const QString& query )
{
  if ( db_.isEmpty() ) {
    lastError_ = QObject::tr("Не задано имя БД.");
    error_ = GridFs::kErrorQuery;
    return GridFile();
  }
  if ( prefix_.isEmpty() ) {
    lastError_ = QObject::tr("Не задан префикс.");
    error_ = GridFs::kErrorQuery;
    return GridFile();
  }

  QByteArray arr = query.toUtf8();
  rapidjson::Document doc;
  doc.Parse( arr.data() );
  if ( true == doc.HasParseError() ) {
    lastError_ =  QObject::tr("Запрос '%1'не является json-документом. Ошибка = %2")
      .arg(query)
      .arg( doc.GetParseError() );
    error_log << lastError_;
    error_ = GridFs::kErrorQuery;
    return GridFile();
  }
  if ( false == doc.HasMember("filename") && false == doc.HasMember("_id") ) {
    lastError_ = QObject::tr("Не указаны имя файла и идентификатор!");
    error_log << lastError_;
    error_ = GridFs::kErrorQuery;
    return GridFile();
  }
  if ( nullptr == client_ ) {
    client_ = Psql::createDatabase(params_);
  }
  if ( nullptr == client_ ) {
    lastError_ = QObject::tr("Не удалось подключиться к БД! Параметры подключения = %1").arg( params_.print() );
    error_log << lastError_;
    error_ = GridFs::kErrorConnect;
    return GridFile();
  }
  GridFile::GridFilePrivate* gfp = new LocalFile( doc, prefix_, client_ );

  GridFile gf(gfp);
  if ( false == gfp->isValid() || false == gfp->hasFile() ) {
    lastError_ = gfp->lastError();
    error_ = GridFs::kErrorQuery;
  }
  return gf;
}

GridFile LocalFs::findOneByName( const QString& filename )
{
  if ( db_.isEmpty() ) {
    lastError_ = QObject::tr("Не задано имя БД.");
    error_ = GridFs::kErrorQuery;
    return GridFile();
  }
  if ( prefix_.isEmpty() ) {
    lastError_ = QObject::tr("Не задан префикс.");
    error_ = GridFs::kErrorQuery;
    return GridFile();
  }

  if ( nullptr == client_ ) {
    client_ = Psql::createDatabase(params_);
  }
  if ( nullptr == client_ ) {
    lastError_ = QObject::tr("Не удалось подключиться к БД! Параметры подключения = %1").arg( params_.print() );
    error_log << lastError_;
    error_ = GridFs::kErrorConnect;
    return GridFile();
  }
  GridFile::GridFilePrivate* gfp = new LocalFile( filename, prefix_, client_ );

  GridFile gf(gfp);
  if ( false == gfp->isValid() || false == gfp->hasFile() ) {
    lastError_ = gfp->lastError();
    error_ = GridFs::kErrorQuery;
  }
  return gf;
}

GridFile LocalFs::findOneById( const QString& id )
{
  if ( db_.isEmpty() ) {
    lastError_ = QObject::tr("Не задано имя БД.");
    error_ = GridFs::kErrorQuery;
    return GridFile();
  }
  if ( prefix_.isEmpty() ) {
    lastError_ = QObject::tr("Не задан префикс.");
    error_ = GridFs::kErrorQuery;
    return GridFile();
  }

  if ( nullptr == client_ ) {
    client_ = Psql::createDatabase(params_);
  }
  if ( nullptr == client_ ) {
    lastError_ = QObject::tr("Не удалось подключиться к БД! Параметры подключения = %1").arg( params_.print() );
    error_log << lastError_;
    error_ = GridFs::kErrorConnect;
    return GridFile();
  }
  int64_t fid = id.toLongLong();
  GridFile::GridFilePrivate* gfp = new LocalFile( fid, prefix_, client_ );

  GridFile gf(gfp);
  if ( false == gfp->isValid() || false == gfp->hasFile() ) {
    lastError_ = gfp->lastError();
    error_ = GridFs::kErrorQuery;
  }
  return gf;
}

GridFileList LocalFs::find(const QString& filter ) const
{
  if ( db_.isEmpty() ) {
    lastError_ = QObject::tr("Не задано имя БД.");
    error_ = GridFs::kErrorQuery;
    return GridFileList();
  }
  if ( prefix_.isEmpty() ) {
    lastError_ = QObject::tr("Не задан префикс.");
    error_ = GridFs::kErrorQuery;
    return GridFileList();
  }
  QByteArray arr = filter.toUtf8();
  rapidjson::Document doc;
  doc.Parse( arr.data() );
  if ( true == doc.HasParseError() ) {
    lastError_ =  QObject::tr("Запрос не является json-документом. Ошибка = %1")
      .arg( doc.GetParseError() );
    error_log << lastError_;
    error_ = GridFs::kErrorQuery;
    return GridFileList();
  }
  if ( nullptr == client_ ) {
    client_ = Psql::createDatabase(params_);
  }
  if ( nullptr == client_ ) {
    lastError_ = QObject::tr("Не удалось подключиться к БД! Параметры подключения = %1").arg( params_.print() );
    error_ = GridFs::kErrorConnect;
    return GridFileList();
  }
  LocalFileList* lfs = new LocalFileList( prefix_, client_ );
  if ( false == lfs->find(doc) ) {
    delete lfs;
    lastError_ = QObject::tr("Не удалось найти файлы!");
    return GridFileList();
  }

  return GridFileList(lfs);
}


GridFileList LocalFs::findByName( const QString& filename ) const
{
  if ( db_.isEmpty() ) {
    lastError_ = QObject::tr("Не задано имя БД.");
    error_ = GridFs::kErrorQuery;
    return GridFileList();
  }
  if ( prefix_.isEmpty() ) {
    lastError_ = QObject::tr("Не задан префикс.");
    error_ = GridFs::kErrorQuery;
    return GridFileList();
  }
  if ( nullptr == client_ ) {
    client_ = Psql::createDatabase(params_);
  }
  if ( nullptr == client_ ) {
    lastError_ = QObject::tr("Не удалось подключиться к БД! Параметры подключения = %1").arg( params_.print() );
    error_ = GridFs::kErrorConnect;
    return GridFileList();
  }
  LocalFileList* lfs = new LocalFileList( prefix_, client_ );
  if ( false == lfs->findByName(filename) ) {
    delete lfs;
    lastError_ = QObject::tr("Не удалось найти файлы!");
    return GridFileList();
  }

  return GridFileList(lfs);
}

QString LocalFs::rootLocalfsPath()
{
  QString path = QString("/localfs/");
  return MnCommon::varPath() + path;
//  return  "/var/localfs/";
}

QString LocalFs::dirNameDb( const QString& db, const QString& subDir)
{
  QString path = QString("%1/%2/%3/")
    .arg( LocalFs::rootLocalfsPath() )
    .arg(db)
    .arg(subDir);
  return path;
}

QString LocalFs::dirNameFs( const QString& db, const QString& prefix )
{
  QString path = QString("/%1/%2/fs_%3/")
    .arg( LocalFs::rootLocalfsPath() )
    .arg(db)
    .arg(prefix);
  return path;
}

QString LocalFs::fileNameFs( const QString& db, const QString& prefix, const QString& filename, int64_t id)
{
  int64_t subdirost = id%100000;
  int64_t subsubdirost = subdirost%100;
  QString subdir = QString::number( id - subdirost );
  QString subsubdir = QString::number( subdirost - subsubdirost );
  QString path = LocalFs::dirNameFs( db, prefix );
  path = QString("%1/%2/%3/%4_%5")
    .arg(path)
    .arg(subdir)
    .arg(subsubdir)
    .arg(id)
    .arg(filename);
  return path;
}

}
