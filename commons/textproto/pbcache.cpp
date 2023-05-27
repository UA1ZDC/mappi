#include "pbcache.h"

#include <qdir.h>
#include <qfile.h>
#include <qdatetime.h>
#include <qcryptographichash.h>

#include <google/protobuf/message.h>

#include <cross-commons/debug/tlog.h>

namespace pbtools {

PbCache::PbCache(const QString& serviceName, const QString& rpcName, int lifeTimeSec)
{
  rpc_ = rpcName;
  service_ = serviceName;
  lifeTime_ = lifeTimeSec;

  path_ = QDir::tempPath() + "/pb_cache_data/";
}

bool PbCache::read(const google::protobuf::Message& request, google::protobuf::Message* reply) const
{
  return false;
  if ( false == checkCache(request) ) { return false; }

  QDir dir(path_);
  QString filename = cacheFileName(request);
  if ( 0 == filename.size() ) {
    error_log << QObject::tr("Пустое имя файла!");
    return false;
  }
  QFile file( dir.absoluteFilePath(filename) );
  if ( false == file.open(QFile::ReadOnly) ) {
    debug_log << "can't open file" << file.fileName();
    return false;
  }

  QByteArray ba = file.readAll();
  std::string s(ba.data(), ba.size());
  if ( false == reply->ParsePartialFromString(s) ) {
    error_log << QObject::tr("Не удалось прочитать прото-сообщение а файле = %1")
      .arg(file.fileName());
    return false;
  }

  return true;
}

bool PbCache::write(const google::protobuf::Message& request, const google::protobuf::Message& reply) const
{
  QDir dir(path_);
  if ( !dir.exists() ) {
    if ( !dir.mkpath(path_) ) {
      debug_log << "can't create:" << path_;
      return false;
    }
    QFile::Permissions all = QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner |
                             QFile::ReadUser  | QFile::WriteUser  | QFile::ExeUser  |
                             QFile::ReadGroup | QFile::WriteGroup | QFile::ExeGroup |
                             QFile::ReadOther | QFile::WriteOther | QFile::ExeOther;
    QFile::setPermissions(dir.absolutePath(), all);
  }

  QString filename = cacheFileName(request);
  if ( 0 == filename.size() ) {
    error_log << QObject::tr("Пустое имя файла");
    return false;
  }
  QFile file( dir.absoluteFilePath(filename) );

  if ( false == file.open(QFile::WriteOnly | QFile::Truncate) ) {
    debug_log << "can't open file for write:" << file.fileName();
    return false;
  }

  std::string s;
  if ( false == reply.SerializePartialToString(&s) ) {
    error_log << QObject::tr("Ошибка сериализации сообщения = \t\n") << reply.Utf8DebugString();
    return false;
  }
  QByteArray ba = QByteArray::fromRawData(s.data(), s.size());

  file.write(ba);
  file.setPermissions(file.permissions() | QFile::WriteUser | QFile::WriteGroup | QFile::WriteOther);

  file.close();

  return true;
}

bool PbCache::checkCache(const google::protobuf::Message& request) const
{
  QDir dir(path_);

  if ( !dir.exists() ) { return false; }
  QString filename = cacheFileName(request);
  if ( 0 == filename.size() ) {
    error_log << QObject::tr("Пустое имя файла");
    return false;
  }
  QFile file(dir.absoluteFilePath(filename));

  if ( !file.exists() ) { return false; }
  if ( !file.open(QFile::ReadOnly) ) { return false; }

  QFileInfo fi(file);
  if ( fi.created().addSecs(lifeTime_).secsTo(QDateTime::currentDateTime()) > 0 ) {
    file.remove();
    return false;
  }

  return true;
}

QString PbCache::md5hash(const google::protobuf::Message& msg) const
{
  std::string s;
  if ( false == msg.SerializePartialToString(&s) ) {
    error_log << QObject::tr("Ошибка сериализации сообщения = \t\n") << msg.Utf8DebugString();
    return QString();
  }
  QByteArray ba = QByteArray::fromRawData(s.data(), s.size());
  return QCryptographicHash::hash(ba, QCryptographicHash::Md5).toHex();
}

QString PbCache::cacheFileName(const google::protobuf::Message& msg) const
{
  QString md5 = md5hash(msg);
  if ( 0 == md5.size() ) {
    error_log << QObject::tr("Пустой md5");
    return QString();
  }
  return service_ + "_" + rpc_ + "_" + md5.left(10);
}

}
