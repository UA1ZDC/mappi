#ifndef METEO_COMMONS_SERVICES_CLIMATDATA_CLIMATSERVICE_H
#define METEO_COMMONS_SERVICES_CLIMATDATA_CLIMATSERVICE_H

#include <meteo/commons/global/appstatusthread.h>
#include <meteo/commons/proto/climat.pb.h>
#include <meteo/commons/services/climatdata/climatsurf.h>
#include <meteo/commons/services/climatdata/climataero.h>

namespace meteo {
class Control;

namespace climat {

enum StatusParam {
  kRequestsCount,
  kSuccessResponseCount,
  kErorrResponseCount,
};

class ClimatService: public QObject, public Service
{
    Q_OBJECT

  public:
    ClimatService(::meteo::AppStatusThread* status = nullptr);
    ~ClimatService();

    void GetAvailable( google::protobuf::RpcController* c,
                       const meteo::climat::Dummy* req,
                       meteo::climat::DataSeriesDate* res,
                       google::protobuf::Closure* d );
    void GetAvailableStations( google::protobuf::RpcController* c,
                               const meteo::climat::Dummy* req,
                               meteo::climat::ClimatSaveProp* res,
                               google::protobuf::Closure* d );

    void GetSurfDay( google::protobuf::RpcController* c,
                     const meteo::climat::DataRequest* req,
                     meteo::climat::SurfDayReply* res,
                     google::protobuf::Closure* d );

    void GetSurfDecade( google::protobuf::RpcController* c,
                        const meteo::climat::DataRequest* req,
                        meteo::climat::SurfDecadeReply* res,
                        google::protobuf::Closure* d );

    void GetSurfMonth( google::protobuf::RpcController* c,
                       const meteo::climat::DataRequest* req,
                       meteo::climat::SurfMonthReply* res,
                       google::protobuf::Closure* d );

    void GetSurfMonthAvg( google::protobuf::RpcController* c,
                          const meteo::climat::DataRequest* req,
                          meteo::climat::SurfMonthAvgReply* res,
                          google::protobuf::Closure* d );

    void GetSurfYear( google::protobuf::RpcController* c,
                      const meteo::climat::DataRequest* req,
                      meteo::climat::SurfYearReply* res,
                      google::protobuf::Closure* d );
    // void GetSurfAll( google::protobuf::RpcController* c,
    // 			const meteo::climat::DataRequest* req,
    // 			meteo::climat::SurfMonthAllReply* res,
    // 			google::protobuf::Closure* d );

    void GetSurfAllList( google::protobuf::RpcController* c,
                         const meteo::climat::DataRequest* req,
                         meteo::climat::SurfAllListReply* res,
                         google::protobuf::Closure* d );

    void GetSurfDate( google::protobuf::RpcController* c,
                      const meteo::climat::DataRequest* req,
                      meteo::climat::SurfDateReply* res,
                      google::protobuf::Closure* d );

    void GetAeroMonthList( google::protobuf::RpcController* c,
                           const meteo::climat::DataRequest* req,
                           meteo::climat::AeroLayerListReply* res,
                           google::protobuf::Closure* d );

  private:
    bool getAvailable(meteo::climat::DataSeriesDate* res);
    bool getAvailable(const QString& collection,
                      meteo::sprinf::MeteostationType cltype,
                      meteo::climat::DataSeriesDate* res);

    bool getAvailableStations(meteo::climat::ClimatSaveProp* res);
  public slots:
    void slotAnsverSendedError();
    void slotAnsverSendedOK();
    void slotRequestReceive();

  signals:
    void signalRequestReceive();
  private:

    ClimatSurfService surf_;
    ClimatAeroService aero_;
    ::meteo::AppStatusThread* status_ = nullptr;

};

}
}

#endif
