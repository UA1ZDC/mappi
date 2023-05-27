#ifndef METEO_MAPPI_CLEANER_ARCHIVETASK_H
#define METEO_MAPPI_CLEANER_ARCHIVETASK_H

#include <meteo/commons/planner/planner.h>

namespace mappi {
namespace cleaner {

class ArchiveTask : public meteo::PlannerTask
{
  Q_OBJECT
public:
  ArchiveTask(QObject* parent = 0);
  virtual ~ArchiveTask(){}

  void setPath(const QString& path) { path_ = path; }

protected:
  virtual void run();

private:
  bool testTar(const QString& filePath) const;

private:
  QString path_;
  int olderThan_;
};

} // cleaner
} // mappi

#endif // METEO_MAPPI_CLEANER_ARCHIVETASK_H
