#include "tusersettings.h"
#include <cross-commons/app/paths.h>
#include <cross-commons/debug/tlog.h>
#include <commons/textproto/tprototext.h>
#include <sql/nosql/nosqlquery.h>
#include <qhostinfo.h>
#include <pwd.h>


namespace meteo {

template<>
internal::TUserSettings* TUserSettings::_instance = 0;

static const char* VGM_DAEMON = "vgmdaemon";
static const QString FORBIDDEN = QObject::tr("Ошибка доступа. Обратитесь к администратору");

namespace internal {

static rpc::Channel* channel()
{
  Settings* settings = gSettings();
  if (settings == nullptr) {
    error_log << QObject::tr("Не удалось загрузить настройки");
    return nullptr;
  }

  bool isOk = false;
  QString addr = settings->serviceAddress(settings::proto::kDiagnostic, &isOk);
  if (!isOk) {
    error_log << QObject::tr("Не удалось получить адрес сервиса '%1'")
      .arg(settings::proto::kDiagnostic);

    return nullptr;
  }

  rpc::Channel* ch = rpc::Channel::connect(addr);
  if (ch == nullptr)
    error_log << QObject::tr("Сервис диагностики не доступен");

  return ch;
}

TUserSettings::TUserSettings()
{
}

TUserSettings::~TUserSettings()
{
}

bool TUserSettings::load()
{
  return (loadRoles() &&
    loadRanks() &&
    loadDepartments() &&
    loadUsers());
}

bool TUserSettings::loadRoles()
{
  roles_.clear();

  QScopedPointer<rpc::Channel> ch(channel());
  if (ch.isNull())
    return false;

  app::Dummy req;
  QScopedPointer<app::RoleResponse> resp(ch->remoteCall(&app::ControlService::RoleList, req, 1000000));
  if (resp.isNull())
    return false;

  if (!resp->result()) {
    error_log << resp->comment();
    return false;
  }

  for (int i = 0, sz = resp->list_size(); i < sz; ++i) {
    app::Role item = resp->list(i);
    roles_[item.id()] = item.name().c_str();
  }

  return true;
}

bool TUserSettings::loadRanks()
{
  ranks_.clear();

  QScopedPointer<rpc::Channel> ch(channel());
  if (ch.isNull())
    return false;

  app::Dummy req;
  QScopedPointer<app::RankResponse> resp(ch->remoteCall(&app::ControlService::RankList, req, 1000000));
  if (resp.isNull())
    return false;

  if (!resp->result()) {
    error_log << resp->comment();
    return false;
  }

  for (int i = 0, sz = resp->list_size(); i < sz; ++i) {
    app::Rank item = resp->list(i);
    ranks_[item.id()] = item.name().c_str();
  }

  return true;
}

bool TUserSettings::loadDepartments()
{
  departments_.clear();

  QScopedPointer<rpc::Channel> ch(channel());
  if (ch.isNull())
    return false;

  app::Dummy req;
  QScopedPointer<app::DepartmentResponse> resp(ch->remoteCall(&app::ControlService::DepartmentList, req, 100000));
  if (resp.isNull())
    return false;

  if (!resp->result()) {
    error_log << resp->comment();
    return false;
  }

  for (int i = 0, sz = resp->list_size(); i < sz; ++i) {
    app::Department item = resp->list(i);
    departments_[item.id()] = item.name().c_str();
  }

  return true;
}

bool TUserSettings::loadUsers()
{
  users_.clear();

  QScopedPointer<rpc::Channel> ch(channel());
  if (ch.isNull())
    return false;

  app::Dummy req;
  QScopedPointer<app::UserResponse> resp(ch->remoteCall(&app::ControlService::UserList, req, 1000000));
  if (resp.isNull())
    return false;

  if (!resp->result()) {
    error_log << resp->comment();
    return false;
  }

  for (int i = 0, sz = resp->list_size(); i < sz; ++i) {
    app::User item = resp->list(i);

    app::Department* department = item.mutable_department();
    department->set_name(departments_.value(department->id(), QString()).toStdString());

    app::Rank* rank = item.mutable_rank();
    rank->set_name(ranks_.value(rank->id(), QString()).toStdString());

    app::Role* role = item.mutable_role();
    role->set_name(roles_[role->id()].toStdString());

    users_[item.username().c_str()] = item;
  }

  return true;
}

bool TUserSettings::isRoot() const
{
  return (::getuid() == 0);
}

bool TUserSettings::isVgmdaemon() const
{
  struct passwd* pw = ::getpwnam(VGM_DAEMON);
  return (pw && (::getuid() == pw->pw_uid));
}

QString TUserSettings::getCurrentHostName() const
{
  return QHostInfo::localHostName();
}

void TUserSettings::setCurrentUserLogin(const QString& login)
{
  selfSysUser_ = login;
}

QString TUserSettings::getCurrentUserLogin()
{
  if (selfSysUser_.isEmpty()) {
    uid_t uid = ::getuid();
    struct passwd* pw = ::getpwuid(uid);
    if (pw != nullptr)
      selfSysUser_ = pw->pw_name;
  }

  return selfSysUser_;
}

QString TUserSettings::getCurrentUserName()
{
  QString username = getCurrentUserLogin();
  if (!username.isEmpty() && users_.contains(username)) {
    const app::User& user = users_[username];
    return QString("%1 %2 %3")
      .arg(user.surname().c_str())
      .arg(user.name().c_str())
      .arg(user.patron().c_str());
  }

  return username;
}

QString TUserSettings::getCurrentUserSurname()
{
  QString username = getCurrentUserLogin();
  if (!username.isEmpty() && users_.contains(username))
    return users_[username].surname().c_str();

  return QString();
}

QString TUserSettings::getCurrentUserShortName()
{
  return getUserShortName(getCurrentUserLogin());
}

QString TUserSettings::getCurrentUserDepartment()
{
  QString username = getCurrentUserLogin();
  if (!username.isEmpty() && users_.contains(username))
    return users_[username].department().name().c_str();

  return QString();
}

QString TUserSettings::getCurrentUserRank()
{
  QString username = getCurrentUserLogin();
  if (!username.isEmpty() && users_.contains(username))
    return users_[username].rank().name().c_str();

  return QString();
}

QString TUserSettings::getUserShortName(const QString& username) const
{
  if (!username.isEmpty() && users_.contains(username)) {
    const app::User& user = users_[username];
    if (!user.surname().empty()) {
      return QString("%1 %2.%3.")
        .arg(user.surname().c_str())
        .arg(QString::fromStdString(user.name()).left(1).toUpper())
        .arg(QString::fromStdString(user.patron()).left(1).toUpper());
    }
 }

 return QString();
}

app::User TUserSettings::getUser(const QString& username, bool* ok /*=*/) const
{
  if (ok != nullptr)
    *ok = users_.contains(username);

 return users_.value(username, app::User());
}

bool TUserSettings::checkUser(const QString& username)
{
  QScopedPointer<rpc::Channel> ch(channel());
  if (ch.isNull())
    return false;

  app::User user;
  user.set_username(username.toStdString());

  app::UserRequest req;
  req.mutable_user()->CopyFrom(user);
  QScopedPointer<app::UserResponse> resp(ch->remoteCall(&app::ControlService::CheckUser, req, 1000000));
  if (resp.isNull())
    return false;

  if (!resp->result()) {
    error_log << resp->comment();
    return false;
  }

  return resp->result();
}

bool TUserSettings::blockUser(const QString& username, bool isBlock)
{
  if (isBlock)
    return blockUser(username);

  return unlockUser(username);
}

bool TUserSettings::appendUser(const app::User& user)
{
  QScopedPointer<rpc::Channel> ch(channel());
  if (ch.isNull())
    return false;

  app::UserRequest req;
  req.mutable_user()->CopyFrom(user);

  QScopedPointer<app::UserResponse> resp(ch->remoteCall(&app::ControlService::NewUser, req, 1000000));
  if (resp.isNull())
    return false;

  if (!resp->result()) {
    error_log << resp->comment();
    return false;
  }

  users_[user.username().c_str()] = user;

  return true;
}

bool TUserSettings::updateUser(const app::User& user)
{
  QScopedPointer<rpc::Channel> ch(channel());
  if (ch.isNull())
    return false;

  app::UserRequest req;
  req.mutable_user()->CopyFrom(user);

  QScopedPointer<app::UserResponse> resp(ch->remoteCall(&app::ControlService::UpdateUser, req, 1000000));
  if (resp.isNull())
    return false;

  if (!resp->result()) {
    error_log << resp->comment();
    return false;
  }

  users_[user.username().c_str()] = user;

  return true;
}

TUserSettings::dict_t TUserSettings::genders() const
{
  static TUserSettings::dict_t genders;
  if (genders.isEmpty()) {
    genders.insert(app::User::GenderType::User_GenderType_Female, QObject::tr("Женский"));
    genders.insert(app::User::GenderType::User_GenderType_Male, QObject::tr("Мужской"));
  }

  return genders;
}

bool TUserSettings::putDepartment(int id, const QString& name)
{
  QScopedPointer<rpc::Channel> ch(channel());
  if (ch.isNull())
    return false;

  app::Department req;
  req.set_id(id);
  req.set_name(name.toStdString());

  QScopedPointer<app::DefaultResponce> resp(ch->remoteCall(&app::ControlService::PutDepartment, req, 1000000));
  if (resp.isNull())
    return false;

  if (!resp->result()) {
    error_log << resp->comment();
    return false;
  }

  departments_[id] = name;

  return true;
}

bool TUserSettings::putRank(int id, const QString& name)
{
  QScopedPointer<rpc::Channel> ch(channel());
  if (ch.isNull())
    return false;

  app::Rank req;
  req.set_id(id);
  req.set_name(name.toStdString());

  QScopedPointer<app::DefaultResponce> resp(ch->remoteCall(&app::ControlService::PutRank, req, 1000000));
  if (resp.isNull())
    return false;

  if (!resp->result()) {
    error_log << resp->comment();
    return false;
  }

  ranks_[id] = name;

  return true;
}

QString TUserSettings::getRank(int code) const
{
  return ranks_.value(code, QString());
}

bool TUserSettings::authUser(const QString& username, const QString& password)
{
  QScopedPointer<rpc::Channel> ch(channel());
  if (ch.isNull())
    return false;

  app::UserAuthRequest req;
  req.set_username(username.toStdString());
  req.set_password(password.toStdString());
  QScopedPointer<app::UserAuthResponse> resp(ch->remoteCall(&app::ControlService::AuthUser, req, 1000000));
  if (resp.isNull())
    return false;

  if (!resp->result()) {
    error_log << resp->comment();
    return false;
  }

  // проверка доступа на уровне подсистемы аутентификации
  if (!resp->status()) {
    error_log << FORBIDDEN;
    return false;
  }

  return isUnlocked(username);
}

bool TUserSettings::isAuth()
{
  return isUnlocked(getCurrentUserLogin());
}

bool TUserSettings::isUnlocked(const QString& username)
{
  if (!users_.contains(username)) {
    error_log << FORBIDDEN;
    return false;
  }

  // проверка доступа на уровне блокировки в БД
  return !users_[username].block();
}

bool TUserSettings::blockUser(const QString& username)
{
  QScopedPointer<rpc::Channel> ch(channel());
  if (ch.isNull())
    return false;

  app::User user;
  user.CopyFrom(getUser(username));
  // user.set_block(true);

  app::UserRequest req;
  req.mutable_user()->CopyFrom(user);
  QScopedPointer<app::UserResponse> resp(ch->remoteCall(&app::ControlService::BlockUser, req, 1000000));
  if (resp.isNull())
    return false;

  if (!resp->result()) {
    error_log << resp->comment();
    return false;
  }

  users_[username].set_block(true);

  return true;
}

bool TUserSettings::unlockUser(const QString& username)
{
  QScopedPointer<rpc::Channel> ch(channel());
  if (ch.isNull())
    return false;

  app::User user;
  user.CopyFrom(getUser(username));
  // user.set_block(false);

  app::UserRequest req;
  req.mutable_user()->CopyFrom(user);
  QScopedPointer<app::UserResponse> resp(ch->remoteCall(&app::ControlService::UnlockUser, req, 1000000));
  if (resp.isNull())
    return false;

  if (!resp->result()) {
    error_log << resp->comment();
    return false;
  }

  users_[username].set_block(false);

  return true;
}

}

}
