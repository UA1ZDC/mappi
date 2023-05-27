#include "ald.h"
#include <cross-commons/app/waitloop.h>


namespace meteo {

namespace auth {

// class AldHealthCheck
/**
 * Класс необходим, т.к. чрез некоторое время (~3 часа) ALD соединение перестает работать, при этом нет уведомлений об ошибках\исключений.
 * Как вариант решения проблемы, спустя определенное время принудительно закрывать соедиение и открывать заново.
 */
AldHealthCheck::AldHealthCheck() :
  aldConnect_(nullptr)
{
}

AldHealthCheck::~AldHealthCheck()
{
  if (aldConnect_)
    aldConnect_->Disconnect();
}

bool AldHealthCheck::connect(auth::AuthConf conf)
{
  ALD::IALDCore* core = nullptr;

  // WARNING как показали эксперименты, стабильно каждая четная попытка подключения (после disconnect) оказывается неудачной
  static const int maxRetry = 2;
  for (int i = 0; i < maxRetry; ++i) {
    core = ALD::AllocateCore();
    ALD::InitCore(*core);

    core->UseInterface(LDAP_ITF_NAME, ALD_MODE_CLIENT);
    core->UseInterface(KADM5_ITF_NAME, ALD_MODE_ADMIN);

    warning_log << QObject::tr("[auth:ald] login попытка: %1/%2")
                     .arg(i + 1)
                     .arg(maxRetry);

    if (ALD::LoginAdmin(*core, conf.ald_admin(), conf.ald_password()) == true) {
      aldConnect_ = ALD::CALDConnectionPtr(new ALD::CALDConnection(*core, ALD::aatUser, ALD_CI_ALL));
      lastConnect_ = QDateTime::currentDateTimeUtc();

      return true;
    }

    ALD::FreeCore(core);
    core = nullptr;

    WaitLoop w;
    w.wait(500);
  }

  lastConnect_ = QDateTime::currentDateTimeUtc();
  aldConnect_.reset();

  return false;
}

bool AldHealthCheck::isTimeout()
{
  return (AldHealthCheck::TIMEOUT < lastConnect_.msecsTo(QDateTime::currentDateTimeUtc()));
}


// class AldEngine
AldEngine::AldEngine(const auth::AuthConf& conf):
  aldConf_(nullptr),
  pam_(new PamEngine()),
  healthCheck_(nullptr),
  conf_(conf)
{
}

AldEngine::~AldEngine()
{
  delete aldConf_;
  aldConf_ = nullptr;

  delete pam_;
  pam_ = nullptr;

  delete healthCheck_;
  healthCheck_ = nullptr;
}

bool AldEngine::isSystemUser(const QString& username)
{
  for (auto item : aldConf_->system_user()) {
    if (username == item.c_str())
      return true;
  }

  return false;
}

bool AldEngine::checkConnect()
{
  if (healthCheck_ != nullptr) {
    if (!healthCheck_->isTimeout())
      return (healthCheck_->connectPtr() != nullptr);

    delete healthCheck_;
  }

  healthCheck_ = new AldHealthCheck();
  return healthCheck_->connect(conf_);
}

ALD::CALDUserPtr AldEngine::aldUser(const QString& username)
{
  ALD::CALDUserPtr user;
  if (!checkConnect())
    return user;

  try {
    user = ALD::CALDUserPtr(new ALD::CALDUser(healthCheck_->connectPtr()));
    if (user->Get(username.toStdString()) == false) {
      user.reset();
      error_log << QObject::tr("[auth:ald] %1 не найден").arg(username);
    }

    if (user) {
      if (!user->IsValid()) {
        user.reset();
        error_log << QObject::tr("[auth:ald] %1 не подтвержден").arg(username);
      }
    }

  } catch (const ALD::EALDError& exp) {
    user.reset();
    setLastError(
      QObject::tr("%1 нет данных, исключение: %2")
        .arg(username)
        .arg(exp.what())
    );

  } catch (...) {
    user.reset();
    setLastError(
      QObject::tr("%1 нет данных, необрабатываемое исключение")
        .arg(username)
    );
  }

  return user;
}

bool AldEngine::init()
{
  aldConf_ = new auth::AldUsersConfig();
  QString path = QObject::tr("%1/ald-users.conf").arg(MnCommon::etcPath());
  if (!TProtoText::fromFile(path, aldConf_)) {
    aldConf_->Clear();
    return false;
  }

  return pam_->init();
}

bool AldEngine::isAdmin(const QString& username)
{
  if (isSystemUser(username))
    return pam_->isAdmin(username);

  if (checkConnect()) {
    ALD::CALDUserPtr user = aldUser(username);
    if (user)
      return user->IsAdm();
  }

  return false;
}

QString AldEngine::homeDir(const QString& username)
{
  if (isSystemUser(username))
    return pam_->homeDir(username);

  if (checkConnect()) {
    ALD::CALDUserPtr user = aldUser(username);
    if (user)
      return user->home().c_str();
  }

  return QString();
}

// WARNING
QStringList AldEngine::userList()
{
  QStringList res;
  if (!checkConnect())
    return res;

  try {
    ALD::ald_string_list userList;
    ALD::CALDDomain domain(healthCheck_->connectPtr());
    domain.EnumerateUsers(userList);
    for (auto item : userList)
      res << item.c_str();

  } catch (const ALD::EALDError& exp) {
    setLastError(QObject::tr("спискок пользователей, исключение: %1")
      .arg(exp.what())
    );

  } catch (...) {
    setLastError(
      QObject::tr("список пользователей, необрабатываемое исключение")
    );
  }

  // дописываем системных пользователей из конфигурации
  for (auto item : aldConf_->system_user())
    res << item.c_str();

  return res;
}

QStringList AldEngine::groupList(const QString& username)
{
  if (isSystemUser(username))
    return pam_->groupList(username);

  QStringList res;
  if (!checkConnect())
    return res;

  ALD::CALDUserPtr user = aldUser(username);
  if (user) try {
    ald_string_list groups;
    user->sgroups(groups);
    for (auto item : groups)
      res << item.c_str();

  } catch (const ALD::EALDError& exp) {
    setLastError(
      QObject::tr("%1 список групп, исключение: %2")
        .arg(username)
        .arg(exp.what())
    );

  } catch (...) {
    setLastError(
      QObject::tr("%1 список групп, необрабатываемое исключение")
        .arg(username)
    );
  }

  return res;
}

bool AldEngine::tryAuth(const QString& username, const QString& password)
{
  if (isSystemUser(username))
    return pam_->tryAuth(username, password);

  if (!checkConnect())
    return false;

  if (!userList().contains(username))
    return false;

  return pam_->tryAuth(username, password);
}

bool AldEngine::registry(const QString& username, const QString& password)
{
  if (isSystemUser(username))
    return pam_->registry(username, password);

  // WARNING пользователь сохраняется только в БД
  return checkConnect();
}

bool AldEngine::getGecos(const QString& username, Gecos& gecos)
{
  if (isSystemUser(username))
    return pam_->getGecos(username, gecos);

  if (checkConnect()) {
    ALD::CALDUserPtr user = aldUser(username);
    if (user) {
      gecos.fromString(user->gecos().c_str());
      return true;
    }
  }

  return false;
}

bool AldEngine::setGecos(const QString& username, const Gecos& gecos)
{
  if (isSystemUser(username))
    return pam_->setGecos(username, gecos);

  if (!checkConnect())
    return false;

  ALD::CALDUserPtr user = aldUser(username);
  if (user) try {
    user->Update(ALD_SET_GECOS,
      user->hometype(),
      user->home(),
      user->homeserver(),
      user->shell(),
      gecos.toString().toStdString(),
      user->desc(),
      gecos.fullName().toStdString(),
      user->policy(),
      std::string(),
      user->gid(),
      user->uid(),
      false
    );
    return true;

  } catch (const ALD::EALDError& exp) {
    setLastError(
      QObject::tr("%1 установить GECOS, исключение: %2")
        .arg(username)
        .arg(exp.what())
    );

  } catch (...) {
    setLastError(
      QObject::tr("%1 установить GECOS, необрабатываемое исключение")
        .arg(username)
    );
  }

  return false;
}

}

}
