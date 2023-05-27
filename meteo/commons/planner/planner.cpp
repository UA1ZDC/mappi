#include "planner.h"

#include <qtimer.h>

#include <cross-commons/debug/tlog.h>


namespace meteo {

Planner::Planner(QObject* parent) :
  QObject(parent),
  timer_(0)
{
  timer_ = new QTimer(this);
  connect( timer_, SIGNAL(timeout()), SLOT(slotProcessTasks()) );
  timer_->start(60000);
}

Planner::~Planner()
{
  stopAllTasks();

  qDeleteAll(tasks_.keys());

  timer_->deleteLater();
  timer_ = 0;
}

int64_t Planner::addTask(PlannerTask* task)
{
  static uint id = 0;

  task->setId(++id);
  task->setObjectName(QString("Task%1").arg(id, 2, 10, QChar('0')));

  tasks_.insert(task, QDateTime());
  connect( task, SIGNAL(finished()), SLOT(slotTaskFinished()) );

  info_log << tr("Добавлено задание #%1").arg(task->id(), 2, 10, QChar('0')) << task->name();

  emit added(task->id());

  return task->id();
}

void Planner::removeTask(int64_t taskId)
{
  foreach ( PlannerTask* task, tasks_.keys() ) {
    if ( task->id() != taskId ) { continue; }

    tasks_.remove(task);

    debug_log << tr("Остановка задания #%1 '%2'").arg(task->id(), 2, 10, QChar('0')).arg(task->name());

    if ( task->isRunning() ) {
      task->quit();
      task->wait(30000);
    }
    if ( task->isRunning() ) {
      task->terminate();
      task->wait();
    }

    emit removed(task->id());
    task->deleteLater();
  }
}

PlannerTask* Planner::task(int taskId) const
{
  foreach ( PlannerTask* task, tasks_.keys() ) {
    if ( task->id() == taskId ) {
      return task;
    }
  }
  return nullptr;
}

void Planner::stopAllTasks()
{
  foreach ( PlannerTask* task, tasks_.keys() ) {
    debug_log << tr("Остановка задания #%1 '%2'").arg(task->id(), 2, 10, QChar('0')).arg(task->name());

    if ( task->isRunning() ) {
      task->quit();
      task->wait(30000);
    }
    if ( task->isRunning() ) {
      task->terminate();
      task->wait();
    }
  }
}

void Planner::slotProcessTasks()
{
  QDateTime dt = QDateTime::currentDateTimeUtc();

  QMap<PlannerTask*,QDateTime>::iterator cur = tasks_.begin();
  QMap<PlannerTask*,QDateTime>::iterator end = tasks_.end();
  for ( ; cur!=end; ++cur ) {
    if ( !cur.value().isValid() && cur.key()->timesheet().match(dt) ) {
      tasks_[cur.key()] = dt;
      info_log << tr("Запуск задания #%1 '%2' --- %3").arg(cur.key()->id(), 2, 10, QChar('0'))
                                                          .arg(cur.key()->name())
                                                          .arg(dt.toString("hh:mm:ss dd-MM-yyyy"));
      cur.key()->start();
    }
  }
}

void Planner::slotTaskFinished()
{
  PlannerTask* task = qobject_cast<PlannerTask*>(sender());

  if ( 0 == task ) { return; }

  if ( tasks_.contains(task) ) {
    int time = tasks_[task].secsTo(QDateTime::currentDateTimeUtc());
    info_log << tr("Выполнение задания #%1 '%2' завершено").arg(task->id(), 2, 10, QChar('0')).arg(task->name());
    debug_log << tr("Время выполнения %1 сек.").arg(time);
    tasks_[task] = QDateTime();
  }
}

} // meteo
