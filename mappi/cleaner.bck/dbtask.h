#ifndef METEO_MAPPI_CLEANER_DBTASK_H
#define METEO_MAPPI_CLEANER_DBTASK_H

#include <qstringlist.h>

#include <meteo/commons/planner/planner.h>

namespace mappi {
namespace cleaner {

class DbTask : public meteo::PlannerTask
{
  Q_OBJECT
public:
  DbTask(QObject* parent = 0) : PlannerTask(parent) {}
  virtual ~DbTask(){}

  void setConnectionName(const QString& connectionName) { connectionName_ = connectionName; }
  void addQuery(const QString& query) { queries_.append(query); }
  void addReindexQuery(const QString& query) { reindexQueries_.append(query); }
  void addVacuumQuery(const QString& query) { vacuumQueries_ += query; }
  void addAnalyzeQuery(const QString& query) { analyzeQueries_ += query; }

protected:
  virtual void run();

private:
  QString connectionName_;
  QStringList queries_;
  QStringList reindexQueries_;
  QStringList vacuumQueries_;
  QStringList analyzeQueries_;
};

} // cleaner
} // mappi

#endif // METEO_MAPPI_CLEANER_DBTASK_H
