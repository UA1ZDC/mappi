#pragma once

#include "pam.h"
#include <ald/ALDCoreUtils.h>
#include <ald/ALDObjects.h>


namespace meteo {

namespace auth {

class AldHealthCheck
{
public :
  static const int TIMEOUT = 1800000;   // 30 мин

public :
  AldHealthCheck();
  ~AldHealthCheck();

  inline ALD::CALDConnectionPtr connectPtr() { return aldConnect_; }

  bool connect(auth::AuthConf conf);
  bool isTimeout();

private :
  ALD::CALDConnectionPtr aldConnect_;   // shared_ptr
  QDateTime lastConnect_;
};


class AldEngine :
  public Engine
{
public :
  AldEngine(const auth::AuthConf& conf);
  virtual ~AldEngine();

  virtual QString className() const { return "ald"; }

  virtual bool init() override;

  virtual bool isAdmin(const QString& username) override;
  virtual QString homeDir(const QString& username) override;

  virtual QStringList userList() override;
  virtual QStringList groupList(const QString& username) override;

  virtual bool tryAuth(const QString& username, const QString& password) override;
  virtual bool registry(const QString& username, const QString& password) override;

  virtual bool getGecos(const QString& username, Gecos& gecos) override;
  virtual bool setGecos(const QString& username, const Gecos& gecos) override;

private :
  bool isSystemUser(const QString& username);
  bool checkConnect();

  ALD::CALDUserPtr aldUser(const QString& username);

private :
  auth::AldUsersConfig* aldConf_;
  PamEngine* pam_;
  AldHealthCheck* healthCheck_;
  auth::AuthConf conf_;
};

}

}
