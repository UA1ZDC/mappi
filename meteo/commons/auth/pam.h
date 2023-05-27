#pragma once

#include "engine.h"
#include <pwd.h>


namespace meteo {

namespace auth {

class PamEngine :
  public Engine {

public :
  PamEngine();
  virtual ~PamEngine();

  virtual QString className() const override { return "pam"; }

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
  bool initPasswd(const QString& username, struct passwd* pwd);
  bool setPassword(const QString& username, const QString& password);

private :
  int bufferSize_;
  QByteArray buffer_;
};

}

}
