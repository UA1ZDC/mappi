#include "cronapp.h"
#include "runapp.h"

#include <qdir.h>

#include <cross-commons/debug/tlog.h>

#include <commons/textproto/pbtools.h>

#include <meteo/commons/planner/planner.h>
#include <meteo/commons/proto/cron.pb.h>

namespace meteo {

CronApp::CronApp(const QString& path, Planner* planner, QObject* parent)
  : ConfigLoader(path, true, parent)
{
  path_ = path;
  planner_ = planner;
}

bool CronApp::slotLoad(const QString& path)
{
  Q_UNUSED( path );

  cron::Settings opt;

  QDir dir(path_);
  QStringList confList = dir.entryList(QStringList("*.conf"), QDir::Files);
  for ( int i=0,isz=confList.size(); i<isz; ++i ) {
    cron::Settings part;
    QString filename = dir.absoluteFilePath( confList[i] );
    if ( !loadConfigPart(&part, filename) ) {
      warning_log << QObject::tr("Не удалось обработать файл %1.").arg(filename);
      continue;
    }
    opt.MergeFrom(part);
  }

  loadRunApps(opt);
  return true;
}

void CronApp::loadRunApps(const cron::Settings& opt)
{
  QStringList md5List;

  for ( int i=0, isz=opt.run_app_size(); i<isz; ++i ) {
    md5List += pbtools::md5hash(opt.run_app(i));
  }

  for ( int i=0, isz=opt.run_app_list_size(); i<isz; ++i ) {
    md5List += pbtools::md5hash(opt.run_app_list(i));
  }

  // удаляем задачи, для которых нет конфигурации

  foreach ( int64_t id, tasks_.keys() ) {
    if ( !md5List.contains(tasks_[id]) ) {
      planner_->removeTask(id);
      tasks_.remove(id);
    }
  }

  for ( int i = 0, isz = opt.run_app_size(); i < isz; ++i ) {
    const cron::RunApp& appconf = opt.run_app(i);

    QString md5 = pbtools::md5hash(appconf);
    if ( tasks_.values().contains(md5) ) { continue; }

    RunApp* task = createTask(appconf);
    if ( 0 == task ) { continue; }

    int64_t id = planner_->addTask(task);
    tasks_[id] = md5;
  }

  for ( int i = 0, isz = opt.run_app_list_size(); i < isz; ++i ) {
    const cron::RunAppList& appconf = opt.run_app_list(i);

    QString md5 = pbtools::md5hash(appconf);
    if ( tasks_.values().contains(md5) ) { continue; }

    RunApp* task = createTask(appconf);
    if ( 0 == task ) { continue; }

    int64_t id = planner_->addTask(task);
    tasks_[id] = md5;
  }
}

RunApp* CronApp::createTask(const cron::RunApp& opt) const
{
  QString strOpt = pbtools::toQString(opt.Utf8DebugString());

  Timesheet ts = Timesheet::fromString(pbtools::toQString(opt.timesheet()));

  if ( !ts.isValid() ) {
    error_log << tr("Расписание задано некорректно. Конфигурация запуска приложения:\n %1").arg(strOpt);
    return 0;
  }

  QString path = pbtools::toQString(opt.path());

  if ( path.isEmpty() ) {
    error_log << QObject::tr("Не указан путь к приложению. Конфигурация запуска приложения:\n %1").arg(strOpt);
    return 0;
  }

  RunApp* task = new RunApp;
  task->setTimesheet(ts);
  task->setName(pbtools::toQString(opt.name()));

  int appId = task->addApp(path);

  for ( int j=0,jsz=opt.arg_size(); j<jsz; ++j ) {
    const cron::RunApp::Arg& arg = opt.arg(j);

    if ( !arg.has_key() ) {
      warning_log << QObject::tr("НЕ задано имя параметра в параметрах запуска приложения %1")
                                .arg(pbtools::toQString(opt.Utf8DebugString()));
      continue;
    }

    if ( false == arg.has_value() ) {
      task->addArg(appId, pbtools::toQString(arg.key()));
    }
    else {
      task->addArg(appId, pbtools::toQString(arg.key()), pbtools::toQString(arg.value()));
    }
  }

  return task;
}

RunApp* CronApp::createTask(const cron::RunAppList& opt) const
{
  QString strOpt = pbtools::toQString(opt.Utf8DebugString());

  Timesheet ts = Timesheet::fromString(pbtools::toQString(opt.timesheet()));

  if ( !ts.isValid() ) {
    error_log << tr("Расписание задано некорректно. Конфигурация запуска приложения:\n %1").arg(strOpt);
    return 0;
  }

  RunApp* task = new RunApp;
  task->setTimesheet(ts);
  task->setName(pbtools::toQString(opt.name()));

  for ( int i=0,isz=opt.app_size(); i<isz; ++i ) {
    const cron::RunItem& app = opt.app(i);

    QString path = pbtools::toQString(app.path());

    if ( path.isEmpty() ) {
      error_log << QObject::tr("Не указан путь к приложению. Конфигурация запуска приложения:\n %1").arg(strOpt);
      continue;
    }

    int appId = task->addApp(path);
    for ( int j=0,jsz=app.arg_size(); j<jsz; ++j ) {
      const cron::RunApp::Arg& arg = app.arg(j);

      if ( !arg.has_key() ) {
        warning_log << QObject::tr("НЕ задано имя параметра в параметрах запуска приложения %1")
                                  .arg(pbtools::toQString(opt.Utf8DebugString()));
        continue;
      }

      if ( false == arg.has_value() ) {
        task->addArg(appId, pbtools::toQString(arg.key()));
      }
      else {
        task->addArg(appId, pbtools::toQString(arg.key()), pbtools::toQString(arg.value()));
      }
    }
  }

  return task;
}

} // meteo
