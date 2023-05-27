#include "test.h"

#include <cross-commons/debug/tlog.h>
#include <meteo/commons/planner/planner.h>

#include <QtCore>


int main(int argc, char** argv)
{
  //QTextCodec::setCodecForCStrings( QTextCodec::codecForLocale() );
  QCoreApplication app(argc, argv);

  QThreadPool::globalInstance()->setMaxThreadCount(10);

  meteo::Planner planner;
  TestTask* task = new TestTask;
  task->setName(QObject::tr("Запуск каждую минуту"));
  task->setTimesheet(meteo::Timesheet::fromString("* * * * *"));
  planner.addTask(task);

  task = new TestTask;
  task->setName(QObject::tr("Ежедневно до обеда"));
  task->setTimesheet(meteo::Timesheet::fromString("* 0-12 * * *"));
  planner.addTask(task);

  task = new TestTask;
  task->setName(QObject::tr("Ежедневно после обеда"));
  task->setTimesheet(meteo::Timesheet::fromString("* 13-23 * * *"));
  planner.addTask(task);

  task = new TestTask;
  task->setName(QObject::tr("Только в пн, вт, пт"));
  task->setTimesheet(meteo::Timesheet::fromString("* * * * 1,2,5"));
  planner.addTask(task);

  task = new TestTask;
  task->setName(QObject::tr("Только в ср и чт"));
  task->setTimesheet(meteo::Timesheet::fromString("* * * * 3-4"));
  planner.addTask(task);

  return app.exec();
}
