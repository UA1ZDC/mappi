#ifndef MAPPI_SCHEDULE_SCHEDULE_H
#define MAPPI_SCHEDULE_SCHEDULE_H

#include <mappi/schedule/session.h>
#include <mappi/schedule/scheduleinfo.h>

namespace mappi {
  namespace schedule {


    class Schedule {

    public :

      Schedule();
      Schedule( const Schedule& object );
      Schedule& operator=( const Schedule& object );
      ~Schedule();

      bool create( const ScheduleInfo& schedInfo );
      bool refresh( const ScheduleInfo& schedInfo );
      void resolveConfl( mappi::conf::PriorType settlType, bool saveUserConfl = false );
      void print() const;
      void clear();

      Schedule acceptedSessions() const;

      bool isEmpty() const;
      int countCompletedSessions(bool isReceived = true) const;
      unsigned int countSession() const;
      int getNumberSession( Session session ) const;
      Session firstSession() const;
      Session lastSession() const;
      Session nearSession(bool withCurrent = true) const;
      Session nearSession(const QDateTime& dt, bool withCurrent = true) const;
      Session nextSession() const;
      Session& getSession( unsigned int i );
      Session& getSession( const QString& name, const unsigned revolve );
      const Session& getSession( unsigned int i ) const;
      void appendSession( const Session& session );
      void prependSession( const Session& session );

      QDateTime start() const;
      QDateTime end() const;

      bool setSessionState( const Session& session);

      static Session& defaultSession();
    private :

      QList< satellite::Schedule > m_list;
      SatSchedule m_satSchedule;

    };
  }

}

#endif 
