#ifndef METEO_COMMONS_FIELD_ANALYSE_H
#define METEO_COMMONS_FIELD_ANALYSE_H

#include <memory>

#include <meteo/commons/proto/field.pb.h>

#include "fieldservice.h"

namespace meteo {
namespace field {

  class TFieldAnalyseWrap;
  class Control;

class TFieldAnalyse : public TFieldDataService
{
  using RpcController = google::protobuf::RpcController;
  using Closure = google::protobuf::Closure;

public:
  TFieldAnalyse(::meteo::AppStatusThread* status);
  ~TFieldAnalyse();

  template <typename Thread, typename Request, typename Response >
  bool runMethod( google::protobuf::RpcController* ctrl,
                  void (Thread::*method)( Request, Response ), Request, Response );

 // void GetAdvectFields(RpcController* c, const AdvectDataRequest* req, AdvectDataReply* resp, Closure* d);
 // void GetAdvectPoints(RpcController* c, const AdvectDataRequest* req, AdvectDataReply* resp, Closure* d);
  void GetAdvectObject(RpcController* c, const AdvectObjectRequest* req, AdvectObjectReply* resp, Closure* d);
//  void GetAdvectObjects(RpcController* c, const AdvectObjectsRequest* req, AdvectObjectsReply* resp, Closure* d);
  void getSynSit(RpcController* c, const DataRequest* req, ValueDataReply* resp, Closure* d);
  void CalcForecastOpr(RpcController* c, const DataAnalyseRequest* req, SimpleDataReply* res, Closure* d);//!< Провести анализ поля
  void GetExtremumsTraj(RpcController* c, const SimpleDataRequest* req, ExtremumTrajReply* res, Closure* d);//!< Получить траектории центров барических образований


 /*
  * НЕОБХОДИМО ДЛЯ ProcIdentWidget
  *
  void AddProcessField(RpcController* c, const ProcessFieldData* req, SimpleDataReply* resp, Closure* d);
  void EditProcessField(RpcController* c, const ProcessFieldData* req, SimpleDataReply* resp, Closure* d);
  void DeleteProcessField(RpcController* c, const SimpleDataRequest* req, SimpleDataReply* resp, Closure* d);
  void GetProcessFields(RpcController* c, const SimpleDataRequest* req, ProcessFieldsData* resp, Closure* d);
  void GetProcessFieldsKorr(RpcController* c, const SimpleDataRequest* req, ProcessFieldsData* resp, Closure* d);
*/

  TFieldAnalyseWrap* wmethod_ = nullptr;

};

}
}

#endif
