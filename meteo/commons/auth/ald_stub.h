#pragma once

#include "pam.h"


namespace meteo {

namespace auth {

class AldStubEngine :
  public Engine
{
public :
  AldStubEngine(const auth::AuthConf& conf);
  virtual ~AldStubEngine();

  virtual QString className() const override { return "ald_stub"; }

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

private :
  auth::AldUsersConfig* aldConf_;
  PamEngine* pam_;
};

}

}
