#ifndef METEO_MAPPI_CLEANER_REMOVETASK_H
#define METEO_MAPPI_CLEANER_REMOVETASK_H

#include <qstringlist.h>
#include <meteo/commons/planner/planner.h>

namespace mappi {
namespace cleaner {

class RemoveTask : public meteo::PlannerTask
{
  Q_OBJECT
public:
  explicit RemoveTask(QObject* parent = 0);
  virtual ~RemoveTask() {}

  void setArchivePath(const QString& path) { _paths << path; }
  void setTelegramsPath(const QString& path) { _paths << path; }
  void setQuery(const QString& query) { query_ = query; }

protected:
  virtual void run();

private:
  bool execQuery() const;
  void removeDir(const QString& dirName) const;
  void removeFile(const QString& fileName) const;
  bool needRemove(const QString& fileName) const;

private:
  int lifeTime_;
  QStringList _paths;
  QString query_;
};

} // cleaner
} // mappi

#endif // METEO_MAPPI_CLEANER_REMOVETASK_H
