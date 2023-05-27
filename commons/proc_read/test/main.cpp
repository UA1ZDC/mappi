#include <qcoreapplication.h>
#include <qtextcodec.h>
#include <qprocess.h>
#include <qmutex.h>

#include <cross-commons/app/paths.h>
#include <cross-commons/app/targ.h>
#include <cross-commons/debug/tlog.h>

#include <commons/proc_read/daemoncontrol.h>
#include <commons/proc_read/checkps.h>


const QStringList kOptT1  = QStringList() << "1" << "test-1";
const QStringList kOptT2  = QStringList() << "2" << "test-2";
const QStringList kOptT3  = QStringList() << "3" << "test-3";

const QStringList kDaemonOpt  = QStringList() << "d" << "daemonize";

bool test_AlreadyRun(QCoreApplication* app)
{
  QString appName = app->applicationName();
  if ( ProcControl::isAlreadyRun(appName) ) {
    error_log << QObject::tr("PID: %1 -- Другой экземпляр %2 уже запущен.").arg(app->applicationPid()).arg(appName);
    return false;
  }

  info_log << QObject::tr("Запущен %2 -- PID: %1.").arg(app->applicationPid()).arg(appName);

  for ( int i=0; i<2; ++i ) {
    QMutex m;
    m.lock();
    m.tryLock(1000);
    QProcess::startDetached("./test");
  }

  app->exec();

  return true;
}

void test_ProcList()
{
  QList<ProcRead::ProcInfo> list = ProcRead::procList(ProcRead::kPID | ProcRead::kPPID);
  info_log << QString("PID").rightJustified(8,' ')
            << QString("PPID").rightJustified(8,' ');
  for ( int i=0,isz=list.size(); i<isz; ++i ) {
    info_log << QString("%1").arg(list[i].pid, 8, 10, QChar(' '))
              << QString("%1").arg(list[i].ppid, 8, 10, QChar(' '));
  }
}

void test_ChildList(unsigned pid)
{
  QList<ProcRead::ProcInfo> list = ProcRead::procChildList(pid, true);
  info_log << QString("PID").rightJustified(8,' ')
            << QString("PPID").rightJustified(8,' ');
  for ( int i=0,isz=list.size(); i<isz; ++i ) {
    info_log << QString("%1").arg(list[i].pid, 8, 10, QChar(' '))
              << QString("%1").arg(list[i].ppid, 8, 10, QChar(' '));
  }
}

int main( int argc, char* argv[] )
{
  TAPPLICATION_NAME("test");

  QCoreApplication app( argc, argv );

  TLog::setLogFileName(app.applicationDirPath() + "/test.log");
  TLog::setMsgHandler(TLog::fileOut);

  TArg args(argc,argv);

  if ( args.contains(kOptT1) ) {
    test_AlreadyRun(&app);
  }
  if ( args.contains(kOptT2) ) {
    test_ProcList();
  }
  if ( args.contains(kOptT3) ) {
    test_ChildList(args.value(kOptT3).toUInt());
  }
  if ( args.contains(kDaemonOpt) ) {
    if ( 0 != ProcControl::daemonizeSock("proc_read_test") ) {
      error_log << QObject::tr("Такой процесс уже запущен");
      return -1;
    }
    info_log << QObject::tr("Демон %1 запущен").arg(argv[0]);

    app.exec();
  }

  return 0;
}
