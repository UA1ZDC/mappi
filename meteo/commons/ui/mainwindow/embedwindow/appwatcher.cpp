#include "appwatcher.h"

#include <qapplication.h>
#include <qfile.h>
#include <qmessagebox.h>

#include <meteo/commons/global/log.h>
#include <cross-commons/debug/tlog.h>

#include "preloader.h"

#include "appcontainer_windows.h"

namespace meteo {
namespace app {

AppWatcher::AppWatcher(QObject *parent) :
  QObject(parent),
  mainwindow_(0)
{
}

AppWatcher::~AppWatcher()
{
  //   qDeleteAll(procX11_);
  //   procX11_.clear();
}

QWidget* AppWatcher::create(const QString& app, const QStringList& args, const QString& title)
{


  if( true == appnameX11_.contains(title) ){
    QWidget* w = appnameX11_[title];
    if( 0 != w ){
      w->activateWindow();
      return w;
    }
  }
  if( false == QFile::exists(app) ){
    QApplication::restoreOverrideCursor();
    QMessageBox::warning(0, "Ошибка запуска приложения",meteo::msglog::kFileNotFound.arg(app));
    return 0;
  }

  AppContainer* x11 = new AppContainer;
  x11->setAttribute(Qt::WA_DeleteOnClose, true);

  QProcess* proc = new QProcess;
  connect( x11, SIGNAL(destroyed(QObject*)), SLOT(slotContainerDestroyed(QObject*)) );
  connect( proc, SIGNAL( readyReadStandardError() ), SLOT( slotReadError()) );
  connect( proc, SIGNAL( readyReadStandardOutput() ), SLOT( slotReadOutput() ) );
  connect( proc, SIGNAL( finished( int, QProcess::ExitStatus) ), SLOT( slotProcFinished(int,QProcess::ExitStatus) ) );

  procX11_.insert(proc, x11);
  appnameX11_.insert(title, x11);
  //mainwindow_->toMdi(x11);
  x11->resize(800, 500);
  //x11->show();
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  env.insert( "METEO_XEMBED_WINID", QString::number((uint64_t)(x11->winId())) );
  proc->setProcessEnvironment( env );
  proc->start(app, args);
  x11->setWindowTitle(title);
  return x11;
}

void AppWatcher::slotReadError()
{
  QProcess* p = static_cast<QProcess*>(sender());
  if(0 == p){
    return;
  }
  if( procX11_.contains(p) ){
    QString read = p->readAllStandardError();
    if( 0 != procX11_[p] && 0 != procX11_[p]->preloader() ){
      procX11_[p]->preloader()->setTextLog(read);
    }
    debug_log << read;
  }
}

void AppWatcher::slotReadOutput()
{
  QProcess* p = static_cast<QProcess*>(sender());
  if(0 == p){
    return;
  }
  if( procX11_.contains(p) ){
    QString read = p->readAllStandardOutput();
    if( 0 != procX11_[p] && 0 != procX11_[p]->preloader() ){
      procX11_[p]->preloader()->setTextLog(read);
    }
  }
}

void AppWatcher::slotProcFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
  QProcess* p = static_cast<QProcess*>(sender());
  if(0 == p){
    return;
  }
  if( procX11_.contains(p) ){
    if( 0 != procX11_[p] && 0 != procX11_[p]->preloader() ){
      procX11_[p]->preloader()->setAppFinish(exitCode, exitStatus);
      procX11_[p]->close();
    }
  }
}

void AppWatcher::slotContainerDestroyed(QObject *o)
{
  AppContainer* x11 = static_cast<AppContainer*>(o);
  if( 0 != x11 ){
    if( true == appnameX11_.values().contains(x11)){
      QString appname = appnameX11_.key(x11);
      appnameX11_.remove(appname);
    }
    QProcess* p = procX11_.key(x11);
    procX11_.remove(p);
    p->terminate();
    if (!p->waitForFinished(1000)) {
      p->kill();
      p->waitForFinished();
    }
    delete p;
  }
}

} //meteo
} //app
