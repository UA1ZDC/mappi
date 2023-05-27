#ifndef MAPPI_SCHEDULE_SAT_SHEDULE_H
#define MAPPI_SCHEDULE_SAT_SHEDULE_H

#include <qstring.h>
#include <qdatetime.h>

//#include <mappi/common/mappi_defines.h>
#include <commons/geobasis/coords.h>
#include <mappi/proto/reception.pb.h>


class Satellite;

namespace mappi {
  namespace satellite {

    struct Schedule {
      QString             name;       //!< Имя спутника
      QDateTime           aos;        //!< Момент времени, когда сигнал будет получен
      QDateTime           los;        //!< Момент времени, когда сигнал будет потерян
      double              maxEl;      //!< Момент кульминации (максимальный угол места)
      unsigned long       revol;      //!< Номер витка на момент времени aos
      conf::SatDirection  direction;  //!< Направление спутника (-1 - неизвестно,0 - C->Ю, 1 - Ю->С)
      float               freq;       //!< Частота приёма данных со спутника
      conf::ConflState    state;      //!< Состояние конфликта (первоначальное, конфликт разрешен автоматически)

      Schedule() {}
      Schedule(const QString& n, const QDateTime& dt_aos, const QDateTime& dt_los,
         double max, unsigned long rev, conf::SatDirection dir, float fr = 0.0, conf::ConflState st = conf::kUnkState ):
        name(n),
        aos(dt_aos),
        los(dt_los),
        maxEl(max),
        revol(rev),
        direction(dir),
        freq(fr),
        state(st)
      {}

      bool operator <(const satellite::Schedule& s) const {
        return (aos.secsTo(s.aos) > 0);
      }
      bool operator ==(const satellite::Schedule& s) const {
        return name == s.name && revol == s.revol;
      }
      bool operator !=(const satellite::Schedule& s) const {
        return name != s.name || revol != s.revol;
      }
    };
  }


  namespace schedule {
    //! создание расписания пролёта спутников
    class SatSchedule {

    public:
      SatSchedule() {}
      SatSchedule( const QString& file, const Coords::GeoCoord& );
      ~SatSchedule();

      void setFile( const QString& file );
      void setSite( const Coords::GeoCoord& );

      bool getSchedule( const QDateTime& dt, const QStringList& satts, float hours, float threshold, 
                        float culmin, QList<satellite::Schedule>& sched ) const;
      bool getSchedule( const QDateTime& dt_dt, const QString& tleName,
                        float hours, float threshold, float culmin,
                        QList<satellite::Schedule>& sched ) const;


      void settlementConflicts(QList<satellite::Schedule>& sched, mappi::conf::PriorType type, bool saveUserConflict = false);
    private:
      int direction(const Satellite& sat, double aos) const;
      int cmpConflict(satellite::Schedule& sat1, satellite::Schedule& sat2, mappi::conf::PriorType type) const;
      int timeWest(satellite::Schedule& sat, int step) const;
  
    private:
      QString _file;
      Coords::GeoCoord _site;
    };

  }
}

#endif 
