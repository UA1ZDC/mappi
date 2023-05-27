#ifndef MAPPI_SCHEDULE_SCHEDULEINFO_H
#define MAPPI_SCHEDULE_SCHEDULEINFO_H

#include <commons/geobasis/coords.h>

#include <qstring.h>
#include <qstringlist.h>
#include <qlist.h>
#include <qdatetime.h>

#include <mappi/proto/reception.pb.h>

namespace mappi {
  namespace schedule {
    
    
    class ScheduleInfo {
    
      QString m_pathTLE;                   //!< путь к телеграмме
      QDateTime m_start;                   //!< начало временного интервала
      QDateTime m_stop;                    //!< конец временного интервала
      Coords::GeoCoord m_coordStation;     //!< координаты пункта приема
      QStringList m_satNamesList;          //!< имена спутников
      //QList<Settings::SatParamBase> m_satList;   //!< имена спутников
      float m_minEl;                       //!< минимальный угол места
      float m_maxEl;                       //!< максимальный угол места
      ::mappi::conf::PriorType m_settlType;  //!< способ решения конфликтов
      float m_hours;		//!< Период на который составлено расписание

    public :

      ScheduleInfo();
      ~ScheduleInfo();

      float getHours() const { return m_hours; }
      //! \brief Получить путь к телеграмме
      QString getPathTLE() const { return m_pathTLE; }
      //! \brief Получить значение начала временного интервала
      QDateTime getStart() const { return m_start; }
      //! \brief Получить значение конца временного интервала
      QDateTime getStop() const { return m_stop; }
      //! \brief Получить координаты пункта приема
      Coords::GeoCoord getCoordStation() const { return m_coordStation; }
      //! \brief Получить список спутников
      QStringList getSatNamesList() const { return m_satNamesList; }
      //    inline QList<Settings::SatParamBase> getSatList() const { return m_satList; };
      //! \brief Получить значение минимального угла места
      float getMinEl() const { return m_minEl; }
      //! \brief Получить значение угла места в кульминации
      float getMaxEl() const { return m_maxEl; }
      //! \brief Получить правило решения конфликтов
      inline ::mappi::conf::PriorType getRuleConflResolv() const { return m_settlType; }

      void setTLE(const QString& tle) { m_pathTLE = tle; }
      //! \brief Установить значение начала временного интервала
      void setStart( const QDateTime& value ) { m_start = value; }
      //! \brief Установить значение конца временного интервала
      inline void setStop( const QDateTime& value ) { m_stop = value; }
      //! \brief Установить координаты пункта приема
      inline void setCoordStation( const Coords::GeoCoord& value ) { m_coordStation = value; }
      //! \brief Установить список спутников
      inline void setSatNamesList( const QStringList& value ) { m_satNamesList = value; }
      //    void setSatList( const QList<Settings::SatParamBase>& list );
      //! \brief Установить значение минимального угла места
      inline void setMinEl( const float& value ) { m_minEl = value; }
      //! \brief Установить значение угла места в кульминации
      inline void setMaxEl( const float& value ) { m_maxEl = value; }
      inline void setHours( float value ) { m_hours = value; }
      //! \brief Установить правило решения конфликтов
      inline void setRuleConflResolv( ::mappi::conf::PriorType value ) { m_settlType = value; }

      //! \brief Разница между концом и началом временного интервала расписания в часах.
      float diffTimeRange() const;

      //! \brief Печать информации о расписании
      void print() const;

    };


  }
}

#endif
