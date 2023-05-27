#ifndef MAPPI_SCHEDULE_SHEDULEHELPER_H
#define MAPPI_SCHEDULE_SHEDULEHELPER_H

#include "schedule.h"

#include <sat-commons/satellite/satellitebase.h>

#include <meteo/commons/global/global.h>

namespace mappi {
namespace schedule {

class ScheduleHelper
{
public:
  ScheduleHelper(const ScheduleHelper&) = delete;
  ScheduleHelper& operator=(const ScheduleHelper&) = delete;

  static ScheduleHelper& instance() {
    static ScheduleHelper instance_;
    return instance_;
  }

public:
  /*! \brief getSchedule Получить расписание */
  static Schedule getSchedule()
  {
    Schedule schedule;
    auto* channel = meteo::global::serviceChannel(meteo::settings::proto::kSchedule);
    if(nullptr != channel)
    {
      conf::ScheduleReply* response =
	channel->remoteCall(&mappi::conf::ScheduleService::GetSchedule, conf::IdleRequest(), 10000, true);
      if(nullptr != response)
      {
        if(response->has_result() && response->result())
        {
          for(auto& it : response->session()) {
            schedule.appendSession(Session(it));
          }
        }
        else if(response->has_comment()) {
          error_log << QString::fromStdString(response->comment());
        }
        delete response;
      }
      delete channel;
    }
    return schedule;
  }

  /*!
   * \brief changeState Решить конфликт сеанса
   * \param session - сеанс для изменения
   */
  static bool changeState(const schedule::Session& session)
  {
    bool ok = false;
    auto* channel = meteo::global::serviceChannel(meteo::settings::proto::kSchedule);
    if(nullptr != channel)
    {
      conf::SessionRequest request;
      request.set_satellite(session.getNameSat().toStdString());
      request.set_revol(session.getRevol());
      request.set_state(session.getState());
      conf::ScheduleReply* response =
          channel->remoteCall(&mappi::conf::ScheduleService::EditSession, request, 10000, true);
      if(nullptr != response)
      {
        if(response->has_result()) {
          ok = response->result();
        }
        if(!ok && response->has_comment()) {
          error_log << QString::fromStdString(response->comment());
        }
        delete response;
      }
      delete channel;
    }
    return ok;
  }

  /*! \brief createSchedule - Пересоздать расписание, удалив пользовательские решения конфликтов */
  static bool createSchedule()
  {
    bool ok = false;
    auto* channel = meteo::global::serviceChannel(meteo::settings::proto::kSchedule);
    if(nullptr != channel)
    {
      conf::ScheduleReply* response =
          channel->remoteCall(&mappi::conf::ScheduleService::Recreate, conf::IdleRequest(), 10000, true);
      if(nullptr != response)
      {
        if(response->has_result()) {
          ok = response->result();
        }
        if(!ok && response->has_comment()) {
          error_log << QString::fromStdString(response->comment());
        }
        delete response;
      }
      delete channel;
    }
    return ok;
  }

  /*! \brief refreshSchedule Обновить расписание, сохранив пользовательские решения конфликтов */
  static bool refreshSchedule()
  {
    bool ok = false;
    auto* channel = meteo::global::serviceChannel(meteo::settings::proto::kSchedule);
    if(nullptr != channel)
    {
      conf::ScheduleReply* response =
          channel->remoteCall(&mappi::conf::ScheduleService::Refresh, conf::IdleRequest(), 10000, true);
      if(nullptr != response)
      {
        if(response->has_result()) {
          ok = response->result();
        }
        if(!ok && response->has_comment()) {
          error_log << QString::fromStdString(response->comment());
        }
        delete response;
      }
      delete channel;
    }
    return ok;
  }

  /*!
   * \brief getSTle Запросить TLE параметры у сервиса расписаний
   * \param satName - Название спутника
   * \param aos     - Время для поиска TLE файла
   * \return { satName, firstString, secondString }
   */
  static MnSat::STLEParams getSTle(const QString& satName, const QDateTime& aos)
  {
    MnSat::STLEParams stle;
    auto* channel = meteo::global::serviceChannel(meteo::settings::proto::kSchedule);
    if(nullptr != channel)
    {
      conf::TleRequest request;
      request.set_name(satName.toStdString());
      request.set_aos(aos.toString(Qt::ISODate).toStdString());
      conf::STleReply* response =
          channel->remoteCall(&mappi::conf::ScheduleService::GetSTle, request, 10000, true);
      if(nullptr != response)
      {
        if(response->has_result() && response->result()) {
          stle.satName      = QString::fromStdString(response->name());
          stle.firstString  = QString::fromStdString(response->firststring());
          stle.secondString = QString::fromStdString(response->secondstring());
        }
        delete response;
      }
      delete channel;
    }
    return stle;
  }

  /*! \brief getAllSatellites Получить все спутники из weather.txt */
  static QStringList getAllSatellites()
  {
    QStringList satellites;
    auto* channel = meteo::global::serviceChannel(meteo::settings::proto::kSchedule);
    if(nullptr != channel)
    {
      conf::AllSatellitesReply* response =
          channel->remoteCall(&mappi::conf::ScheduleService::GetAllSatellites, conf::IdleRequest(), 10000, true);
      if(nullptr != response)
      {
        for(auto satname : response->name()) {
          satellites.append(QString::fromStdString(satname));
        }
        delete response;
      }
      delete channel;
    }
    return satellites;
  }

private:
  ScheduleHelper() {}
  ~ScheduleHelper() {}
};

} // schedule
} // mappi

#endif // MAPPI_SCHEDULE_SHEDULEHELPER_H
