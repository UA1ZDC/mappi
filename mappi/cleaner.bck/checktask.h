#ifndef METEO_MAPPI_CLEANER_CHECKTASK_H
#define METEO_MAPPI_CLEANER_CHECKTASK_H

#include <meteo/commons/planner/planner.h>

namespace mappi {
namespace cleaner {

class CheckTask : public meteo::PlannerTask
{
  Q_OBJECT
public:
  CheckTask(QObject* parent = 0) : PlannerTask(parent) {}

protected:
  virtual void run();
};

} // cleaner
} // mappi

#endif // METEO_MAPPI_CLEANER_CHECKTASK_H
