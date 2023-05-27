/*!
 * \class SpSchedule::ScheduleInfo
 * \brief Информация на основе которой создается расписание.
 *
 * \author Бурыкин Е.А.
 * \date 10.06.2010
 */
#include "scheduleinfo.h"

#include <cross-commons/debug/tlog.h>

using namespace mappi;
using namespace schedule;


ScheduleInfo::ScheduleInfo() {
    m_start = QDateTime::currentDateTimeUtc();
    m_stop = QDateTime::currentDateTimeUtc();
    m_minEl = 89.9;
    m_maxEl = 89.9;
    m_settlType = mappi::conf::kUnkPrior;
    //    m_satList.setAutoDelete( false );
}

ScheduleInfo::~ScheduleInfo() {
}

float ScheduleInfo::diffTimeRange() const {
    return m_start.secsTo( m_stop ) / 3600.0;
}

void ScheduleInfo::print() const {
  trc;
  var(m_pathTLE);
  debug_log << "interval" << m_start.toString( Qt::ISODate ) << m_stop.toString( Qt::ISODate );
  debug_log << "station" << m_coordStation.lat << m_coordStation.lon << m_coordStation.alt;
  QString sat;
  //for ( Settings::SatParamBase* psat = m_satList.first(); psat; psat = m_satList.next() ) {
  for (int idx = 0; idx < m_satNamesList.size(); idx++) {
    sat += QString( "%1 " ).arg(m_satNamesList.at(idx));
  }
  var(sat);
  var(m_minEl);
  var(m_maxEl);
  var((int)m_settlType);
}

// void ScheduleInfo::setSatList( const QList<Settings::SatParamBase>& satlist ) {
//   m_satList.clear();
//   m_satList = satlist;
// }
