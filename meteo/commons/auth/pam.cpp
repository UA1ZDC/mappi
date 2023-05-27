#include "pam.h"
#include "pam_context.h"
#include <qprocess.h>
#include <sys/types.h>
#include <grp.h>
#include <unistd.h>


namespace meteo {

namespace auth {

static const QStringList adminGroupList({ "astra-admin", "vgmdaemon", "meteo", "root" });
static const QString sudo = "sudo";
static const QString wwwdata = "www-data";
static const QString meteogroup = "meteouser";

static const uid_t minUid = 500;
static const uid_t maxUid = 9999;

PamEngine::PamEngine() :
  bufferSize_(-1)
{
}

PamEngine::~PamEngine()
{
}

bool PamEngine::init()
{
  bufferSize_ = sysconf(_SC_GETPW_R_SIZE_MAX);
  if (bufferSize_ == -1) {
    error_log << QObject::tr("[auth:pam] _SC_GETPW_R_SIZE_MAX: -1");
    return false;
  }

  buffer_.resize(bufferSize_);
  return true;
}

bool PamEngine::initPasswd(const QString& username, struct passwd* pwd)
{
  struct passwd* result = nullptr;
  // error_log << QObject::tr("[auth:pam] pwd %1 , %2").arg(pwd->pw_name).arg(pwd->pw_gid);
  ::getpwnam_r(username.toStdString().c_str(), pwd, buffer_.data(), buffer_.size(), &result);
  if (result == nullptr) {
    error_log << QObject::tr("[auth:pam] %1 не найден").arg(username);
    return false;
  }

  return true;
}

bool PamEngine::setPassword(const QString& username, const QString& password)
{
  QProcess proc;
  proc.start(sudo, { "chpasswd" });
  proc.waitForStarted(3000);
  if (proc.state() != QProcess::Running) {
    setLastError(QObject::tr("chpasswd, не запустился по таймеру"));
    return false;
  }

  QString updateStr = QObject::tr("%1:%2")
    .arg(username)
    .arg(password);

  proc.write(updateStr.toStdString().c_str());
  proc.closeWriteChannel();

  proc.waitForFinished(3000);
  if (proc.state() == QProcess::Running || proc.exitStatus() != 0) {
    setLastError(proc.readAllStandardError());
    return false;
  }

  return true;
}

bool PamEngine::isAdmin(const QString& username)
{
  for (const QString& group : groupList(username)) {
    if (adminGroupList.contains(group))
      return true;
  }

  return false;
}

QString PamEngine::homeDir(const QString& username)
{
  struct passwd pwd;
  if (initPasswd(username, &pwd))
    return pwd.pw_dir;

  return QString();
}

QStringList PamEngine::userList()
{
  QStringList res;

  struct passwd pwbuf;
  struct passwd* pwbufp = nullptr;
  static const long buflen = 4096;      // Значение взято из man 3 getpwent_r
  char buf[buflen];

  FILE* passwd = ::fopen("/etc/passwd", "r");
  while (true) {
    int code = ::fgetpwent_r(passwd, &pwbuf, buf, buflen, &pwbufp);
    if (code == ENOENT)
      break;

    if (code == ERANGE) {
      error_log << QObject::tr("[auth:pam] недостаточный размер буфера. Обратитесь к разработчику");
      break;
    }

    if (pwbuf.pw_uid < minUid || maxUid < pwbuf.pw_uid)
      continue;

    res << pwbufp->pw_name;
  }
  ::fclose(passwd);

  return res;
}

QStringList PamEngine::groupList(const QString& username)
{
  QStringList res;
  struct passwd pwd;
  // error_log << QObject::tr("[auth:pam] initPasswd %1 , ").arg(username);
  if (!initPasswd(username, &pwd))
    return res;

  int ngroups = 0;
  ::getgrouplist(pwd.pw_name, pwd.pw_gid, nullptr, &ngroups);
  if (ngroups <= 0) {
    setLastError(
      QObject::tr("%1 должен входить в состав не менее одной группы")
        .arg(username)
    );
    return res;
  }

  gid_t* groups = new gid_t[ngroups];
  // here we actually get the groups
  ::getgrouplist(pwd.pw_name, pwd.pw_gid, groups, &ngroups);

  const long grpbuflen = ::sysconf(_SC_GETGR_R_SIZE_MAX);
  if (grpbuflen == -1) {
    error_log << QObject::tr("[auth:pam] _SC_GETGR_R_SIZE_MAX: -1");
    delete[] groups;
    return res;
  }

  char* grpbuf = new char[grpbuflen];
  for (int i = 0; i < ngroups; ++i) {
    gid_t gid = groups[i];
    struct group groupbuf;
    struct group *group = nullptr;
    ::getgrgid_r(gid, &groupbuf, grpbuf, grpbuflen, &group);
    if (group == nullptr) {
      setLastError(QObject::tr("группа не найдена: %1").arg(gid));
      continue;
    }

    res << group->gr_name;
  }

  delete[] groups;
  delete[] grpbuf;

  return res;
}

bool PamEngine::tryAuth(const QString& username, const QString& password)
{
  PamContext pam(username, password);
  if (!pam.create() || !pam.authenticate() || !pam.terminate()) {
    setLastError(QObject::tr("аутентификации завершилась с ошибкой"));
    return false;
  }

  return true;
}

bool PamEngine::registry(const QString& username, const QString& password)
{
  Q_UNUSED(password)
  if (userList().contains(username))
    return true;

  {
    QProcess proc;
    proc.start(sudo, { "useradd", username });
    proc.waitForFinished(3000);
    if (proc.state() == QProcess::Running) {
      setLastError(QObject::tr("useradd, завершил работу по таймеру"));
      return false;
    }

    if (proc.exitCode() != 0) {
      if (!userList().contains(username)) {
        setLastError(proc.readAllStandardError());
        return false;
      } else
        warning_log << proc.readAllStandardError() << proc.readAllStandardOutput();
    }
  }

  // if (!setPassword(username, password))
  //   return false;

  struct group groupbuf;
  struct group *group = nullptr;
  ::getgrnam_r(meteogroup.toStdString().c_str(), &groupbuf, buffer_.data(), buffer_.size(), &group);
  if (group == nullptr) {
   QProcess proc;
    proc.start(sudo, { "groupadd", meteogroup });
    proc.waitForFinished(3000);
    if (proc.state() == QProcess::Running || proc.exitCode() != 0) {
      setLastError(proc.readAllStandardError());
      return false;
    }
  }

  for (const QString& usergroup : { meteogroup, wwwdata }) {
    QProcess proc;
    proc.start(sudo, { "usermod", "-a", "-G", usergroup, username});
    proc.waitForFinished(3000);
    if (proc.state() == QProcess::Running || proc.exitCode() != 0) {
      setLastError(proc.readAllStandardError());
      return false;
    }
  }

#ifdef T_OS_ASTRA_LINUX
  {
    QProcess proc;
    proc.start(sudo, { "pdpl-user", "-i", "0:0", "-c", "0:0", "-i", "0", username });
    proc.waitForFinished();
    if (proc.state() == QProcess::Running || proc.exitCode() != 0) {
      setLastError(proc.readAllStandardError());
      return false;
    }
  }
#endif

  return true;
}

bool PamEngine::getGecos(const QString& username, Gecos& gecos)
{
  struct passwd pwd;
  if (!initPasswd(username, &pwd))
    return false;

  gecos.fromString(pwd.pw_gecos);
  return true;
}

bool PamEngine::setGecos(const QString& username, const Gecos& gecos)
{
  QProcess proc;
  proc.start(sudo, { "chfn",
    "--full-name", gecos.fullName(),
    "--other", gecos.other(),
    "--room", gecos.roomNumber(),
    "--work-phone", gecos.workNumber(),
    username
  });

  proc.waitForFinished(3000);
  if (proc.state() == QProcess::Running || proc.exitStatus() != 0) {
    setLastError(proc.readAllStandardError());
    return false;
  }

  return true;
}

}

}
