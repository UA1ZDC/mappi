/*!
 * \class SpSchedule::Schedule
 * \brief Расписание сеансов
 *
 * Время в расписание только UTC.
 *
 * \author Бурыкин Е.А.
 * \date 09.06.2010
 */
#include "schedule.h"
#include "sat_schedule.h"
#include <commons/mathtools/mnmath.h>
#include <mappi/proto/reception.pb.h>

#include <cross-commons/debug/tlog.h>

#include <time.h>


// вывод отладочной информации
//#define _DEBUG_
#ifdef _DEBUG_
#include <stdio.h>
#endif

using namespace mappi;
using namespace schedule;

Schedule::Schedule() {
}

Schedule::Schedule( const Schedule& object ) {
    m_list.clear();
    m_list = object.m_list;
}

Schedule& Schedule::operator=( const Schedule& object ) {
    m_list.clear();
    m_list = object.m_list;
    return *this;
}

Schedule::~Schedule() { }

/*!
 * \brief Создать расписание.
 * \param schedInfo - информация о расписании.
 * \return true если расписание создано.
 */
bool Schedule::create( const ScheduleInfo& schedInfo ) {
    m_list.clear();
    m_satSchedule.setFile( schedInfo.getPathTLE() );
    m_satSchedule.setSite( schedInfo.getCoordStation() );
    //    QList<Settings::SatParamBase> satlist = schedInfo.getSatList();
    bool is_OK = m_satSchedule.getSchedule( schedInfo.getStart(),
					    schedInfo.getSatNamesList(),
					    //  satlist,
        schedInfo.diffTimeRange(),
        MnMath::deg2rad( schedInfo.getMinEl() ),
        MnMath::deg2rad( schedInfo.getMaxEl() ),
        m_list
    );

    if ( is_OK )
        qSort( m_list );

#ifdef _DEBUG_
    printf( "[ Schedule::create(1) ] - status=[%i];\n", is_OK );
#endif
    return is_OK;
}

/*!
 * \brief Schedule::refresh Обновить расписание учитывае пользовательское разрешение конфликтов
 * \param schedInfo - информация о расписании.
 * \return true если расписание обновлено.
 */
bool Schedule::refresh( const ScheduleInfo& schedInfo ) {
  QList<satellite::Schedule> tmpSched;
  m_satSchedule.setFile( schedInfo.getPathTLE() );
  m_satSchedule.setSite( schedInfo.getCoordStation() );
  bool ok = m_satSchedule.getSchedule(schedInfo.getStart(), schedInfo.getSatNamesList(),
                                      schedInfo.diffTimeRange(),
                                      meteo::DEG2RAD * schedInfo.getMinEl(),
                                      meteo::DEG2RAD * schedInfo.getMaxEl(),
                                      tmpSched);
  if(ok) {
    qSort(tmpSched);
    if(false == m_list.isEmpty())
    {
      for(auto& tmpSession : tmpSched)
      {
        for(const auto& session : m_list)
        {
          if(tmpSession == session) {
            tmpSession.state = session.state;
            break;
          }
        }
      }
    }
    m_list.swap(tmpSched);
  }
  return ok;
}

/*!
 * \brief Решить конфликты между сеансами.
 * \param settlType     - способ решения конфликтов.
 * \param saveUserConfl - сохранить пользовательское решение конфликтов
 */
void Schedule::resolveConfl( mappi::conf::PriorType settlType, bool saveUserConfl ) {
    m_satSchedule.settlementConflicts( m_list, settlType, saveUserConfl );
}

/*! \brief Печать расписания в консоль. */
void Schedule::print() const {
  for(const auto& it : m_list)
  {
    debug_log << "[ Schedule::print() ] - count=" << m_list.count();
    const Session& session = it;
    debug_log << session.toLine();
  }
}

/*! \brief Очистить расписание. */
void Schedule::clear() {
    m_list.clear();
}

bool Schedule::isEmpty() const {
  return m_list.isEmpty();
}

/*! \brief Только Ожидаемые сеансы. */
Schedule Schedule::acceptedSessions() const {
    Schedule sched;
    for(const auto& it : m_list) {
      const Session& session = it;
      if(session.adopted() == true) {
        sched.appendSession(session);
      }
    }
    return sched;
}

/*!
 * \brief Schedule::countCompletedSessions Количество прошедших сеансов
 * \param isReceived - Только сеансы со статусом "Принимаемые"
 * \return количество прошедших сеансов
 */
int Schedule::countCompletedSessions(bool isReceived) const
{
  int count = 0;
  for(const auto& it : m_list) {
    if(it.los <= QDateTime::currentDateTimeUtc()) {
      if(isReceived) {
        if(it.state == conf::kNormalState ||
           it.state == conf::kSettlState ||
           it.state == conf::kUserEnableState)
        {
          count++;
        }
      }
      else {
        count++;
      }
    }
  }
  return count;
}

/*!
 * \brief Текущее количество сеансов в расписании.
 * \return количество сеансов.
 */
unsigned int Schedule::countSession() const {
    return m_list.count();
}

/*!
 * \brief По заданному сеансу получить его порядковый номер.
 * \param session - сеанс.
 * \return порядковый номер сеанса в расписании и (-1) если сеанс в расписании не найден.
 */
