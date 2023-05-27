#ifndef MAPPI_SCHEDULE_SCHEDULER_H
#define MAPPI_SCHEDULE_SCHEDULER_H

#include <mappi/schedule/schedule.h>
#include <mappi/schedule/scheduleinfo.h>

namespace mappi {
  namespace schedule {

    class Scheduler {

    public :

      Scheduler(mappi::conf::RecvMode mode = mappi::conf::kRateUnk);
      ~Scheduler();

      bool existTLE() const;

      bool create( Schedule& sched );
      bool create( Schedule& sched, const QDateTime& start, const QDateTime& end );
      bool refresh( Schedule& sched);
      bool refresh( Schedule& sched, const QDateTime& start, const QDateTime& end);
      void resolveConfl( Schedule& sched, bool saveUserConfl = false ) const;

      static bool save( Schedule& sched, const QString& path );
      static bool restore( Schedule& sched, const QString& path );

    private:
      Scheduler( const Scheduler& object );
      Scheduler& operator=( const Scheduler& object );
      bool readSettings(mappi::conf::RecvMode mode = mappi::conf::kRateUnk);

    private :

      ScheduleInfo m_schedInfo;


    };

  }
}

#endif //_SCHEDULER_
