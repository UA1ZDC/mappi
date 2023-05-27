#ifndef TAPPCONTROL_H
#define TAPPCONTROL_H

#include <google/protobuf/stubs/common.h>

#include <qobject.h>
#include <qmap.h>
#include <qpair.h>
#include <qprocess.h>
#include <qmutex.h>

#include <cross-commons/app/tsigslot.h>
#include <cross-commons/singleton/tsingleton.h>
#include <cross-commons/debug/tlog.h>
#include <meteo/commons/proto/appconf.pb.h>

class QDir;

namespace google
{
namespace protobuf
{
class Closure;
}
}

namespace meteo {

namespace rpc
{
class Channel;
class Controller;
}
namespace app {

typedef QPair<AppState::Proc*, google::protobuf::Closure*> ProcSubscribe;
typedef QPair<AppOutReply*, google::protobuf::Closure*> OutSubscribe;

class Service;
class AppLogWatcher;

static const QString kManagerLogFile = "app.manager.log";
static const QString kManagerId = "app.manager";

namespace internal {

class Control : public QObject
{
  Q_OBJECT
  public:    
    ~Control();

    bool loadConf();
    bool startProcs();
    void startProc( QProcess* proc );
    void stopProc( QProcess* proc );
    void startProc( int64_t proc );
    void stopProc( int64_t proc );
    void changeProc( const OperationStatus* status );
    void getStatus( int64_t proc, OperationStatus* status);
    void getStatus( QProcess* proc, OperationStatus* status);

    const QMap< QProcess*, Application >& applications()
    { return applications_; }

    static QString logFileName( const QString& appid, int64_t procid );
    static QString outputToFile();
    static void restoreOutput();

    void changesSubscribe( AppState::Proc* proc, google::protobuf::Closure* done );
    void appOutSubscribe( AppOutReply* out, google::protobuf::Closure* done );

    const QMap< QProcess*, QDateTime >& startdates() const { return startdates_; }
    const QMap< QProcess*, QDateTime >& stopdates() const { return stopdates_; }

  public slots:
    void slotClientSubscribed( meteo::rpc::Controller* );
    void slotClientUnsubscribed( meteo::rpc::Controller* );

  private slots:
    void slotProcError( QProcess::ProcessError err );
    void slotProcFinished( int exitCode, QProcess::ExitStatus exitStatus );
    void slotRestartProcess(int exitCode, QProcess::ExitStatus exitStatus);
    void slotProcStarted();
    void slotProcStateChanged( QProcess::ProcessState newState );

  private:
    Control();
    void setProcEnv(Application* app, QProcess* proc );
    void setOutFile( const QDir& path, Application* app, QProcess* proc );
    void reloadConfig(QProcess* proc);
    void clearOldLogs( const QDir& path );
    void selfLog();

  private:
    QMutex mutex_;
    const int32_t OUT_MAX_LINE_SIZE;
    QMap< QProcess*, Application > applications_;
    QMap< QProcess*, QDateTime > startdates_;
    QMap< QProcess*, QDateTime > stopdates_;
    AppLogWatcher* logwatcher_;
    QMap< int64_t, Application > appmap_;
    QMap< int64_t, QProcess* > procmap_;
    QMap< int64_t, OperationStatus> statusmap_;

    QMap< google::protobuf::Closure*, rpc::Channel* > unksubscribes_;
    QMap< rpc::Channel*, ProcSubscribe > changesubscribes_;
    QMap< rpc::Channel*, OutSubscribe > outsubscribes_;

    QStringList outfilelist_;
    bool wasdisconnected_;

    friend class TSingleton<Control>;
};

}

typedef  TSingleton<internal::Control> Control;

}
}

#endif
