#include "appstatus.h"

#include <cross-commons/debug/tlog.h>

#include <meteo/commons/planner/planner.h>
#include <commons/textproto/pbtools.h>

namespace mappi {
namespace cleaner {

const std::string AppStatus::kRunnig  = pbtools::toString(QObject::tr("выполняется"));
const std::string AppStatus::kStopped = pbtools::toString(QObject::tr("остановлено"));

AppStatus::AppStatus(meteo::Planner* planner, QObject* parent)
  : StatusSender(parent)
{
  planner_ = planner;
}

meteo::app::OperationStatus AppStatus::operationStatus() const
{
  bool hasWarnings = false;
  bool hasErrors   = false;

  meteo::app::OperationStatus status;

  std::string dt = QDateTime::currentDateTime().toString(Qt::SystemLocaleDate).toUtf8().constData();

  status.set_id(appId());

  QList<meteo::PlannerTask*> tasks = planner_->tasks();

  foreach ( meteo::PlannerTask* task, tasks ) {
    QString title = QString("#%1 - %2").arg(task->id(), 2, 10, QChar('0')).arg(task->name());
    data_[title] = meteo::app::OperationParam();
  }

  foreach ( const QString title, data_.keys() ) {
    data_[title].Clear();
    data_[title].set_title(pbtools::toString(title));
    data_[title].set_value(kStopped);
    data_[title].set_state(meteo::app::OperationState_NONE);
  }

  foreach ( meteo::PlannerTask* task, tasks ) {
    int64_t id = task->id();
    QString title = QString("#%1 - %2").arg(id, 2, 10, QChar('0')).arg(task->name());
    QString value;

    if ( task->isRunning() ) {
      value = QString::fromUtf8("выполняется");
    }
    else {
      QString nextRun = task->timesheet().nextRun().toString("hh:mm dd-MM-yyyy");
      value = QString::fromUtf8("завершено (следующий запуск в %2)").arg(nextRun);
    }

    data_[title].set_title(pbtools::toString(title));
    data_[title].set_value(pbtools::toString(value));
    data_[title].set_dt(dt);
    data_[title].set_state(meteo::app::OperationState_NONE);
  }

  QMapIterator<QString,meteo::app::OperationParam> it(data_);
  while ( it.hasNext() ) {
    it.next();
    status.add_param()->CopyFrom(it.value());
  }

  status.set_state(meteo::app::OperationState_NORM);
  if ( hasWarnings ) { status.set_state(meteo::app::OperationState_WARN); }
  if ( hasErrors )   { status.set_state(meteo::app::OperationState_ERROR); }

  return status;
}


} // cleaner
} // mappi
