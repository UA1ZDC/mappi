#include "daemoncontrol.h"

#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <new>

#include <qdir.h>
#include <qstring.h>
#include <qlocalsocket.h>
#include <qlocalserver.h>
#include <qfileinfo.h>
#include <qlist.h>
#include <qobject.h>

#include <cross-commons/funcs/mn_errdefs.h>
#include <cross-commons/funcs/mn_funcs.h>
#include <cross-commons/debug/tlog.h>

#include "checkps.h"

using std::nothrow;

void logmessage( int res, const QString& optparam )
{
  switch ( res ) {
    case -1:
      std::cout << QObject::tr("Неизвестная ошибка при попытке запустить процесс в фоновом режиме").toLocal8Bit().constData()<< std::endl;;
      //error_log << "Unknown error when trying to daemonize process\n\t*" << optparam;
      break;
    case -2:
      std::cout << QObject::tr("Процесс ").toLocal8Bit().constData()
                << optparam.toLocal8Bit().constData()
                << QObject::tr(" уже запущен ").toLocal8Bit().constData() <<std::endl;;
   //   info_log << "Process" << optparam << "already running";
      break;
    case -3:
      std::cout << QObject::tr("Вам не разрешено запускать процесс в фоновом режиме").toLocal8Bit().constData()
                <<std::endl;;
      //error_log << "Error. You are not allowed to run daemon" << optparam;
      break;
    default:
      break;
  }
}

//! закрыть все дескрипторы процесса, перенаправить стдин, стдаут, стдерр в /dev/null
void setupStdFds()
{
  int fd = 0;

//  struct rlimit flim;
//  getrlimit(RLIMIT_NOFILE,&flim);
//  for( fd = 0; fd < (int)( flim.rlim_max ); ++fd ) {
//    close(fd);
//  }
  ::close(STDIN_FILENO);
  ::close(STDOUT_FILENO);
  ::close(STDERR_FILENO);

  fd = open("/dev/null", O_RDONLY);
  if (fd != -1) {
    dup2(fd, STDIN_FILENO);
    if (fd != STDIN_FILENO)
    close(fd);
  }

  fd = open("/dev/null", O_WRONLY);
  if (fd != -1) {
    dup2(fd, STDOUT_FILENO);
    dup2(fd, STDERR_FILENO);
    if (fd != STDOUT_FILENO && fd != STDERR_FILENO)
      close(fd);
  }
}

//! установить обработчики сигналов
void setupSignals()
{
  //TODO либо убрать установку игнорирования обращений к консоли,
  //либо убрать перенаправление стдин, стдаут, стдерр
  signal(SIGTTOU,SIG_IGN);
  signal(SIGTTIN,SIG_IGN);
  signal(SIGTSTP,SIG_IGN);
  signal(SIGTERM,SIG_IGN);
  signal(SIGHUP,SIG_IGN);
}

//! демонизировать процесс с использованием QLocalServer
int ProcControl::daemonizeSock( const QString& sn, bool daemon )
{
  if ( sn.isEmpty() ) {
    std::cout << QObject::tr("Сокет не указан. Процесс запущен не в фоновом режиме").toLocal8Bit().constData()<<std::endl;;
    //info_log << "Socket name is not specified. Process run in normal state i.e. not daemon.";
    return -1;
  }

  QString sockname = MnCommon::sockPath(sn);
//  QString sockname = QString(QDir::homePath()) + "/" + sn + ".sock";

  int res = checkSockInUse(sockname);
  logmessage(res, sockname );
  if ( ERR_NOERR != res ) {
    return res;
  }

  if ( true == daemon ) {
    pid_t pid = fork();
    switch(pid) {
      case 0:
        pid = setsid();
        break;
      case -1:
        return -1;
      default:
        exit(0);
    }

    //закрыть все дескрипторы процесса, перенаправить stdin, stdout, stderr в /dev/null
    setupStdFds();
    //поставить обработчики сигналов
    setupSignals();
    if ( 0 != chdir("/") ) {
      return -1;
    }
  }

  res = singleInstanceSock(sn);
  if ( ERR_NOERR != res ) {
    return res;
  }

  return ERR_NOERR;
}

int ProcControl::singleInstanceSock( const QString& sn )
{
  QString sockname = MnCommon::sockPath(sn);
  int res = checkSockInUse(sockname);
  logmessage(res, sockname );
  if ( ERR_NOERR != res ) {
    return res;
  }
  QLocalServer* localsrv_ = new(nothrow) QLocalServer;
  localsrv_->setMaxPendingConnections(1000000);
  if ( false == localsrv_->listen(sockname) ) {
    std::cout << QObject::tr("Неизвестная ошибка при попытке запустить процесс в фоновом режиме").toLocal8Bit().constData()<< std::endl;;
    //error_log << "Unknown error when trying to daemonize process\n\t*" << sockname;
    return -1;
  }
  return ERR_NOERR;
}

int ProcControl::checkSockInUse( const QString& sockname )
{
  if ( true == QFile::exists(sockname) ) { //check already running
    QLocalSocket sock;
    sock.connectToServer( sockname, QIODevice::WriteOnly );
    if ( false == sock.waitForConnected(100) ) {
      switch ( sock.error() ) {
        case QLocalSocket::ConnectionRefusedError:
          break;
        default:
          error_log << QObject::tr("Last error code = %1. Description = %2")
            .arg( sock.error() )
            .arg( sock.errorString() );
          return -1;
      }
      //try to remove socket
      QFile::remove(sockname);
    }
    else { //socket is busy
      sock.abort();
      return -2;
    }
  }
  else { //check dir path
    QDir dir = QFileInfo(sockname).absoluteDir();
    if ( !dir.exists() ) {
      bool res = dir.mkpath( dir.absolutePath() );
      if ( false == res ) {
        return -3;
      }
    }
  }
  return ERR_NOERR;
}
