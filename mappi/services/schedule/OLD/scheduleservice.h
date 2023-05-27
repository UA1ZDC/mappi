#ifndef MAPPI_SCHEDULE_SERVICE_SCHEDULESERVICE_H
#define MAPPI_SCHEDULE_SERVICE_SCHEDULESERVICE_H

#include <qobject.h>
#include <qmap.h>
#include <qset.h>
#include <qmutex.h>

#include <meteo/commons/global/requestcard.h>
#include <meteo/commons/rpc/rpc.h>

#include <mappi/proto/schedule.pb.h>

class QFileSystemWatcher;
class QTimer;


namespace mappi {
namespace schedule {

class Schedule;

class ScheduleService : public QObject, public conf::ScheduleService
{
  Q_OBJECT
  using ScheduleMultiCard  = RequestCard<const mappi::conf::ScheduleSubscribeRequest, mappi::conf::ScheduleReply>;
public:
  ScheduleService();
  ~ScheduleService();

  void Subscribe(google::protobuf::RpcController *controller
                       , const conf::ScheduleSubscribeRequest* request
                       , conf::ScheduleReply *response
                       , google::protobuf::Closure *done);

  void GetSchedule(::google::protobuf::RpcController* ctrl
                  , const conf::IdleRequest* req
                  , conf::ScheduleReply* resp
                  , ::google::protobuf::Closure* done);

  void EditSession(::google::protobuf::RpcController* ctrl
                  , const conf::SessionRequest* req
                  , conf::ScheduleReply* resp
                  , ::google::protobuf::Closure* done);

  void Recreate(::google::protobuf::RpcController* ctrl
               , const conf::IdleRequest* req
               , conf::ScheduleReply* resp
               , ::google::protobuf::Closure* done);

  void Refresh(google::protobuf::RpcController *ctrl
             , const mappi::conf::IdleRequest *request
             , mappi::conf::ScheduleReply *response
             , google::protobuf::Closure *done);

  void GetSTle(google::protobuf::RpcController *ctrl
             , const mappi::conf::TleRequest *request
             , mappi::conf::STleReply* response
             , google::protobuf::Closure *done);

  void GetAllSatellites(google::protobuf::RpcController *ctrl
                      , const mappi::conf::IdleRequest *reauest
                      , mappi::conf::AllSatellitesReply * response
                      , google::protobuf::Closure *done);
signals:
  void scheduleChanged();

public slots:
  // void clientSubscribed(meteo::rpc::Controller*);
  void clientUnsubscribed(meteo::rpc::Controller*);    
			
private slots:
  int slotRefreshSchedule();
  void slotChangedTleFile();



private:
  int createSchedule();

  void sendSchedule(const schedule::Schedule& sched);

private:
  QFileSystemWatcher* watcher_   = nullptr;
  QTimer*             checkTimer = nullptr;
  QSet<QString>       files_;

  QMap<meteo::rpc::Channel*, google::protobuf::Closure*> subs_;         //TODO как будто лишнее
  QMap<meteo::rpc::Channel*, ScheduleMultiCard> scheduleMultiCards_;

  QMutex mutex_;
};

} // schedule
} // mappi

#endif // MAPPI_SCHEDULE_SERVICE_SCHEDULESERVICE_H
