#ifndef METEO_PLANNER_TEST_TEST_H
#define METEO_PLANNER_TEST_TEST_H

#include <cross-commons/debug/tlog.h>
#include <meteo/commons/planner/planner.h>

#include <QtCore>

class TestTask : public meteo::PlannerTask
{
  Q_OBJECT
public:
  TestTask(QObject* parent = 0) : PlannerTask(parent) {}
  virtual ~TestTask(){}

  virtual void run()
  {
    info_log << QString(25, ' ').insert(0, name()).left(25)
             << QDateTime::currentDateTime().toString(Qt::LocaleDate);

    QElapsedTimer t;
    t.start();
    while ( t.elapsed() < 5000 ) {
      // do nothing ...
    }
  }
};

#endif // METEO_PLANNER_TEST_TEST_H

