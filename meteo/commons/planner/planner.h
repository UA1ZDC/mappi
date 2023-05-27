#ifndef METEO_PLANNER_PLANNER_H
#define METEO_PLANNER_PLANNER_H

#include <qmap.h>
#include <qthread.h>
#include <qdatetime.h>

#include <meteo/commons/planner/timesheet.h>

class QTimer;

namespace meteo {

/*! PlannerTask - базовый класс для задач, которые необходимо выполнять по расписанию.
 */
class PlannerTask : public QThread
{
  Q_OBJECT
public:
  explicit PlannerTask(QObject* parent = 0) : QThread(parent), id_(-1) {}
  virtual ~PlannerTask(){}

  //! Устанавливает название задачи
  void setName(const QString& name) { name_ = name; }
  //! Устанавливает периодичность запуска задачи. Параметр
  void setTimesheet(const Timesheet& timesheet) { timesheet_ = timesheet; }

  //! Идентификатор задачи (назначается планировщиком)
  int64_t id() const                { return id_; }
  //! Название задачи
  QString name() const              { return name_; }
  //! Периодичность запуска
  const Timesheet& timesheet() const { return timesheet_; }

private:
  void setId(int64_t id)  { id_ = id; }

private:
  int64_t id_;
  QString name_;
  Timesheet timesheet_;

  friend class Planner;
};

//! Класс Planner управляет запуском задач.
class Planner : public QObject
{
  Q_OBJECT
public:
  explicit Planner(QObject *parent = 0);
  virtual ~Planner();

  /*! Возвращает идентификатор задачи.
   *
   * При добавлении задачи Planner берёт на себя ответственность по освобождению памяти объекта task. Используйте
   * removeTask(), чтобы исключить задачу из расписания.
   */
  int64_t addTask(PlannerTask* task);
  void removeTask(int64_t taskId);
  //! \todo void removeTask(PlannerTask* task);
  //! \todo PlannerTask* takeTask(int taskId);
  PlannerTask* task(int taskId) const;
  //! \todo bool isRun(int taskId) const;
  void stopAllTasks();

  QList<PlannerTask*> tasks() const { return tasks_.keys(); }
  QDateTime startTime(PlannerTask* task) const { return tasks_.value(task, QDateTime()); }

signals:
  //! \todo void taskStart(int taskId);
  //! \todo void taskFinished(int taskId);
  void removed(int taskId);
  void added(int taskId);

private slots:
  void slotProcessTasks();
  void slotTaskFinished();

private:
  QTimer* timer_;
  QMap<PlannerTask*,QDateTime> tasks_;  // если QDateTime не задано, значит задача не выполняется
};

} // meteo

#endif // METEO_PLANNER_PLANNER_H
