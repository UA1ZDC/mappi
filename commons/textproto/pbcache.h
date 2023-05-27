#ifndef COMMONS_TEXTPROTO_PBCACHE_H
#define COMMONS_TEXTPROTO_PBCACHE_H

#include <qstring.h>

namespace google {
namespace protobuf {
class Message;
}
}

namespace pbtools {

//!
class PbCache
{
public:
  explicit PbCache(const QString& serviceName, const QString& rpcName, int lifeTimeSec = 36000);

  void setCachePath(const QString& path) { path_ = path; }
  void setLifeTime(int sec) { lifeTime_ = sec; }

  bool read(const ::google::protobuf::Message& request, ::google::protobuf::Message* reply) const;
  bool write(const ::google::protobuf::Message& request, const ::google::protobuf::Message& reply) const;
  bool checkCache(const ::google::protobuf::Message& request) const;

private:
  QString md5hash(const ::google::protobuf::Message& msg) const;
  QString cacheFileName(const ::google::protobuf::Message& msg) const;

private:
  // параметры
  QString path_;
  int lifeTime_;
  QString service_;
  QString rpc_;
};

}

#endif
