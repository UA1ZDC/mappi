#ifndef METEO_MAPPI_CLEANER_CLEARTABLETASK_H
#define METEO_MAPPI_CLEANER_CLEARTABLETASK_H

#include <meteo/commons/planner/planner.h>

namespace mappi {
namespace cleaner {

class ClearTableTask : public meteo::PlannerTask
{
  Q_OBJECT
public:
  ClearTableTask(QObject* parent = 0) : PlannerTask(parent), lifeTime_(-1) {}

  void setConnectionName(const QString& connectionName) { connectionName_ = connectionName; }
  void setTableName(const QString& name) { tableName_ = name; }
  void setDateTimeFieldName(const QString& name) { dtField_ = name; }
  void setPathFieldName(const QString& name) { pathField_ = name; }

protected:
  virtual void run();

private:
  QString connectionName_;
  QString tableName_;
  QString idField_;
  QString dtField_;
  QString pathField_;
  int lifeTime_;
};

} // cleaner
} // mappi

#endif // METEO_MAPPI_CLEANER_CLEARTABLETASK_H
