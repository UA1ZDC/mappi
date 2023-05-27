#ifndef METEO_COMMONS_SERVICE_FIELDATA_FIELDSERVICE_H
#define METEO_COMMONS_SERVICE_FIELDATA_FIELDSERVICE_H

#include <memory>

#include <meteo/commons/global/appstatusthread.h>
#include <meteo/commons/proto/field.pb.h>


namespace meteo {
namespace field {

enum StatusParam {
  kRequestsCount,
  kSuccessResponseCount,
  kErorrResponseCount,
};

class Control;
class MethodWrap;

class TFieldDataService : public QObject, public FieldService
{
    Q_OBJECT
    using RpcController = google::protobuf::RpcController;
    using Closure = google::protobuf::Closure;

  public:
    TFieldDataService(AppStatusThread* status);
    ~TFieldDataService();


    ////обработка запросов на получение одного поля
    void GetFieldData(RpcController* c, const DataRequest* req, DataReply* res, Closure* d);

    ////обработка запросов на получение набора полей данных
    void GetFieldsData(RpcController* c, const DataRequest* req, ManyDataReply* res, Closure* d);

    void GetFieldsMData(RpcController* c, const DataRequest* req, meteo::surf::DataReply* res, Closure* d);

    void GetFieldsDataWithoutForecast(RpcController* c, const DataRequest* req, ManyDataReply* res, Closure* d);

    void GetAvailableData(RpcController* c, const DataRequest* req, DataDescResponse* resp, Closure* d);

    // получаем точку адвекции
    // void GetAdvectPoints(RpcController* c, const DataRequest* req, DataDescResponse* resp, Closure* d);

    void GetFieldDataPoID(RpcController* c, const SimpleDataRequest* req, DataReply* resp, Closure* d);

    void GetADZond(RpcController* c, const DataRequest* req, meteo::surf::TZondValueReply* resp, Closure* d);

    void GetLastDate(RpcController* c, const DataRequest* req, SimpleDataReply* resp, Closure* d);

    void GetFieldDates(RpcController* c, const DataRequest* req, DateReply* resp, Closure* d);

    void GetNearDate(RpcController* c, const DataRequest* req, DateReply* resp, Closure* d);

    void GetFieldsForDate(RpcController*, const DataRequest* req, ManyFieldsForDatesResponse* resp, Closure* d);

    void GetForecastValues(RpcController* c, const DataRequest* req, ValueDataReply* resp, Closure* d);

    void GetAvailableCenters(RpcController* c, const DataRequest* req, CentersResponse* resp, Closure* d);

    void GetAvailableCentersForecast(RpcController* c, const DataRequest* req, CentersResponse* resp, Closure* d);

    void GetAvailableCentersAD(RpcController *c, const DataRequest *req, CentersResponse *resp, Closure *d);

    void GetAvailableHours(RpcController* c, const DataRequest* req, HourResponse* resp, Closure* d);

    void GetFieldDataForMeteosummary(RpcController* c, const DataRequest* req, ValueDataReply* resp, Closure* d);

    void GetIsoLines(::google::protobuf::RpcController* controller,
                     const ::meteo::field::DataRequest* request,
                     ::meteo::IsoLines* response,
                     ::google::protobuf::Closure* done);

    void GetFieldProto(::google::protobuf::RpcController* controller,
                       const ::meteo::field::DataRequest* request,
                       ::meteo::field::FieldReply* response,
                       ::google::protobuf::Closure* done);
    void GetGeoTiff(RpcController* c, const DataRequest* req, DataReply* res, Closure* d);
    void GetIsoLinesShp(RpcController* c, const DataRequest* req, IsoLinesShp* res, Closure* d);

  public slots:
    void slotAnsverSendedError();
    void slotAnsverSendedOK();
    void slotRequestReceive();
  signals:
    void signalRequestReceive();
  protected:
    AppStatusThread* status_;
  private:
    MethodWrap* method_ = nullptr;
};

}
}

#endif
