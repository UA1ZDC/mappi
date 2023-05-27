#include "tappcontrol.h"

#include <qdir.h>
#include <qfileinfo.h>
#include <qprocess.h>
#include <qstringlist.h>

#include <cross-commons/debug/tlog.h>

#include <commons/proc_read/checkps.h>
#include <commons/textproto/pbtools.h>

#include <meteo/commons/global/dateformat.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/proto/appconf.pb.h>
#include <meteo/commons/rpc/rpc.h>

#include "tappconf.h"
#include "applogwatcher.h"

template <>
meteo::app::internal::Control* meteo::app::Control::_instance = nullptr;

namespace meteo
{
namespace app
{

namespace internal {
static const char* kPropLogFile = "p_log_file";

Control::Control()
  : mutex_( QMutex::Recursive ),
  OUT_MAX_LINE_SIZE(1024 * 16),
  logwatcher_(nullptr),
  wasdisconnected_(false)
{
}

Control::~Control()
{
  QMutexLocker lock(&mutex_);
  foreach ( QProcess* p, applications_.keys() ) {
    p->disconnect(this);
    p->terminate();
    p->waitForFinished();
    p->kill();
    p->waitForFinished();
    p->deleteLater();
  }
  applications_.clear();
}

bool Control::loadConf()
{
  QMutexLocker lock(&mutex_);
  AppConfig config;
  if ( false == Conf::load(&config) ) {
    info_log << QObject::tr("Нет приложений для запуска");
    info_log << QObject::tr("Путь конфигурационных файлов:") << MnCommon::etcPath() + "/app.conf.d";
    return false;
  }
  logwatcher_ = new AppLogWatcher(this, config.sendsecs() );

  QDir dir(QDir::homePath() + "/.meteo/logs/");
  if ( false == dir.exists(dir.absolutePath()) ) {
    if ( false == dir.mkpath(dir.absolutePath()) ) {
      error_log << QObject::tr("Не удалось создать директорию %1").arg( dir.absolutePath() );
      dir.setPath("./");
    }
  }
  clearOldLogs(dir);
  selfLog();
  for ( int i = 0, sz = config.app_size(); i < sz; ++i ) {
    Application* app = config.mutable_app(i);
    QFileInfo fi(pbtools::toQString(app->path()) );
    if ( false == fi.exists() ) {
      error_log << QObject::tr("Приложение '%1' не найдено и не будет запущено. \n\t--Путь - %2")
        .arg(pbtools::toQString(app->title()))
        .arg(pbtools::toQString(app->path()));
      continue;
    }
    QProcess* proc = new QProcess(this);
    connect( proc, SIGNAL(error(QProcess::ProcessError)), SLOT( slotProcError(QProcess::ProcessError)) );
    connect( proc, SIGNAL(finished(int, QProcess::ExitStatus)), SLOT( slotProcFinished(int,QProcess::ExitStatus )) );
    connect( proc, SIGNAL(started()), this, SLOT(slotProcStarted()) );
    connect( proc, SIGNAL(stateChanged(QProcess::ProcessState)), SLOT( slotProcStateChanged(QProcess::ProcessState)) );
    applications_.insert( proc, *app );
    appmap_.insert( reinterpret_cast<int64_t>(proc), *app );
    procmap_.insert( reinterpret_cast<int64_t>(proc), proc);
    if ( 0 != app->env_size() ) {
      setProcEnv( app, proc );
    }
    if ( true == app->has_workdir() ) {
      proc->setWorkingDirectory( pbtools::toQString( app->workdir() ) );
    }
    setOutFile( dir, app, proc );
  }
  logwatcher_->setOutFiles(outfilelist_);
  return true;
}

bool Control::startProcs()
{
  QMutexLocker lock(&mutex_);
  QMap< QProcess*, Application >::iterator it = applications_.begin();
  QMap< QProcess*, Application >::iterator end = applications_.end();
  for ( ; it != end; ++it ) {
    const Application& app = it.value();
    if ( true == app.has_autostart() && true == app.autostart() ) {
      startProc( it.key() );
    }
  }
  return true;
}
//
//void Control::slotClientDisconnected( rpc::Channel* channel )
//{
//  wasdisconnected_ = true;
//
//  QList<google::protobuf::Closure*> unkdones_ = unksubscribes_.keys(channel);
//  for ( int i = 0, sz = unkdones_.size(); i < sz; ++i ) {
//    unksubscribes_.remove(unkdones_[i]);
//  }
//  changesubscribes_.remove(channel);
//  QList<OutSubscribe> subscrlist = outsubscribes_.values(channel);
//  for ( int i = 0, sz = subscrlist.size(); i < sz; ++i ) {
//    logwatcher_->rmSubscribe( subscrlist[i] );
//  }
//  outsubscribes_.remove(channel);
//}
//
//void Control::slotClientSubscribed( rpc::Channel* ch, google::protobuf::Closure* done )
//{
//  unksubscribes_.insert( done, ch );
//}

void Control::slotClientSubscribed( meteo::rpc::Controller* call )
{
  QMutexLocker lock(&mutex_);
  unksubscribes_.insert( call->closure(), call->channel() );
}

void Control::slotClientUnsubscribed( meteo::rpc::Controller* call )
{
  QMutexLocker lock(&mutex_);
  rpc::Channel* ch = call->channel();
  wasdisconnected_ = true;

  QList<google::protobuf::Closure*> unkdones_ = unksubscribes_.keys(ch);
  for ( int i = 0, sz = unkdones_.size(); i < sz; ++i ) {
    unksubscribes_.remove(unkdones_[i]);
  }
  changesubscribes_.remove(ch);
  QList<OutSubscribe> subscrlist = outsubscribes_.values(ch);
  for ( int i = 0, sz = subscrlist.size(); i < sz; ++i ) {
    logwatcher_->rmSubscribe( subscrlist[i] );
  }
  outsubscribes_.remove(ch);
}

void Control::changesSubscribe( AppState::Proc* res, google::protobuf::Closure* done )
{
  QMutexLocker lock(&mutex_);
  while ( true == unksubscribes_.contains(done) ) {
    rpc::Channel* ch = unksubscribes_.take( done );
    changesubscribes_.insertMulti( ch, ProcSubscribe(res, done) );
  }

  OperationStatus status;
  foreach( status, statusmap_ ) {
    changeProc( &status );
  }
}

void Control::appOutSubscribe( AppOutReply* res, google::protobuf::Closure* done )
{
  QMutexLocker lock(&mutex_);
  while ( true == unksubscribes_.contains(done) ) {
    rpc::Channel* ch = unksubscribes_.take( done );
    outsubscribes_.insertMulti( ch, OutSubscribe(res, done) );
  }
  QProcess* p = reinterpret_cast<QProcess*>( res->id() );
  if ( false == applications_.contains(p) ) {
    error_log << QObject::tr("Неизвестная ошибка");
    return;
  }
  QString filename = logFileName( pbtools::toQString( applications_[p].id() ), res->id() );
  logwatcher_->addSubscribe( filename, OutSubscribe( res, done ) );
}

void Control::setProcEnv( Application* app, QProcess* proc )
{
  QMutexLocker lock(&mutex_);
  QProcessEnvironment procenv = QProcessEnvironment::systemEnvironment();
  for ( int i = 0, sz = app->env_size(); i < sz; ++i ) {
    QStringList list = pbtools::toQString( app->env(i) ).split("=");
    QString arg = list[0].simplified();
    QString val;
    if ( 1 < list.size() ) {
      val = list[1].simplified();
    }
    if ( true == procenv.contains( arg ) ) {
      val = procenv.value(arg) + ";" + val;
    }
    procenv.insert( arg, val );
  }
  proc->setProcessEnvironment(procenv);
}

void Control::setOutFile( const QDir& dir, Application* app, QProcess* proc )
{
  QMutexLocker lock(&mutex_);
  QString file = logFileName(pbtools::toQString(app->id()), reinterpret_cast<int64_t>(proc));
  file = dir.absoluteFilePath(file);
  if ( false == dir.exists() ) {
    if ( false == dir.mkpath( dir.absolutePath() ) ) {
      file = QDir::home().absoluteFilePath(file);
    }
  }
  proc->setProcessChannelMode( QProcess::MergedChannels );
  proc->setStandardOutputFile( file, QIODevice::Append );
  proc->setProperty(kPropLogFile, file);
  if ( false == outfilelist_.contains(file) ) {
    outfilelist_.append(file);
  }
}

void Control::reloadConfig(QProcess* proc)
{
  QMutexLocker lock(&mutex_);
  if( true == applications_.contains(proc) ){
    meteo::app::Application app = applications_[proc];
    meteo::app::AppConfig config;
    meteo::app::Conf::load(&config);
    for ( int i = 0, sz = config.app_size(); i < sz; ++i ) {
      meteo::app::Application reload_app = config.app(i);
      if( app.id() == reload_app.id() ){
        applications_[proc] = reload_app;
        int64_t procid = procmap_.key(proc);
        if( true == appmap_.contains(procid) ){
          appmap_[procid] = reload_app;
        }
        break;
      }
    }
  }else{
    error_log << QObject::tr("Неизвестный процесс");
    return;
  }

}

void Control::clearOldLogs( const QDir& path )
{
  QStringList files = path.entryList( QStringList("*.log"), QDir::Files | QDir::NoDotAndDotDot );
  for ( int i = 0, sz = files.size(); i < sz; ++i ) {
    QFile::remove( path.absoluteFilePath( files[i] ) );
  }
}

void Control::selfLog()
{
  QMutexLocker lock(&mutex_);
  auto fileNameOutput = outputToFile();
  QProcess* proc = new QProcess(this);
  Application* manager = new Application();
  manager->set_id(kManagerId.toStdString());
  manager->set_title("Менеджер приложений");
  manager->set_comment("Менеджер приложений");
  manager->set_path("empty");
  manager->set_sponame("Контроль и диагностика");
  applications_.insert( proc, *manager );
  appmap_.insert( reinterpret_cast<int64_t>(proc), *manager );
  procmap_.insert( reinterpret_cast<int64_t>(proc), proc);
  proc->setProperty(kPropLogFile, fileNameOutput);
  outfilelist_.append( fileNameOutput );
}

void Control::startProc( QProcess* proc )
{
  QMutexLocker lock(&mutex_);
  QStringList args;
  const Application& app = applications_[proc];
  for ( int i = 0, sz = app.arg_size(); i < sz; ++i ) {
    args.append( pbtools::toQString( app.arg(i) ) );
  }
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  env.insert( "procid", QString::number( reinterpret_cast<int64_t>(proc) ));
  proc->setProcessEnvironment( env );

  // устанавливаем обработчик перезапуска
  const RestartPolicy restartPolicy = app.restart();
  if(restartPolicy != RestartPolicy::NO_RESTART)
    connect(proc, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(slotRestartProcess(int, QProcess::ExitStatus)));

  proc->start( pbtools::toQString( app.path() ), args );
}

void Control::slotRestartProcess(int exitCode, QProcess::ExitStatus exitStatus){
  QProcess* proc = qobject_cast<QProcess*>(sender());
  if(!proc || !applications_.contains(proc)) return;
  const RestartPolicy restartPolicy = applications_[proc].restart();

  debug_log << tr("Процесс остановлен (policy: %1, code: %2) [%3] %4")
      .arg(pbtools::toQString(RestartPolicy_Name(restartPolicy)))
      .arg(exitCode)
      .arg(proc->pid())
      .arg(pbtools::toQString(applications_[proc].title()));

  bool needsRestart = false;
  switch(restartPolicy){
    case RestartPolicy::ALWAYS:
    case RestartPolicy::UNLESS_STOPPED: //если выключен через менеждер, сигнал не должен быть послан
      needsRestart = true;
      break;
    case RestartPolicy::ON_FAILURE: //если выключен через менеждер, сигнал не должен быть послан
      if(exitStatus == QProcess::ExitStatus::CrashExit) needsRestart = true;
      break;
    default:
      break;
  }
  if(needsRestart){
    info_log << tr("Перезапускаем процесс (policy: %1) [%2] %3")
        .arg(pbtools::toQString(RestartPolicy_Name(restartPolicy)))
        .arg(proc->pid())
        .arg(pbtools::toQString(applications_.value(proc, Application()).title()));
    return startProc(proc);
  }
}


void Control::stopProc( QProcess* proc )
{
  QMutexLocker lock(&mutex_);
  info_log << tr("Остановка процесса [%1] %2").arg(proc->pid())
                                                  .arg(pbtools::toQString(applications_.value(proc, Application()).path()));

  // проверяем условия перезапуска
  if(applications_.contains(proc)){
    const RestartPolicy restartPolicy = applications_[proc].restart();
    if(restartPolicy == RestartPolicy::UNLESS_STOPPED || restartPolicy == RestartPolicy::ON_FAILURE)
      proc->disconnect(SLOT(SLOT(slotRestartProcess(int, QProcess::ExitStatus))));
  }

  // завершаем все дочерние процессы
  QList<ProcRead::ProcInfo> list = ProcRead::procChildList(proc->pid(), true);
  for ( int i=0,isz=list.size(); i<isz; ++i ) {
    QProcess* killer = new QProcess(this);
    connect( killer, SIGNAL(finished(int)), killer, SLOT(deleteLater()) );
    killer->start("kill", QStringList() << "-9" << QString::number(list[i].pid));
  }
  proc->kill();
}

QString Control::logFileName( const QString& appid, int64_t procid )
{
  QDir dir(QDir::homePath() + "/.meteo/logs/");
  if ( false == dir.exists( dir.absolutePath() ) ) {
    dir.setPath("./");
  }
  if ( kManagerId == appid ) {
    return dir.absoluteFilePath( kManagerLogFile );
  }
  return dir.absoluteFilePath(appid) + "." + QString::number(procid) + ".log";
}

QString Control::outputToFile()
{
  QDir dir(QDir::homePath() + "/.meteo/logs/");
  if ( false == dir.exists(dir.absolutePath()) ) {
    if ( false == dir.mkpath(dir.absolutePath()) ) {
      error_log << QObject::tr("Не удалось создать директорию %1").arg( dir.absolutePath() );
      dir.setPath("./");
    }
  }
  QString fileNameOutput = dir.absoluteFilePath(kManagerLogFile);
  QFile::remove(fileNameOutput);
  QFile fileOut(fileNameOutput);
  fileOut.open(QFile::WriteOnly);
  int fileDescrOut = fileOut.handle();
  dup2(fileDescrOut,STDOUT_FILENO);
  dup2(fileDescrOut,STDERR_FILENO);
  fileOut.close();
  return fileNameOutput;
}

void Control::restoreOutput()
{
  dup2(1,STDOUT_FILENO);
  dup2(2,STDERR_FILENO);
}

void Control::startProc( int64_t id )
{
  QMutexLocker lock(&mutex_);
  QProcess* proc = reinterpret_cast<QProcess*>(id);
  if ( false == applications_.contains(proc) ) {
    error_log << QObject::tr("Попытка запустить процесс с неизвестным идентификатором");
    return;
  }
  if ( kManagerId.toStdString() == applications_[proc].id() ) {
    warning_log << QObject::tr("Управлять менеджером процессов из клиента запрещено");
    return;
  }
  if ( QProcess::Running == proc->state() ) {
    info_log << QObject::tr("Процесс уже запущен");
    return;
  }
  reloadConfig(proc);
  startProc(proc);
}

void Control::stopProc( int64_t id )
{
  QMutexLocker lock(&mutex_);
  QProcess* proc = reinterpret_cast<QProcess*>(id);
  if ( false == applications_.contains(proc) ) {
    error_log << QObject::tr("Попытка остановить процесс с неизвестным идентификатором");
    return;
  }
  if ( kManagerId.toStdString() == applications_[proc].id() ) {
    warning_log << QObject::tr("Управлять менеджером процессов из клиента запрещено");
    return;
  }
  if ( QProcess::Running != proc->state() ) {
    info_log << QObject::tr("Процесс не запущен");
    return;
  }
  stopProc(proc);
}

void Control::changeProc(const OperationStatus* status)
{
  QMutexLocker lock(&mutex_);
  if( nullptr == status ){
    error_log << QObject::tr("Ошибка. Нулевой указатель на статус");
    return;
  }
  if( false == appmap_.contains( status->id()) ){
    // error_log << QObject::tr("Ошибка. Приложение c идентификатором %1 не найдено").arg(status->id());
    return;
  }

  if( false == procmap_.contains( status->id()) ){
    error_log << meteo::msglog::kProcessNotFound.arg(status->id());
    return;
  }
  QProcess* p = procmap_[status->id()];
  if( nullptr == p ){
      error_log << QObject::tr("Ошибка. Нулевой указатель на процесс %1").arg(status->id());
    return;
  }
  AppState::ProcState st = ( QProcess::Running == p->state() ) ? AppState::PROC_RUNNING : AppState::PROC_STOPPED ;
  const Application& app = appmap_[status->id()];
  statusmap_[status->id()].CopyFrom(*status);

  wasdisconnected_ = false;
  for ( rpc::Channel* ch : changesubscribes_.keys() ) {
    if ( false == changesubscribes_.contains(ch) ) {
      continue;
    }

    meteo::app::AppState::Proc* proc = changesubscribes_[ch].first;
    if( nullptr == proc ){
      continue;
    }
    proc->mutable_app()->CopyFrom(app);
    if ( true == startdates_.contains(p) ) {
      proc->set_startdt( startdates_[p].toString( Qt::SystemLocaleShortDate ).toStdString() );
    }
    else {
      proc->clear_startdt();
    }
    if ( true == stopdates_.contains(p) ) {
      proc->set_stopdt( stopdates_[p].toString( Qt::SystemLocaleShortDate ).toStdString() );
    }
    else {
      proc->clear_stopdt();
    }
    proc->set_id( status->id() );
    proc->mutable_status()->CopyFrom(*status);
    proc->set_state(st);

    changesubscribes_[ch].second->Run();
  }

  for ( rpc::Channel* ch : changesubscribes_.keys() ) {
    if ( false == changesubscribes_.contains(ch) ) {
      continue;
    }

    meteo::app::AppState::Proc* proc = changesubscribes_[ch].first;
    if( 0 == proc ){
      continue;
    }

    proc->mutable_status()->Clear();
    proc->mutable_status()->set_id(proc->id());
  }
}

void Control::slotProcError( QProcess::ProcessError err )
{
  QMutexLocker lock(&mutex_);
  QProcess* proc = qobject_cast<QProcess*>( sender() );
  if ( nullptr == proc ) {
    error_log << QObject::tr("Неизвестная ошибка. Отправитель сигнала не процесс.");
    return;
  }
  const Application& app = applications_[proc];
  error_log << QObject::tr("Ошибка %1 в процессе %2.\n\tОписание ошибки: %3")
    .arg(err)
    .arg( pbtools::toQString( app.id() ) )
    .arg( proc->errorString() );
}

void Control::slotProcFinished( int exitCode, QProcess::ExitStatus exitStatus )
{
  QMutexLocker lock(&mutex_);
  QProcess* proc = qobject_cast<QProcess*>( sender() );
  if ( nullptr == proc ) {
    error_log << QObject::tr("Неизвестная ошибка. Отправитель сигнала не процесс.");
    return;
  }
  const Application& app = applications_[proc];
  info_log << QObject::tr("Процесс %1 завершился с кодом %2. Статус завершения %3")
    .arg( pbtools::toQString( app.id() ) )
    .arg(exitCode)
    .arg(exitStatus);
  QDateTime dt = QDateTime::currentDateTime();
  stopdates_.insert( proc, dt );

  QString logFile = proc->property(kPropLogFile).toString();
  if ( !logFile.isEmpty() ) {
    QFile file(logFile);
    if ( file.open(QFile::WriteOnly|QFile::Append) ) {
      QString dt = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
      QString app = pbtools::toQString(appmap_[reinterpret_cast<int64_t>(proc)].path());
      file.write(tr("%1 Процесс '%2' остановлен\n").arg(dt, app).toLocal8Bit());
      file.flush();
    }
  }
}

void Control::slotProcStarted()
{
  QMutexLocker lock(&mutex_);
  QProcess* proc = qobject_cast<QProcess*>( sender() );
  if ( nullptr == proc ) {
    error_log << QObject::tr("Неизвестная ошибка. Отправитель сигнала не процесс.");
    return;
  }

  info_log << tr("Запущен процесс [%1] %2")
                 .arg(proc->pid()).arg(pbtools::toQString(applications_.value(proc,Application()).path()));

  QString logFile = proc->property(kPropLogFile).toString();
  if ( !logFile.isEmpty() ) {
    QFile file(logFile);
    if ( file.open(QFile::WriteOnly|QFile::Append) ) {
      QString dt = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
      QString app = pbtools::toQString(appmap_[reinterpret_cast<int64_t>(proc)].path());
      file.write(tr("%1 Процесс '%2' запущен\n").arg(dt, app).toLocal8Bit());
    }
  }

  const Application& app = applications_[proc];
  info_log << QObject::tr("Процесс %1 запущен.")
    .arg( pbtools::toQString( app.id() ) );
  QDateTime dt = QDateTime::currentDateTime();
  startdates_.insert( proc, dt );
}

void Control::slotProcStateChanged( QProcess::ProcessState newState )
{
  QMutexLocker lock(&mutex_);
  if( newState == QProcess::Starting ){
    return;
  }
  QProcess* p = qobject_cast<QProcess*>( sender() );
  if ( nullptr == p ) {
    error_log << QObject::tr("Неизвестная ошибка. Отправитель сигнала не процесс.");
    return;
  }
  if ( true == applications_.contains(p) ) {
    QMap< rpc::Channel*, ProcSubscribe >::iterator it = changesubscribes_.begin();
    QMap< rpc::Channel*, ProcSubscribe >::iterator end = changesubscribes_.end();
    const Application& app = applications_[p];
    QDateTime dt = QDateTime::currentDateTime();
    for ( ; it != end; ++it ) {
      AppState::Proc* proc = it.value().first;
      proc->mutable_app()->CopyFrom(app);
      proc->set_id( reinterpret_cast<int64_t>(p) );
      AppState::ProcState st;
      if ( QProcess::Running == p->state() ) {
        st = AppState::PROC_RUNNING;
        startdates_.insert( p, dt );
      }
      else {
        st = AppState::PROC_STOPPED;
        stopdates_.insert( p, dt );
      }
      if ( true == startdates_.contains(p) ) {
        proc->set_startdt( startdates_[p].toString( Qt::SystemLocaleShortDate ).toStdString() );
      }
      if ( true == stopdates_.contains(p) ) {
        proc->set_stopdt( stopdates_[p].toString( Qt::SystemLocaleShortDate ).toStdString() );
      }
      proc->set_state(st);

      QStringList args;
      const Application& app = applications_[p];
      for ( int i = 0, sz = app.arg_size(); i < sz; ++i ) {
        args.append( pbtools::toQString( app.arg(i) ) );
      }
      it.value().second->Run();
    }
  }
  QString statestr;
  switch (newState) {
    case QProcess::NotRunning:
      statestr = QObject::tr("'не запущен'");
      break;
    case QProcess::Starting:
      statestr = QObject::tr("'запускается'");
      return; //пока нет пользы в такой информации      
    case QProcess::Running:
      statestr = QObject::tr("'запущен'");
      return; //дублируется в слоте slotProcStarted      
    default:
      statestr = QObject::tr("'неизвестно'");
      break;
  }
  const Application& app = applications_[p];
  info_log << QObject::tr("Состояние процесса %1 изменилось. Теперь его состояние = %2")
    .arg( pbtools::toQString( app.id() ) )
    .arg( statestr );
}

void Control::getStatus( int64_t proc, OperationStatus* status)
{
  QMutexLocker lock(&mutex_);
  OperationStatus stat = statusmap_.value(proc);  
  status->CopyFrom(stat);

  auto addDateParam = [status](const QString& title, const QDateTime& dt){
    if ( false == dt.isValid() ){
      return ;
    }

    OperationParam *param = status->add_param();
    param->set_state(app::OperationState_NORM);
    param->set_title(title.toStdString());
    auto dtParamHumanString = dateToHumanTimeShort(dt);
    param->set_dt(dtParamHumanString.toStdString());
    param->set_value(dtParamHumanString.toStdString());
  };


  QProcess* targetProcess = procmap_.value(proc, nullptr);
  if ( nullptr != targetProcess ){
    auto dtStart = startdates_.value(targetProcess, QDateTime());
    addDateParam(QObject::tr("Время запуска"), dtStart);

    auto dtStop = stopdates_.value(targetProcess, QDateTime());
    addDateParam(QObject::tr("Время остановки"), dtStop);
  }
}

void Control::getStatus( QProcess* proc, OperationStatus* status)
{
  int64_t id = procmap_.key( proc, -1 );
  if ( -1 == id) {  //FIXME функция getStatus должна быть булевая
    return;
  }
  return getStatus( id, status);
}

}
}
}
