/*!
 * \class SpSchedule::Scheduler
 * \brief Планировщик расписания.
 *
 * Необходимая информация для получения расписания:
 * - координаты пункта приема;
 * - список спутников;
 * - интервал времени в часах (на какой срок составлять расписание);
 * - автоматический способ решения конфликтов;
 * - наличие TLE-телеграммы.
 *
 * \author Бурыкин Е.А.
 * \date 08.06.2010
 */
#include "scheduler.h"
#include <qfile.h>

//#define _DEBUG_
#ifdef _DEBUG_
#include <stdio.h>
#endif // _DEBUG_

#include <commons/textproto/tprototext.h>
#include <commons/textproto/pbtools.h>
#include <mappi/proto/reception.pb.h>
#include <mappi/global/global.h>

#define RECEPTION_FILE MnCommon::etcPath("mappi") + "reception.conf"

using namespace mappi;
using namespace schedule;


Scheduler::Scheduler(mappi::conf::RecvMode mode /*= mappi::conf::kRateUnk*/) 
{
  readSettings(mode);
}

Scheduler::~Scheduler() {
}

/*!
 * \brief Проверка существования TLE телеграммы.
 * \return true если существует.
 */
bool Scheduler::existTLE() const {
    QFile file( m_schedInfo.getPathTLE() );

#ifdef _DEBUG_
    printf( "[ Scheduler::existTLE() ] - status=[%i];\n", file.exists() );
#endif
    return file.exists();
}

/*!
 * \brief Чтение параметров для составления расписания.
 * \return true если параметры считаны успешно.
 */
bool Scheduler::readSettings(mappi::conf::RecvMode mode /*= mappi::conf::kRateUnk*/) 
{
  Q_UNUSED(mode);

  mappi::conf::Reception conf;
  QFile file(RECEPTION_FILE);
  if ( !file.open(QIODevice::ReadOnly) ) {
    error_log << QObject::tr("Ошибка при загрузке параметров приёма '%1'").arg(RECEPTION_FILE);
    return false;
  }

  QString text = QString::fromUtf8(file.readAll());
  file.close();

  if ( !TProtoText::fillProto(text, &conf) ) {
    error_log << QObject::tr("Ошибка в структуре файла параметров кадра '%1'").arg(RECEPTION_FILE);
    return false;
  }

  Coords::GeoCoord coord(conf.site().point().lat_radian(), 
			 conf.site().point().lon_radian(), 
			 conf.site().point().height_meters());
  m_schedInfo.setCoordStation(coord);

  QStringList satNames;
  for (int idx = 0; idx < conf.sched().sats_size(); idx++) {
    satNames << QString::fromStdString(conf.sched().sats(idx));
  }
  QDateTime dt = QDateTime::currentDateTimeUtc();
  //m_schedInfo.setTLE(pbtools::toQString(conf.tlefile()));
  m_schedInfo.setTLE(meteo::global::findWeatherFile(pbtools::toQString(conf.tledir()), dt));
  m_schedInfo.setSatNamesList(satNames);
  m_schedInfo.setMinEl(conf.sched().elmin());
  m_schedInfo.setMaxEl(conf.sched().elmax() );
  m_schedInfo.setRuleConflResolv(conf.sched().priority());
  m_schedInfo.setHours(conf.sched().period() );
  return true;

 //    bool is_OK = true;
//     SpSettings settings;
//     QList< Settings::SatParamBase > satParam;
//     if ( settings.getSettings( satParam, Settings::ORBITAL_SAT, mode ) == 0 ) {
//       satParam.setAutoDelete(false);
//       m_schedInfo.setSatList( satParam );

// //        QStringList satNamesList;
// //        int satCount = satParam.count();
// //        for ( int i = 0; i < satCount; ++i )
// //            satNamesList.append( satParam.at( i ) -> tleName );
// //        m_schedInfo.setSatNamesList( satNamesList );
//     } else
//         is_OK = false;

//     if ( is_OK ) {

//         Settings::SiteSettings station;
//         if ( settings.getSettings( &station ) == 0 ) {
//             m_schedInfo.setCoordStation( station.coord );
//         } else
//             is_OK = false;
//     };

//     if ( is_OK ) {

//         Settings::RecvParam recvParam;
//         if ( settings.getSettings( recvParam ) == 0 ) {

//             m_schedInfo.setMinEl( recvParam.startHrptEl );
// //            m_schedInfo.setMinEl( recvParam.startEl );
//             m_schedInfo.setMaxEl( recvParam.maxEl );
//             m_schedInfo.setRuleConflResolv( recvParam.type );
//         } else
//             is_OK = false;
//     };

// #ifdef _DEBUG_
//     printf( "[ Scheduler::readSettings() ] - status=[%i];\n", is_OK );
//     //m_schedInfo.print();
// #endif
//     return is_OK;
}

