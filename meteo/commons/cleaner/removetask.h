#ifndef METEO_COMMONS_CLEANER_REMOVETASK_H
#define METEO_COMMONS_CLEANER_REMOVETASK_H

#include "archutil.h"
#include <meteo/commons/planner/planner.h>


namespace meteo {
namespace cleaner {

class RemoveTask : public PlannerTask
{
  Q_OBJECT

public:
  explicit RemoveTask(QObject* parent = nullptr);

  void setOptions(const FileDbRemove& options);

protected:
  virtual void run();

private:
  void jobRemoveDocsGridFsMongodb();
  void jobRemove();

  int64_t flushExpired(const QString& root, const QDateTime& expired);
  int64_t flushExpired(const QDir& dir, const QDateTime& expired);

private:
  FileDbRemove opt_;
};

}
}

#endif
