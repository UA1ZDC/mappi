#include "cache.h"
// #include <QtDebug>


namespace meteo {

namespace auth {

// class UserCache
UserCache::UserCache()
{
}

UserCache::~UserCache()
{
}

bool UserCache::isValid()
{
  if (cache_.isEmpty()) {
    debug_log << QObject::tr("[auth:cache] нет данных");
    return false;
  }

  if (UserCache::CACHE_TTL < lastReload_.msecsTo(QDateTime::currentDateTimeUtc())) {
    debug_log << QObject::tr("[auth:cache] expired ttl");
    return false;
  }

  return true;
}

void UserCache::reload(Engine* impl)
{
  debug_log << QObject::tr("[auth:cache] перезагрузка");
  cache_.clear();

  for (auto username : impl->userList()) {
    item_t item;

    item.isAdmin = impl->isAdmin(username);
    item.home = impl->homeDir(username);
    item.groups = impl->groupList(username);
    // WARNING осмысленно не проверяется
    impl->getGecos(username, item.gecos);

    // debug_log << "is admin:" << item.isAdmin;
    // debug_log << "home:" << item.home;
    // debug_log << "groups:" << item.groups;
    // debug_log << "gecos:" << item.gecos.toString();

    cache_.insert(username, item);
  }

  lastReload_ = QDateTime::currentDateTimeUtc();
}

UserCache::item_t UserCache::value(const QString& username)
{
  return cache_[username];
}

QStringList UserCache::keys() const
{
  return cache_.keys();
}


// class CacheEngine
CacheEngine::CacheEngine(Engine* impl) :
  impl_(impl)
{
}

CacheEngine::~CacheEngine()
{
  delete impl_;
  impl_ = nullptr;
}

QString CacheEngine::className() const
{
  return QString("cache:%1").arg(impl_->className());
}

bool CacheEngine::init()
{
  if (impl_ == nullptr)
    return false;

  return impl_->init();
}

bool CacheEngine::isAdmin(const QString& username)
{
  if (!cache_.isValid())
    cache_.reload(impl_);

  return cache_.value(username).isAdmin;
}

QString CacheEngine::homeDir(const QString& username)
{
  if (!cache_.isValid())
    cache_.reload(impl_);

  return cache_.value(username).home;
}

QStringList CacheEngine::userList()
{
  if (!cache_.isValid())
    cache_.reload(impl_);

  return cache_.keys();
}

QStringList CacheEngine::groupList(const QString& username)
{
  if (!cache_.isValid())
    cache_.reload(impl_);

  return cache_.value(username).groups;
}

bool CacheEngine::tryAuth(const QString& username, const QString& password)
{
  return impl_->tryAuth(username, password);
}

bool CacheEngine::registry(const QString& username, const QString& password)
{
  if (impl_->registry(username, password)) {
    cache_.reload(impl_);
    return true;
  }

  return false;
}

bool CacheEngine::getGecos(const QString& username, Gecos& gecos)
{
  if (!cache_.isValid())
    cache_.reload(impl_);

  gecos = cache_.value(username).gecos;
  return true;
}

bool CacheEngine::setGecos(const QString& username, const Gecos& gecos)
{
  if (impl_->setGecos(username, gecos)) {
    cache_.reload(impl_);
    return true;
  }

  return false;
}

QString CacheEngine::lastError()
{
  return impl_->lastError();
}

void CacheEngine::setLastError(const QString& msg)
{
  return impl_->setLastError(msg);
}

bool CacheEngine::hasError() const
{
  return impl_->hasError();
}

}

}