/*!
 * \brief Создать расписание относительно текущего времени (UTC).
 *
 * \param sched - расписание;
 * \param hours - продолжительность расписания в часах.
 * \return true если расписание создано успешно.
 */
bool Scheduler::create(Schedule& sched) {
  QDateTime curDateTime = QDateTime::currentDateTimeUtc();
  return create( sched, curDateTime.addSecs(-24*3600), curDateTime.addSecs( ( int )( m_schedInfo.getHours() * 3600 ) ) );
}

/*!
 * \brief Создать расписание для заданного временного интервала (UTC).
 *
 * \param sched - расписание;
 * \param start - начало временного интервала;
 * \param end - конец временного интервала;
 * \return true если расписание создано успешно.
 */
bool Scheduler::create( Schedule& sched, const QDateTime& start, const QDateTime& end ) {
    m_schedInfo.setStart( start );
    m_schedInfo.setStop( end );
    bool is_OK = sched.create( m_schedInfo );

#ifdef _DEBUG_
    printf( "[ Scheduler::create(3) ] - status=[%i];\n", is_OK );
#endif
    return is_OK;
}
/*!
 * \brief Scheduler::refresh - обновить расписание сохранив пользовательское решение конфликтов
 * \return true если расписание создано успешно
 */
bool Scheduler::refresh( Schedule& sched) {
  QDateTime curDateTime = QDateTime::currentDateTimeUtc();
  return refresh(sched, curDateTime.addSecs(-24 * 3600), curDateTime.addSecs(static_cast<int>(m_schedInfo.getHours()) * 3600));
}

/*!
 * \brief Scheduler::refresh - обновить расписание сохранив пользовательское решение конфликтов
 * \param sched - расписание
 * \param start - начало временного интервала
 * \param end   - конец временного интервала
 * \return true если расписание создано успешно
 */
bool Scheduler::refresh( Schedule& sched, const QDateTime& start, const QDateTime& end) {
  m_schedInfo.setStart( start );
  m_schedInfo.setStop( end );
  return sched.refresh( m_schedInfo );
}

/*!
 * \brief Решить конфликты, способ решения задается из настроек.
 * \param sched         - расписание.
 * \param saveUserConfl - сохранить пользовательское решение конфликтов
 */
void Scheduler::resolveConfl( Schedule& sched, bool saveUserConfl ) const {
    sched.resolveConfl( m_schedInfo.getRuleConflResolv(), saveUserConfl );
}

/*!
 * \brief Сохранитиь расписание.
 *
 * \param sched - расписание;
 * \param path - путь к файлу.
 * \return true если расписание сохранено.
 */
bool Scheduler::save(Schedule& sched, const QString& path)
{
  QFile schedFile(path);
  bool is_OK = schedFile.open(QIODevice::WriteOnly);
  if ( !is_OK ) {
    error_log << QObject::tr("Ошибка создания/открытия файла %1").arg(path);
    return false;
  }
  
  unsigned int count = sched.countSession();

  for ( unsigned int number = 0; number < count; ++number ) {
    QByteArray line;
    line.append(sched.getSession( number ).toLine() + "\n");
    if ( schedFile.write( line ) <= 0 ) {
      is_OK = false;
      break;
    }
  }

  schedFile.close();
  
#ifdef _DEBUG_
  printf( "[ Scheduler::save(2) ] - path=[%s] status=[%i];\n", (const char*)path, is_OK );
#endif
  return is_OK;
}

/*!
 * \brief Восстановить расписание из файла.
 *
 * \param sched - расписание;
 * \param path - путь к файлу.
 * \return если расписание восстановлено.
 */
bool Scheduler::restore( Schedule& sched, const QString& path ) 
{
  QFile schedFile(path);
  bool is_OK = schedFile.open(QIODevice::ReadOnly);
  if ( !is_OK ) {
    error_log << QObject::tr("Ошибка открытия файла %1").arg(path);
    return false;
  }

  sched.clear();

  QTextStream stream( &schedFile );
  while ( !stream.atEnd() ) {
    QString line = stream.readLine();
    Session session;
    if ( session.toSession( line ) != true ) {
      is_OK = false;
      break;
    }
    sched.appendSession( session );
  }

  schedFile.close();

#ifdef _DEBUG_
    printf( "[ Scheduler::restore(2) ] - path=[%s] status=[%i];\n", (const char*)path, is_OK );
#endif
    return is_OK;
}
