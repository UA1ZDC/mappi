#include "localfilelist.h"
#include "localfile.h"
#include "psql.h"

#include <sql/dbi/dbiquery.h>
#include <sql/dbi/dbientry.h>

#include <cross-commons/debug/tlog.h>

namespace meteo {

LocalFileList::LocalFileList( const QString& prefix, Dbi* client )
  : prefix_(prefix),
  client_(client)
{
}

LocalFileList::~LocalFileList()
{
}

bool LocalFileList::find( const rapidjson::Document& doc )
{
  files_.clear();
  currentpos_ = -1;
  if ( nullptr == client_ || false == client_->connected() ) {
    error_log << QObject::tr("Нет подключения к БД!");
  }
  QString qname("find_file_list");
  auto query = client_->queryptrByName(qname);
  if ( nullptr == query ) {
    error_log << QObject::tr("Не создан запрос = '%1'").arg(qname);
    return false;
  }
  QString filename; 
  int64_t fileid = -1; 
  query->arg( "tablename", prefix_ );
  if ( true == doc.HasMember("filename") ) {
    filename = doc["filename"].GetString();
  }
  if ( true == doc.HasMember("_id") ) {
    QString strid = doc["_id"].GetString();
    fileid = strid.toLongLong();
  }
  if ( false == filename.isEmpty() ) {
    query->arg( "filename", filename );
  }
  if ( -1 != fileid ) {
    query->arg( "id", fileid );
  }
  if ( false == query->exec() ) {
    error_log << query->errorMessage();
    error_log << QObject::tr("Ошибка выполнения запроса = '%1'").arg( query->query() );
    return false;
  }
  if ( false == query->initIterator() ) {
    error_log << QObject::tr("Пустой ответ на запрос = %1")
      .arg( query->query() );
    return false;
  }
  while ( true == query->next() ) {
    const DbiEntry& entry = query->entry();
    files_.append( entry.valueInt64("id") );
  }
  return true;
}


bool LocalFileList::findByName( const QString& filename )
{
  files_.clear();
  currentpos_ = -1;
  if ( nullptr == client_ || false == client_->connected() ) {
    error_log << QObject::tr("Нет подключения к БД!");
  }
  QString qname("find_file_list");
  auto query = client_->queryptrByName(qname);
  if ( nullptr == query ) {
    error_log << QObject::tr("Не создан запрос = '%1'").arg(qname);
    return false;
  }
  query->arg( "tablename", prefix_ );
  if ( false == filename.isEmpty() ) {
    query->arg( "filename", filename );
  }
  if ( false == query->exec() ) {
    error_log << query->errorMessage();
    error_log << QObject::tr("Ошибка выполнения запроса = '%1'").arg( query->query() );
    return false;
  }
  if ( false == query->initIterator() ) {
    error_log << QObject::tr("Пустой ответ на запрос = %1")
      .arg( query->query() );
    return false;
  }
  while ( true == query->next() ) {
    const DbiEntry& entry = query->entry();
    files_.append( entry.valueInt64("id") );
  }
  return true;
}

bool LocalFileList::isValid() const
{
  return ( nullptr != client_ );
}

bool LocalFileList::hasList() const
{
  return ( 0 < files_.size() );
}

GridFile LocalFileList::file() const
{
  if ( currentpos_ >= files_.size() || 0 > currentpos_ ) {
    return GridFile();
  }
  Dbi* client = Psql::createDatabase( client_->params() );
  LocalFile* lc = new LocalFile( files_[currentpos_], prefix_, client );
  return GridFile(lc);
}

bool LocalFileList::next()
{
  ++currentpos_;
  return ( currentpos_ < files_.size() );
}

}
