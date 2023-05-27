#include "auth.h"
#include "pam.h"
#include "cache.h"

#ifdef T_OS_ASTRA_LINUX
#include "ald.h"
#else
#include "ald_stub.h"
#endif


namespace meteo {

namespace auth {

static const QStringList adminGroupList({ "astra-admin", "vgmdaemon", "root" });

Auth::Auth()
{
  QString path = QString("%1/os-auth.conf").arg(MnCommon::etcPath());
  if (!TProtoText::fromFile(path, &conf_)) {
    conf_.set_auth_provider(auth::kAuthPAM);
    error_log << logID() << QObject::tr("Ошибка разбора конфигурации: %1").arg(path);
  }
}

Auth::~Auth()
{
  delete impl_;
  impl_ = nullptr;
}

QString Auth::logID() const
{
  return QString("[%1]").arg(impl_ ? impl_->className() : "auth");
}

bool Auth::init(WorkType wtype)
{
  QMutexLocker locker(&mutex_);

  switch (wtype) {
    case Auth::kDirect : {
      switch (conf_.auth_provider()) {
        case auth::kAuthALD :
#ifdef T_OS_ASTRA_LINUX
          impl_ = new AldEngine(conf_);
#else
          impl_ = new AldStubEngine(conf_);
#endif
          break;

        default :
          impl_ = new PamEngine();
      }
      break;
    }

    case Auth::kCachedDirect : {
      Engine* provider;
      switch (conf_.auth_provider()) {
        case auth::kAuthALD :
#ifdef T_OS_ASTRA_LINUX
          provider = new AldEngine(conf_);
#else
          provider = new AldStubEngine(conf_);
#endif
          break;

        default :
          provider = new PamEngine();
      }

      impl_ = new CacheEngine(provider);
      break;
    }

    case Auth::kSubprocess :
    case Auth::kCashedSubprocess :
      break;
  };

  if (impl_ == nullptr)
    return false;

  info_log << logID() << QObject::tr("текущий обработчик");
  return impl_->init();
}

bool Auth::isAdmin(const QString& username)
{
  bool isOk = false;
  for (const QString& group : groupList(username)) {
    if (adminGroupList.contains(group))
      isOk = true;
  }

  debug_log << logID() << QObject::tr("%1, права администратора: %2")
                            .arg(username)
                            .arg(isOk ? "true" : "false");

  if (impl_->hasError())
    error_log << logID() << impl_->lastError();

  return isOk;
}

QString Auth::homeDir(const QString& username)
{
  QMutexLocker locker(&mutex_);
  QString res = impl_->homeDir(username);
  debug_log << logID() << QObject::tr("%1, домашний каталог: %2")
                            .arg(username)
                            .arg(res);

  if (impl_->hasError())
    error_log << logID() << impl_->lastError();

  return res;
}

QStringList Auth::userList()
{
  QMutexLocker locker(&mutex_);
  QStringList res = impl_->userList();
  debug_log << logID() << QObject::tr("список пользователей: %1").arg(res.join(','));

  if (impl_->hasError())
    error_log << logID() << impl_->lastError();

  return res;
}

QStringList Auth::groupList(const QString& username)
{
  QMutexLocker locker(&mutex_);
  QStringList res = impl_->groupList(username);
  debug_log << logID() << QObject::tr("%1, входит в группы: %2")
                           .arg(username)
                           .arg(res.join(','));

  if (impl_->hasError())
    error_log << logID() << impl_->lastError();

  return res;
}

bool Auth::tryAuth(const QString& username, const QString& password)
{
  QMutexLocker locker(&mutex_);
  bool isOk = impl_->tryAuth(username, password);
  debug_log << logID() << QObject::tr("%1, аутентификация: %2")
                            .arg(username)
                            .arg(isOk ? "true" : "false");

  if (impl_->hasError())
    error_log << logID() << impl_->lastError();

  return isOk;
}

bool Auth::registry(const QString& username, const QString& password, QString* msg)
{
  QMutexLocker locker(&mutex_);
  bool isOk = impl_->registry(username, password);
  if (!isOk && (msg != nullptr)) {
    *msg = impl_->lastError();
    error_log << logID() << *msg;
  }

  debug_log<< logID() << QObject::tr("%1, регистрация: %2")
                           .arg(username)
                           .arg(isOk ? "true" : "false");

  return isOk;
}

bool Auth::getGecos(const QString& username, Gecos& gecos)
{
  QMutexLocker locker(&mutex_);
  bool isOk = impl_->getGecos(username, gecos);
  // debug_log << logID() << QObject::tr("%1, текущий GECOS: %2")
  //                           .arg(username)
  //                           .arg(gecos.toString());

  if (impl_->hasError())
    error_log << logID() << impl_->lastError();

  return isOk;
}

bool Auth::setGecos(const QString& username, const Gecos& gecos)
{
  QMutexLocker locker(&mutex_);
  bool isOk = impl_->setGecos(username, gecos);
  // debug_log << logID() << QObject::tr("%1, установить GECOS: %2 %3")
  //                            .arg(username)
  //                            .arg(gecos.toString())
  //                            .arg(isOk ? "true" : "false");

  if (impl_->hasError())
    error_log << logID() << impl_->lastError();

  return isOk;
}

}

}
