#ifndef METEO_COMMONS_SERVICES_SPRINF_SPRINFSERVICE_H
#define METEO_COMMONS_SERVICES_SPRINF_SPRINFSERVICE_H

#include <QFuture>
#include <QFutureWatcher>

#include <meteo/commons/global/appstatusthread.h>
#include <meteo/commons/proto/sprinf.pb.h>

namespace meteo {
namespace sprinf {

enum StatusParam {
  kRequestsCount,
  kSuccessResponseCount,
  kErorrResponseCount,
};

class ClientHandler;

class TSprinfService :public QObject, public sprinf::SprinfService
{
    Q_OBJECT

  public:
    TSprinfService(AppStatusThread*);
    ~TSprinfService();

    void GetStationTypes(google::protobuf::RpcController* controller,
                         const TypesRequest* request,
                         StationTypes* response,
                         google::protobuf::Closure* done);
    void GetGmiTypes(google::protobuf::RpcController* controller,
                     const TypesRequest* request,
                     GmiTypes* response,
                     google::protobuf::Closure* done);

    void GetLevelTypes(google::protobuf::RpcController* controller,
                       const TypesRequest* request,
                       LevelTypes* response,
                       google::protobuf::Closure* done);
    void GetStations(google::protobuf::RpcController* controller,
                     const MultiStatementRequest* request,
                     Stations* response,
                     google::protobuf::Closure* done);

    void GetRegions(google::protobuf::RpcController* controller,
                     const MultiStatementRequest* request,
                     Regions* response,
                     google::protobuf::Closure* done);

    void GetMilitaryDistrict(google::protobuf::RpcController* controller,
                     const StatementRequest* request,
                     Regions* response,
                     google::protobuf::Closure* done);

    // Метод получения групп регионов
    void GetRegionGroups( google::protobuf::RpcController* controller,
                      const RegionGroupsRequest* request,
                      RegionGroupsReply* response,
                      google::protobuf::Closure* done);
    // Метод создания/редактирования групп регионов
    void SaveRegionGroups( google::protobuf::RpcController* controller,
                      const RegionGroupsRequest* request,
                      RegionGroupsReply* response,
                      google::protobuf::Closure* done);
    // Метод удаления групп регионов
    void DeleteRegionGroups( google::protobuf::RpcController* controller,
                      const RegionGroupsRequest* request,
                      RegionGroupsReply* response,
                      google::protobuf::Closure* done);

    void GetStationsByPosition(google::protobuf::RpcController* controller,
                               const CircleRegionRequest* request,
                               StationsWithDistance* response,
                               google::protobuf::Closure* done);

    void GetMeteoCenters(google::protobuf::RpcController* controller,
                         const MeteoCenterRequest* request,
                         MeteoCenters* response,
                         google::protobuf::Closure* done);

    void GetMeteoParametersByBufr(google::protobuf::RpcController* controller,
                                  const MeteoParameterRequest* request,
                                  MeteoParameters* response,
                                  google::protobuf::Closure* done);

    void GetBufrParametersTables(google::protobuf::RpcController* controller,
                                 const BufrParametersTableRequest* request,
                                 BufrParametersTables* response,
                                 google::protobuf::Closure* done);

    void GetCountry(::google::protobuf::RpcController* controller,
                    const ::meteo::sprinf::CountryRequest* request,
                    ::meteo::sprinf::Countries* response,
                    ::google::protobuf::Closure* done);
    void UpdateStation(::google::protobuf::RpcController* controller,
                       const ::meteo::sprinf::Station* request,
                       ::meteo::sprinf::ReportStationsAdded* response,
                       ::google::protobuf::Closure* done);
    void DeleteStation(::google::protobuf::RpcController* controller,
                       const ::meteo::sprinf::Station* request,
                       ::meteo::sprinf::ReportStationsAdded* response,
                       ::google::protobuf::Closure* done);

    void GetTlgStatistic(::google::protobuf::RpcController* controller,
                       const ::meteo::sprinf::TlgStatisticRequest* request,
                       ::meteo::sprinf::TlgStatisticResponse* response,
                       ::google::protobuf::Closure* done);
  
    void GetCities(::google::protobuf::RpcController* controller,
                       const ::meteo::sprinf::Dummy* request,
                       ::meteo::map::proto::Cities* response,
                       ::google::protobuf::Closure* done);
  
    void GetCityDisplayConfig(::google::protobuf::RpcController* controller,
                       const ::meteo::sprinf::Dummy* request,
                       ::meteo::map::proto::CityDisplayConfig* response,
                       ::google::protobuf::Closure* done);


    ClientHandler* handler() const { return handler_; }

    void setUnlockEditDb() { editMutex_.lock(); lockEditDb_ = false; editMutex_.unlock(); }
  public slots:
    void slotAnsverSendedError();
    void slotAnsverSendedOK();
    void slotRequestReceive();
  signals:
    void signalRequestReceive();
  private:
    ClientHandler* handler_;
    QMutex editMutex_;
    bool lockEditDb_;
    AppStatusThread* status_;
};
} // sprinf
} // meteo

#endif // METEO_COMMONS_SERVICES_SPRINF_SPRINFSERVICE_H