int Schedule::getNumberSession( Session session ) const {
  int result = -1;
  for(const auto& it : m_list) {
    result++;
    if(session == it){
      break;
    }
  }
  return result;
}

/*!
 * \brief Первый сеанс в расписании.
 * \warning Если в расписании нет сеансов, то возвращается пустой сеанс.
 * \return сеанс.
 * \sa lastSession()
 */
Session Schedule::firstSession() const {
    return ( 0 < m_list.isEmpty() ? defaultSession() : m_list.first());
}

/*!
 * \brief Последний сеанс в расписании.
 * \warning Если в расписании нет сеансов, то возвращается пустой сеанс.
 * \return сеанс.
 * \sa firstSession()
 */
Session Schedule::lastSession() const {
  return (m_list.isEmpty() ? defaultSession() : m_list.last());
}

/*!
 * \brief Первый принимаемый сеанс, относительно текущего времени (даже если он уже принимается).
 * \param withCurrent true - вместе с принимаемым, false - не учитывая принимаемый
 * \warning Если в расписании нет сеансов, то возвращается пустой сеанс.
 * \return сеанс.
 * \sa nextSession().
 */
Session Schedule::nearSession(bool withCurrent /* = true*/) const {
    unsigned int count = countSession();
    for( unsigned int number = 0; number < count; ++number ) {

        const Session& session = getSession( number );
        if ( session.adopted() == true ) {

            Session::stage_t stage = session.currentStage();
            if ( (stage == Session::ALREADY_TAKEN && withCurrent) || (stage == Session::EXPECTED) )
               return session;
        };
    };
    return defaultSession();
}


/*!
 * \brief Первый принимаемый сеанс, относительно времени dt
 * \param withCurrent true - вместе с принимаемым, false - не учитывая принимаемый
 * \warning Если в расписании нет сеансов, то возвращается пустой сеанс.
 * \return сеанс.
 * \sa nextSession().
 */
Session Schedule::nearSession(const QDateTime& dt, bool withCurrent /*= true*/) const
{
  unsigned int count = countSession();
  for( unsigned int number = 0; number < count; ++number ) {
    
    const Session& session = getSession( number );
    if ( session.adopted() == true ) {
      
      Session::stage_t stage = session.dtStage(dt);
      if ( (stage == Session::ALREADY_TAKEN && withCurrent) || (stage == Session::EXPECTED) )
	return session;
    };
  };
  return defaultSession();
}
  
/*!
 * \brief Следующий принимаемый сеанс.
 *
 * \warning Если в расписании нет сеансов, то возвращается пустой сеанс.
 * \return сеанс.
 * \sa nearSession().
 */
Session Schedule::nextSession() const {
    unsigned int count = countSession();
    unsigned int start = getNumberSession( nearSession() ) + 1;
    for( unsigned int number = start; number < count; ++number ) {

        const Session& session = getSession( number );
        if ( session.adopted() == true ) {
            if ( session.currentStage() == Session::EXPECTED )
               return session;
        };
    };
    return defaultSession();
}

/*!
 * \brief Получить сеанс по его порядковому номеру в расписании.
 * \warning Если произойдет выход за границы расписания, то возвращается пустой сеанс.
 * \return сеанс.
 */
Session& Schedule::getSession( unsigned int i ) {
  return ( (int)i < m_list.count() ? static_cast< Session& >( m_list[ i ] ) : defaultSession() );
}

/*! 
 * \brief Найти элемент расписания по номеру витка и имени спутника
 * \param name Имя спутника
 * \param revolve Номер витка
 * \return Найденный элемент в случае успеха, иначе - пустая сессия
 */
Session& Schedule::getSession( const QString& name, const unsigned revolve ) {
  for(auto& it : m_list) {
    if(revolve == it.revol && name == it.name){
      return static_cast<Session&>(it);
    }
  }
  return defaultSession();
}

/*!
 * \brief Получить сеанс по его порядковому номеру в расписании, для константных объектов.
 * \warning Если произойдет выход за границы расписания, то возвращается пустой сеанс.
 * \return сеанс.
 */
const Session& Schedule::getSession( unsigned int i ) const {
  return ( (int)i < m_list.count() ? static_cast< const Session& >( m_list[ i ] ) : defaultSession() );
}

/*! \brief Пустой сеанс. */
Session& Schedule::defaultSession() {
    static Session session;
    session.defaults();
    return session;
}

/*! \brief Добавить сеанс в конец расписания. */
void Schedule::appendSession( const Session& session ) {
    m_list.append( session );
}

/*! \brief Добавить сеанс в начало расписания. */
void Schedule::prependSession( const Session& session ) {
    m_list.prepend( session );
}

/*! \brief Schedule::setSessionState Установить новое разрешение конфликта для сеанса */
bool Schedule::setSessionState( const Session& session) {
  for(auto& item : m_list) {
    if(item == session) {
      item.state = session.state;
      return true;
    }
  }
  return false;
}

/*! \brief Начало временного интервала расписания. */
QDateTime Schedule::start() const {
    return firstSession().getAos();
}

/*! \brief Конец временного интервала расписания. */
QDateTime Schedule::end() const {
    return lastSession().getLos();
}
