#include "localfile.h"
#include "localfs.h"

#include <qfileinfo.h>
#include <qfile.h>
#include <qdir.h>
#include <qfile.h>
#include <qelapsedtimer.h>
#include <qcryptographichash.h>

#include <string>
#include <fstream>
#include <streambuf>

#include <cross-commons/debug/tlog.h>
#include <sql/dbi/dbi.h>
#include <sql/dbi/dbiquery.h>
#include <sql/dbi/dbientry.h>

namespace meteo {

LocalFile::LocalFile( const QString& filename, const QString& prefix, Dbi* client )
  : filename_(filename),
  prefix_(prefix),
  client_(client),
  dbname_( client->dbname() )
{
  loadFileInfo( filename_, "" );
}

LocalFile::LocalFile( int64_t id, const QString& prefix, Dbi* client )
  : prefix_(prefix),
  client_(client),
  dbname_( client->dbname() )
{
  loadFileInfo( "", QString::number(id)  );
}

LocalFile::LocalFile( const rapidjson::Document& doc, const QString& prefix, Dbi* client )
  : prefix_(prefix),
  client_(client),
  dbname_( client->dbname() )
{
  if ( true == doc.HasMember("filename") ) {
    filename_ = doc["filename"].GetString();
    loadFileInfo( filename_, "" );
  }
  else if ( true == doc.HasMember("_id") ) {
    QString strid = doc["_id"].GetString();
    id_ = strid.toLongLong();
    loadFileInfo( "", strid );
  }
}

LocalFile::LocalFile( const QString& filename, const QByteArray& data, const QString& prefix, Dbi* client )
  : filename_(filename),
  data_(data),
  prefix_(prefix),
  client_(client),
  size_( data.size() ),
  dbname_( client->dbname() )
{
  md5_ = QCryptographicHash::hash( data_, QCryptographicHash::Md5 ).toHex();
}

LocalFile::~LocalFile()
{
  delete file_; file_ = nullptr;
}

bool LocalFile::isValid() const
{
  return ( false == filename_.isEmpty() );
}

bool LocalFile::hasFile() const
{
  return QFile::exists( LocalFs::fileNameFs( dbname_, prefix_, filename_, id_ ) );
}

QString LocalFile::id() const
{
  if ( 0 > id_ ) {
    return QString();
  }
  return QString::number(id_);
}

int64_t LocalFile::size() const
{
  return size_;
}

bool LocalFile::setMd5( const QString& md5 )
{
  md5_ = md5;
  return true;
}

QString LocalFile::md5() const
{
  if ( !hasFile() ) { return QString(); }

  return md5_;
}

QString LocalFile::contentType() const
{
  warning_log << QObject::tr("Не реализовано!");
  return QString();
}

bool LocalFile::setContentType(const QString& type)
{
  Q_UNUSED(type);
  warning_log << QObject::tr("Не реализовано!");
  return false;
}

QDateTime LocalFile::uploadDate() const
{
  QFileInfo fi(filename_);
  if ( false == fi.exists() ) {
    return QDateTime();
  }
  return fi.lastModified();
}

QString LocalFile::fileName() const
{
  return filename_;
}

bool LocalFile::setFileName(const QString& fn )
{
  filename_ = fn;
  return true;
}

int64_t LocalFile::write(const QByteArray& data, bool truncate )
{
  if ( !isValid() ) { return -1; }

  QByteArray md5new = QCryptographicHash::hash( data, QCryptographicHash::Md5 ).toHex();
  QString fn = LocalFs::fileNameFs( dbname_, prefix_, filename_, id_ );
  if ( true == truncate && md5new == md5_ ) {
    debug_log << QObject::tr("Дубликат не записывется!");
    return -2; //FIXME реализовать коды ошибок!
  }

  if ( nullptr != file_ ) {
    if ( ( false == truncate && 0 == ( QIODevice::Append & file_->openMode() ) )
      || ( true == truncate && 0 == ( QIODevice::Truncate & file_->openMode() ) ) ) {
      file_->close();
      delete file_; file_ = nullptr;
    }
  }
  if ( nullptr == file_ ) {
    QIODevice::OpenMode flags = QIODevice::ReadWrite;
    flags |= ( true == truncate ) ? QIODevice::Truncate : QIODevice::Append;
    file_ = new QFile(fn);
    if ( false == file_->open(flags) ) {
      lastError_ = file_->errorString();
      return -1;
    }
  }

  if ( true == truncate ) {
    debug_log << QObject::tr("Перезапись файла! = %1").arg(fn);
    file_->resize(0);
    data_ = data;
  }
  else {
    if ( 0 == data_.size() ) {
      data_ = file_->readAll();
    }
    debug_log << QObject::tr("Дозапись в существующий файл! %1").arg(fn);
    data_ += data;
  }


  md5new = QCryptographicHash::hash( data_, QCryptographicHash::Md5 ).toHex();
  md5_ = md5new;
  int64_t sz = file_->write(data);
  updateDbEntry();
  return sz;
}

QByteArray LocalFile::readAll(bool* ok) const
{
  if ( nullptr != ok ) {
    *ok = false;
  }
  QString fn = LocalFs::fileNameFs( dbname_, prefix_, filename_, id_ );
 /* QFileInfo fi(fn);
  if ( false == fi.exists() ) {
    return QByteArray();
  }*/
 /* ///>>>>>>>>>TEST
  QElapsedTimer timer; timer.start();

  std::ifstream t(fn.toStdString());
   if(false == t.is_open()){
     return QByteArray();
   }
  std::string str;

  t.seekg(0, std::ios::end);
  str.reserve(t.tellg());
  t.seekg(0, std::ios::beg);

  str.assign((std::istreambuf_iterator<char>(t)),
              std::istreambuf_iterator<char>());

  return QByteArray(str.c_str(), str.length());
  ///<<<<<<<<<TEST
*/
  QFile file(fn);
  if ( false == file.open( QIODevice::ReadWrite) ) {
    lastError_ = file.errorString();
    return QByteArray();
  }
  if ( nullptr != ok ) {
    *ok = true;
  }
  return file.readAll();
}

QByteArray LocalFile::read(int64_t maxSize, bool* ok) const
{
  if ( nullptr != ok ) { *ok = false; }

  if ( !hasFile() ) { return QByteArray(); }

  if ( nullptr == file_ ) {
    QString fn = LocalFs::fileNameFs( dbname_, prefix_, filename_, id_ );
    file_ = new QFile(fn);
    if ( false == file_->open(QIODevice::ReadWrite) ) {
      lastError_ = file_->errorString();
      return QByteArray();
    }
  }

  int64_t sz = maxSize;
  if ( sz < 0 ) {
    sz = size();
  }

  QByteArray raw = file_->read(maxSize);

  if ( nullptr != ok ) { *ok = true; }
  return raw;
}

uint64_t LocalFile::position(bool* ok) const
{
  if ( nullptr != ok ) { *ok = false; }
  if ( nullptr == file_ ) {
    return 0;
  }
  if ( nullptr != ok ) { *ok = true; }
  return static_cast<uint64_t>(file_->pos());
}

bool LocalFile::seek(int64_t pos)
{
  if ( false == hasFile() ) {
    return false;
  }
  if ( false == QFile::exists( LocalFs::fileNameFs( dbname_, prefix_, filename_, id_ ) ) ) {
    return false;
  }
  if ( nullptr == file_ ) {
    QString fn = LocalFs::fileNameFs( dbname_, prefix_, filename_, id_ );
    file_ = new QFile(fn);
    if ( false == file_->open(QIODevice::ReadWrite) ) {
      lastError_ = file_->errorString();
      return false;
    }
  }
  return file_->seek(pos);
}

bool LocalFile::remove()
{
  bool res = removeFromDb();
  if ( true == res ) {
    QString fn = LocalFs::fileNameFs( dbname_, prefix_, filename_, id_ );
    QFile f;
    res = f.remove(fn);
    lastError_ = f.errorString();
  }
  return res;
}

bool LocalFile::removeFromDb()
{
  if ( nullptr == client_ ) {
    lastError_ = QObject::tr("Нет соединения с БД");
    error_log << lastError_;
    return false;
  }
  if ( false == hasFile() || false == isValid() ) {
    lastError_ = QObject::tr("Файл не корректный");
    error_log << lastError_;
    return false;
  }
  QString qname("remove_file_description");
  auto query = client_->queryptrByName(qname);
  if ( nullptr == query ) {
    lastError_ = QObject::tr("Не создан запрос = '%1'").arg(qname);
    error_log << lastError_;
    return false;
  }
  query->arg( "tablename", LocalFile::fs_table_name(prefix_) );
  query->arg( "id", id_ );
  if ( false == query->exec() ) {
    lastError_ = QObject::tr("Ошибка выполнения запроса = '%1'").arg( query->query() );
    return false;
  }
  const DbiEntry& res = query->result();
  if ( false == res.hasField("res") ) {
    lastError_ = QObject::tr("Ошибка обработки результатов запроса при удалении описания файла");
    return false;
  }

  return res.valueBool("res");
}

bool LocalFile::save( bool* dup_md5 )
{
  if (false == put2db( dup_md5 ) ) {
    return false;
  }
  if ( false == put2fs() ) {
    remove();
    return false;
  }
  return true;
}

bool LocalFile::ensurePathExists()
{
  if ( true == filename_.isEmpty() ) {
    lastError_ = QObject::tr("Не задано имя");
    return false;
  }
  QString fn = LocalFs::fileNameFs( dbname_, prefix_, filename_, id_ );
  QFileInfo fi(fn);
  QString path = fi.absolutePath();
  QDir dir;
  return dir.mkpath(path);
}

bool LocalFile::put2fs()
{
  if ( true == filename_.isEmpty() ) {
    lastError_ = QObject::tr("Не задано имя");
    return false;
  }
  if ( false == ensurePathExists() ) {
    return false;
  }
  QString fn = LocalFs::fileNameFs( dbname_, prefix_, filename_, id_ );
  QFile file(fn);
  if ( false == file.open( QIODevice::ReadWrite | QIODevice::Truncate ) ) {
    error_log << QObject::tr("Ошибка открытия файла '%1' для записи. Ошибка = %2")
      .arg(filename_)
      .arg( file.errorString() );
    lastError_ = file.errorString();
    return false;
  }
  file.write(data_);
  return true;
}

bool LocalFile::put2db( bool* dup_md5 )
{
  *dup_md5 = false;
  if ( nullptr == client_ || false == client_->connected() ) {
    lastError_ = QObject::tr("Нет подключения к БД!");
  }
  QString qname("put_file_description");
  auto query = client_->queryptrByName(qname);
  if ( nullptr == query ) {
    lastError_ = QObject::tr("Не создан запрос = '%1'").arg(qname);
    error_log << lastError_;
    return false;
  }

  query->arg( "filename", filename_ )
    .arg( "tablename", LocalFile::fs_table_name(prefix_) )
    .arg( "md5", md5_ )
    .arg( "size", size_ );
  if ( false == query->exec() ) {
    lastError_ = QObject::tr("Ошибка выполнения запроса = '%1'").arg( query->query() );
    return false;
  }
  const DbiEntry& entry = query->result();
  id_ = entry.valueInt64("id");
  size_ = entry.valueInt32("size");
  filename_ = entry.valueString("name");
  md5_ = entry.valueString("md5");
  if ( true == entry.valueBool("dup") ) {
    lastError_ = QObject::tr("Ошибка ункальности файла по md5");
    *dup_md5 = true;
    return false;
  }
  return true;
}

bool LocalFile::updateDbEntry()
{
  if ( nullptr == client_ || false == client_->connected() ) {
    lastError_ = QObject::tr("Нет подключения к БД!");
  }
  QString qname("update_file_description");
  auto query = client_->queryptrByName(qname);
  if ( nullptr == query ) {
    lastError_ = QObject::tr("Не создан запрос = '%1'").arg(qname);
    error_log << lastError_;
    return false;
  }

  query->arg( "filename", filename_ )
    .arg( "tablename", LocalFile::fs_table_name(prefix_) )
    .arg( "md5", md5_ )
    .arg( "size", size_ )
    .arg( "id", id_ );
  if ( false == query->exec() ) {
    lastError_ = QObject::tr("Ошибка выполнения запроса = '%1'").arg( query->query() );
    return false;
  }
  return true;

}

bool LocalFile::loadFileInfo( const QString& filename, const QString& fileid )
{
  if ( nullptr == client_ || false == client_->connected() ) {
    lastError_ = QObject::tr("Нет подключения к БД!");
    return false;
  }
  QString qname("find_file_description");
  auto query = client_->queryptrByName(qname);
  if ( nullptr == query ) {
    lastError_ = QObject::tr("Не создан запрос = '%1'").arg(qname);
    error_log << lastError_;
    return false;
  }
  query->arg( "tablename", LocalFile::fs_table_name(prefix_) );
  if ( false == filename.isEmpty() ) {
    query->arg( "filename", filename_ );
  }
  if ( false == fileid.isEmpty() ) {
    query->arg( "id", (int64_t)fileid.toLongLong() );
  }
  if ( false == query->exec() ) {
    lastError_ = query->errorMessage();
    error_log << QObject::tr("Ошибка выполнения запроса = '%1'").arg( query->query() );
    return false;
  }
  if ( false == query->initIterator() ) {
    lastError_ = QObject::tr("Пустой ответ на запрос = %1")
      .arg( query->query() );
    return false;
  }
  const DbiEntry& entry = query->result();
  id_ = entry.valueInt64("id");
  filename_ = entry.valueString("filename");
  md5_ = entry.valueString("filemd5");
  size_ = entry.valueInt32("filesize");

  return true;
}
QString LocalFile::fs_table_name( const QString& prefix )
{
  return QString("fs_%1").arg(prefix);
}

}
