#pragma once

#include "engine.h"
#include <qdatetime.h>


namespace meteo {

namespace auth {

class UserCache
{
public :
  static const int CACHE_TTL = 60000; // 60 сек

  typedef struct {
    bool isAdmin;
    QString home;
    Gecos gecos;
    QStringList groups;
  } item_t;

public :
  UserCache();
  ~UserCache();

  bool isValid();
  void reload(Engine* impl);

  item_t value(const QString& username);
  QStringList keys() const;

private :
  QDateTime lastReload_;
  QMap<QString, item_t> cache_;
};


class CacheEngine :
  public Engine
{
public :
  CacheEngine(Engine* impl);
  virtual ~CacheEngine();

  virtual QString className() const override;

  virtual bool init() override;

  virtual bool isAdmin(const QString& username) override;
  virtual QString homeDir(const QString& username) override;

  virtual QStringList userList() override;
  virtual QStringList groupList(const QString& username) override;

  virtual bool tryAuth(const QString& username, const QString& password) override;
  virtual bool registry(const QString& username, const QString& password) override;

  virtual bool getGecos(const QString& username, Gecos& gecos) override;
  virtual bool setGecos(const QString& username, const Gecos& gecos) override;

  virtual QString lastError() override;
  virtual void setLastError(const QString& msg) override;
  virtual bool hasError() const override;

private :
  Engine* impl_;
  UserCache cache_;
};

}

}
