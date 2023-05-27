#include "ald_stub.h"


namespace meteo {

namespace auth {

AldStubEngine::AldStubEngine(const auth::AuthConf& conf) :
  aldConf_(nullptr),
  pam_(new PamEngine())
{
  Q_UNUSED(conf);
}

AldStubEngine::~AldStubEngine()
{
  delete aldConf_;
  aldConf_ = nullptr;

  delete pam_;
  pam_ = nullptr;
}

bool AldStubEngine::isSystemUser(const QString& username)
{
  for (auto item : aldConf_->system_user()) {
    if (username == item.c_str())
      return true;
  }

  return false;
}

bool AldStubEngine::init()
{
  aldConf_ = new auth::AldUsersConfig();
  QString path = QObject::tr("%1/ald-users.conf").arg(MnCommon::etcPath());
  if (!TProtoText::fromFile(path, aldConf_)) {
    aldConf_->Clear();
    return false;
  }

  return pam_->init();
}

bool AldStubEngine::isAdmin(const QString& username)
{
  if (isSystemUser(username))
    return pam_->isAdmin(username);

  return true;
}

QString AldStubEngine::homeDir(const QString& username)
{
  if (isSystemUser(username))
    return pam_->homeDir(username);

  return QString();
}

QStringList AldStubEngine::userList()
{
  QStringList res;
  for (auto item : aldConf_->system_user())
    res << item.c_str();

  res << pam_->userList();
  return res;
}

QStringList AldStubEngine::groupList(const QString& username)
{
  if (isSystemUser(username))
    return pam_->groupList(username);

  return QStringList();
}

bool AldStubEngine::tryAuth(const QString& username, const QString& password)
{
  if (isSystemUser(username))
    return pam_->tryAuth(username, password);

  return true;
}

bool AldStubEngine::registry(const QString& username, const QString& password)
{
  Q_UNUSED(username);
  Q_UNUSED(password);

  return true;
}

bool AldStubEngine::getGecos(const QString& username, Gecos& gecos)
{
  if (isSystemUser(username))
    return pam_->getGecos(username, gecos);

  return true;
}

bool AldStubEngine::setGecos(const QString& username, const Gecos& gecos)
{
  if (isSystemUser(username))
    return pam_->setGecos(username, gecos);

  return true;
}

}

}
