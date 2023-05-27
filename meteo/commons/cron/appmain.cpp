#include <csignal>

#include <qfile.h>
#include <qdir.h>
#include <qcoreapplication.h>

#include <cross-commons/app/paths.h>
#include <cross-commons/debug/tlog.h>

#include <meteo/commons/planner/planner.h>

#include <commons/textproto/tprototext.h>
#include <commons/textproto/pbtools.h>

#include <meteo/commons/global/global.h>
#include <meteo/commons/global/appstatusthread.h>
#include <meteo/commons/proto/cron.pb.h>
#include <meteo/commons/settings/settings.h>
#include <meteo/commons/cron/cronapp.h>

namespace
{

meteo::Planner* planner = 0;

void quitApp(int)
{
  if ( 0 != planner ) {
    planner->stopAllTasks();
    delete planner;
  }
  ::exit(EXIT_SUCCESS);
}

}

int appMain(int argc,  char** argv)
{
  ::signal(SIGINT, &(quitApp));
  ::signal(SIGTERM, &(quitApp));
  ::signal(SIGQUIT, &(quitApp));

  try {
    TAPPLICATION_NAME("meteo");
    meteo::global::setLogHandler();

    QCoreApplication app(argc, argv);

    meteo::gSettings(new ::meteo::Settings);

    if ( !meteo::gSettings()->load() ) {
      error_log << QObject::tr("Не удалось загрузить настройки");
      return EXIT_FAILURE;
    }

    planner = new meteo::Planner;

    meteo::CronApp cron(MnCommon::etcPath() + "cron.d", planner);
    if ( !cron.slotLoad() ) {
      return EXIT_FAILURE;
    }

    QTimer::singleShot( 0, planner, SLOT(slotProcessTasks()) ); // hack to fast start

    meteo::AppStatusThread* status = new meteo::AppStatusThread;
    status->setUpdateLimit(1);
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
          QDateTime curdt = QDateTime::currentDateTimeUtc();
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

    QObject::connect( planner, &meteo::Planner::removed, [=](int taskId){
      status->setParam(taskId, QObject::tr("ОСТАНОВЛЕНО"));
    });

    QObject::connect( planner, &meteo::Planner::added, [=](int taskId){
      meteo::PlannerTask* task = planner->task(taskId);
      QString title = QString("#%1 - %2").arg(taskId, 2, 10, QChar('0')).arg(task->name());
      status->setTitle(taskId, title);

      QString value;
      if ( task->isRunning() ) {
        value = QString::fromUtf8("ВЫПОЛНЯЕТСЯ");
      }
      else {
        QString nextRun = task->timesheet().nextRun().toString("hh:mm dd-MM-yyyy");
        value = QString::fromUtf8("ЗАВЕРШЕНО (следующий запуск в %2)").arg(nextRun);
      }

      status->setParam(taskId, value);
    });

    app.exec();

    status->terminate();
    status->wait(5000);

    delete planner;
    delete status;
  }
  catch( const std::bad_alloc& ) {
    critical_log << QObject::tr("Недостаточно памяти для работы программы");
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
