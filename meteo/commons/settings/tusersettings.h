#pragma once

#include "settings.h"
#include <cross-commons/singleton/tsingleton.h>
#include <qstringlist.h>


namespace meteo {

namespace internal {

class TUserSettings
{
  friend class TSingleton<TUserSettings>;

public :
  typedef QMap<int, QString> dict_t;

public :
  bool load();

  bool loadRoles();
  bool loadRanks();
  bool loadDepartments();
  bool loadUsers();

  bool isRoot() const;
  bool isVgmdaemon() const;
  QString getCurrentHostName() const;

  inline QStringList usernameList() const { return users_.keys(); }
  inline QMap<QString, app::User>& userList() { return users_; }

  void setCurrentUserLogin(const QString& login);
  QString getCurrentUserLogin();
  QString getCurrentUserName();
  QString getCurrentUserSurname();
  QString getCurrentUserShortName();
  QString getCurrentUserDepartment();
  QString getCurrentUserRank();

  QString getUserShortName(const QString& username) const;

  app::User getUser(const QString& username, bool* ok = nullptr) const;
  bool checkUser(const QString& username);
  bool blockUser(const QString& username, bool isBlock);

  bool appendUser(const app::User& user);
  bool updateUser(const app::User& user);

  inline dict_t roles() { return roles_; }
  dict_t genders() const;

  inline dict_t& departments() { return departments_; }
  bool putDepartment(int id, const QString& name);

  inline dict_t& ranks() { return ranks_; }
  bool putRank(int id, const QString& name);
  QString getRank(int code) const;

  bool authUser(const QString& username, const QString& password);
  bool isAuth();

private :
  TUserSettings();
  ~TUserSettings();

  inline QString getUserGroup() const { return "meteousers"; }
  inline QString getAdminGroup() const { return "meteoadministrators"; }

  bool isUnlocked(const QString& username);

  bool blockUser(const QString& username);
  bool unlockUser(const QString& username);

private :
  QString selfSysUser_;

  QMap<QString, app::User> users_;
  dict_t roles_;
  dict_t ranks_;
  dict_t departments_;
};

}

typedef TSingleton<internal::TUserSettings> TUserSettings;

}
