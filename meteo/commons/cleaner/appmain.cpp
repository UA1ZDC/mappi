#include <qdir.h>
#include <qfile.h>
#include <qtimer.h>
#include <qtextcodec.h>
#include <qcoreapplication.h>

#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/targ.h>
#include <cross-commons/app/paths.h>

#include <commons/textproto/tprototext.h>

#include <meteo/commons/cleaner/removetask.h>

#include <meteo/commons/global/appstatusthread.h>
#include <meteo/commons/planner/planner.h>
#include <meteo/commons/proto/cleaner.pb.h>

#include <meteo/commons/global/global.h>
#include <meteo/commons/settings/settings.h>

const QStringList kPathOpt = QStringList() << QObject::tr("p")
                                           << QObject::tr("path");


void out(tlog::Priority priority, const QString& facility, const QString& fileName, int line, const QString& msg)
{
  QString m = msg;
  meteo::PlannerTask* task = qobject_cast<meteo::PlannerTask*>(QThread::currentThread());
  if ( 0 != task ) {
    m.prepend("#" + QString::number(task->id()) + " ");
  }

  TLog::stdOut(priority, facility, fileName, line, m);
}

int appMain(int argc, char** argv)
{
  TAPPLICATION_NAME("meteo");
  meteo::global::setLogHandler();

  try {
    QCoreApplication app(argc, argv);

    TLog::setMsgHandler( &out );

    ::meteo::gSettings(new ::meteo::Settings);

    if ( !meteo::gSettings()->load() ) {
      error_log << QObject::tr("Ошибка при загрузке настроек.");
      return EXIT_FAILURE;
    }

    TArg args(argc,argv);

    QDir dir(args.value(kPathOpt, MnCommon::etcPath() + "cleaner.conf.d"));
    if ( false ==  dir.exists() ) {
      error_log << QObject::tr("Отсутствует директория с конфигурационными файлами '%1'.").arg(dir.dirName());
      return EXIT_FAILURE;
    }

    meteo::cleaner::Tasks tasks;

    foreach ( const QFileInfo& f, dir.entryInfoList(QStringList("*.conf"), QDir::Files) ) {
      meteo::cleaner::Tasks part;
      if ( false  == TProtoText::fromFile(f.absoluteFilePath(), &part) ) {
        warning_log << QObject::tr("Не удалось обработать файл %1").arg(f.absoluteFilePath());
        continue;
      }
      tasks.MergeFrom(part);
    }

    meteo::Planner* planner = new meteo::Planner;

    for ( int i = 0, isz = tasks.remove_size(); i < isz; ++i ) {
      const meteo::cleaner::FileDbRemove& opt = tasks.remove(i);

      meteo::Timesheet ts = meteo::Timesheet::fromString(QString::fromStdString(opt.timesheet()));
      meteo::cleaner::RemoveTask* task = new meteo::cleaner::RemoveTask;
      task->setName(QString::fromStdString(opt.name()));
      task->setTimesheet(ts);
      task->setOptions(opt);

      error_log_if( !ts.isValid() ) << QObject::tr("Шаблон расписания некорректен (db_task)");

      planner->addTask(task);
    }

    if ( true == planner->tasks().isEmpty() ){
      error_log << QObject::tr("Нет заданий в планировщике. Завершение работы");
      return 0;
    }

    QTimer::singleShot( 0, planner, SLOT(slotProcessTasks()) ); // fast start

    meteo::AppStatusThread* status = new meteo::AppStatusThread;
    status->setUpdateLimit(1);
    status->setSendLimit(500);
    status->start();

    for ( meteo::PlannerTask* task : planner->tasks() ) {
      QString title = QString("#%1 - %2").arg(task->id(), 2, 10, QChar('0')).arg(task->name());
      status->setTitle(task->id(), title);

      QString value;
      if ( task->isRunning() ) {
        value = QString::fromUtf8("ВЫПОЛНЯЕТСЯ");
      }
      else {
        QString nextRun = task->timesheet().nextRun().toString("hh:mm dd-MM-yyyy");
        value = QString::fromUtf8("ЗАВЕРШЕНО (следующий запуск в %2)").arg(nextRun);
      }

      status->setParam(task->id(), value);
    }

    QTimer* timer = new QTimer;
    timer->start(1000);
    QObject::connect( timer, &QTimer::timeout, [=]{
      for ( meteo::PlannerTask* task : planner->tasks() ) {
        QString value;
        if ( task->isRunning() ) {
          QDateTime taskdt = planner->startTime(task);
          QDateTime curdt = QDateTime::currentDateTime();
          int sec = taskdt.secsTo(curdt);

          int h = sec/3600.;
          int m = (sec - h*3600)/60.;
          int s = (sec - h*3600 - m*60);

          value = QString::fromUtf8("ВЫПОЛНЯЕТСЯ (%1 ч. %2 мин. %3 сек.)").arg(h).arg(m).arg(s);
        }
        else {
          QString nextRun = task->timesheet().nextRun().toString("hh:mm dd-MM-yyyy");
          value = QString::fromUtf8("ЗАВЕРШЕНО (следующий запуск в %2)").arg(nextRun);
        }

        status->setParam(task->id(), value);
      }
    });

    app.exec();

    status->terminate();
    status->wait(5000);

    delete planner;
    delete status;
  }
  catch(const std::bad_alloc& ) {
    critical_log << QObject::tr("Недостаточно памяти для работы приложения");
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
