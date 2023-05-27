#include <qdir.h>
#include <qprocess.h>
#include <qdatetime.h>
#include <qstringlist.h>
#include <qdiriterator.h>
#include <meteo/commons/global/global.h>
#include <cross-commons/app/paths.h>

#include "datasaver.h"
#include <qthread.h>

namespace meteo{

DataSaver::DataSaver(const QStringList& savepath, const QString& pgflag, const QString& restorepath, const QString& host) :
  savePath_(savepath),
  pgFlag_(pgflag),
  workPath_(restorepath),
  host_(host)
{
}

DataSaver::DataSaver(const QStringList &savepath, const QStringList &dbs, const QString &restorepath, const QString &host) :
  savePath_(savepath),
  dbs_(dbs),
  workPath_(restorepath),
  host_(host)
{

}

DataSaver::~DataSaver()
{
}

bool DataSaver::exec_app(const QString& app, const QStringList& arg)
{
  if( arg.isEmpty() ){
    return false;
  }
  QProcess p;

  connect( &p, SIGNAL( readyReadStandardError() ), SLOT( slotReadError()) );
  connect( &p, SIGNAL( readyReadStandardOutput() ), SLOT( slotReadOutput() ) );

  p.start(app, arg);
  if ( !p.waitForStarted() ) {
    return false;
  }
  p.waitForFinished(-1);
  if ( p.exitCode() != 0 ) {
    return false;
  }
  return true;
}

void DataSaver::save()
{
//  pg_dumpall();
  mongodump();
  make_tar();
  emit saveComplete();
}

void DataSaver::restore()
{
  extract();
  mongorestore();
  emit restoreComplete();
}

void DataSaver::slotReadError()
{
  QProcess* p = static_cast<QProcess*>(sender());
  if(0 == p){
    return;
  }
  QString read = p->readAllStandardError();
  emit newMessage("[E] "+ read);
}

void DataSaver::slotReadOutput()
{
  QProcess* p = static_cast<QProcess*>(sender());
  if(0 == p){
    return;
  }
  QString read = p->readAllStandardOutput();
  emit newMessage("[I] " + read);
}

void DataSaver::extract()
{
  QDir restore(workPath_);
  if ( false == restore.exists() ){
    QString err =  QObject::tr("[E] Нет данных для восстановления");
    emit newMessage(err);
    return;
  }

  QStringList files = restore.entryList(QDir::NoDotAndDotDot	| QDir::Hidden | QDir::Files);

  for ( auto file : files ) {
    QStringList args;
    args.append("-xzf");
    args.append(workPath_ + '/' + file);
    args.append("-C");
    args.append(QDir::rootPath());
    exec_app("tar", args);
  }
}

void DataSaver::mongorestore()
{
  emit newMessage(QObject::tr("[I] Восстановление баз данных..."));
  emit newMessage(workPath_);
  QDir work(workPath_ + "/mongodump");
  if ( false == work.exists() ) {
    emit newMessage(QObject::tr("[E] Нет директории восстановления базы данных"));
    return;
  }
  QStringList args;
  args.append(QString("--host=%1").arg(host_));
  args.append(QString("--port=27017"));
  args.append(QString("--dir=%1").arg(workPath_ + "/mongodump/"));
  if ( true == dropBeforeRestore_ ) {
    args.append(QString("--drop"));
  }
  exec_app("mongorestore", args);
}

void DataSaver::mongodump()
{
  emit newMessage(QObject::tr("[I] Создание дампа баз данных..."));
  QDir restore(workPath_);
  if ( false == restore.exists() ){
    if( false == restore.mkpath(workPath_) ){
      QString err = QObject::tr("[E] Не удалось создать директорию %1 для сохранения файлов").arg(restore.absolutePath());
      emit newMessage(err);
      return;
    }
  }
  for ( auto db : dbs_ ) {
      auto conf = meteo::Global::instance()->dbConf(db);
      QString hostArg = QString("--host=%1").arg(conf.host());
      QString portArg = QString("--port=%1").arg(conf.port());
      QString dbArg = QString("--db=%1").arg(conf.name());
      QString outArg = QString("--out=%1").arg(workPath_ + "/mongodump/");
      QString scanArg = QString("--forceTableScan");
      QStringList args = QStringList() << hostArg << portArg << dbArg << scanArg << outArg;
      emit newMessage(QObject::tr("[I] Создание дампа коллекции %1").arg(conf.name()));
      exec_app("mongodump", args);
      emit newMessage(QObject::tr("[I] Создание дампа коллекции %1 завершено").arg(conf.name()));
  }
}

void DataSaver::make_tar()
{
  QDir restore(workPath_);
  if ( false == restore.exists() ){
    if( false == restore.mkpath(workPath_) ){
      QString err = QObject::tr("[E] Не удалось создать директорию %1 для сохранения файлов").arg(restore.absolutePath());
      emit newMessage(err);
    }
  }
  for ( auto path : savePath_ ){
    QDir dir(path);
    if( true ==  dir.exists() ){
      QStringList args;
      args.append("--dereference");
      args.append("-czf");
      args.append(workPath_ + dir.dirName() + ".tar.gz");
      args.append(dir.absolutePath());
      emit newMessage(QObject::tr("[I] Архивирование директории %1...").arg(path));
      exec_app("tar", args );
    }
    else{
      emit newMessage(QObject::tr("[W] Директория %1 не существует.").arg(path));
    }
  }

  QDir dump(workPath_ + "/mongodump" );
  if( false == dump.exists() ){
    return;
  }
  QStringList args;
  args.append("--dereference");
  args.append("-czf");
  args.append(workPath_ + "/db.tar.gz");
  args.append(workPath_ + "/mongodump");
  emit newMessage(QObject::tr("[I] Архивирование дампа БД %1...").arg(workPath_ + "/mongodump"));
  exec_app("tar", args );
  dump.removeRecursively();
}

}
