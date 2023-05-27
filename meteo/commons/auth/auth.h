#pragma once

#include "gecos.h"
#include "engine.h"
#include <qmutex.h>


namespace meteo {

namespace auth {

class Auth
{
public :
  enum WorkType {
    kDirect = 0,      // Запросы выполняются напрямую
    kSubprocess,      // Запросы выполняются в отдельном процессе
    kCachedDirect,    // Запросы выполняются напрямую, результаты кэшируются
    kCashedSubprocess // Запросы выполняются в отдельном процессе, результаты кэшируются
  };

public :
  Auth();
  ~Auth();

  Auth(const Auth& other) = delete;
  void operator=(const Auth& other) = delete;

  bool init(WorkType wtype);

  bool isAdmin(const QString& username);
  QString homeDir(const QString& username);

  QStringList userList();
  QStringList groupList(const QString& username);

  bool tryAuth(const QString& username, const QString& password);
  bool registry(const QString& username, const QString& password, QString* msg);

  bool getGecos(const QString& username, Gecos& gecos);
  bool setGecos(const QString& username, const Gecos& gecos);

private :
  QString logID() const;

private :
  auth::AuthConf conf_;
  Engine* impl_;
  QMutex mutex_;
};

}

};
