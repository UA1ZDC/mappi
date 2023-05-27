#ifndef METEO_COMMONS_SERVICES_CUSTOMVIEWER_CUSTOMVIEWERSERVICE_H
#define METEO_COMMONS_SERVICES_CUSTOMVIEWER_CUSTOMVIEWERSERVICE_H

#include <meteo/commons/proto/customviewer.pb.h>
#include <meteo/commons/global/appstatusthread.h>
#include <meteo/commons/services/customviewer/customviewerservicehandler.h>

namespace meteo {

class CustomViewerService: public proto::CustomViewerService {
public:
  CustomViewerService();
  ~CustomViewerService() override;


  virtual void LoadData(::google::protobuf::RpcController* controller,
                       const ::meteo::proto::CustomviewerDataRequest* request,
                       ::meteo::proto::CustomviewerDataResponce* response,
                       ::google::protobuf::Closure* done) override;

  virtual void LoadConfig(::google::protobuf::RpcController* controller,
                       const ::meteo::proto::ViewerConfigRequest* request,
                       ::meteo::proto::ViewerConfig* response,
                       ::google::protobuf::Closure* done) override;

  virtual void CountData(::google::protobuf::RpcController* controller,
                       const ::meteo::proto::CustomViewerCountRequest* request,
                       ::meteo::proto::CustomViewerCountResponce* response,
                       ::google::protobuf::Closure* done) override;

  virtual void GetLastTlgId(::google::protobuf::RpcController* controller,
                       const ::meteo::proto::Dummy* request,
                       ::meteo::proto::GetLastTlgIdResponce* response,
                       ::google::protobuf::Closure* done) override;

  virtual void GetAvailableTT(::google::protobuf::RpcController* controller,
                       const ::meteo::proto::Dummy* request,
                       ::meteo::proto::GetAvailableTTResponce* response,
                       ::google::protobuf::Closure* done) override;

  virtual void GetTypecAreas(::google::protobuf::RpcController* controller,
                       const ::meteo::proto::Dummy* request,
                       ::meteo::proto::GetTypecAreasResponce* response,
                       ::google::protobuf::Closure* done) override;

  virtual void GetIcon(::google::protobuf::RpcController* controller,
                       const ::meteo::proto::CustomViewerIconRequest* request,
                       ::meteo::proto::CustomViewerIconResponce* response,
                       ::google::protobuf::Closure* done) override;

  virtual void GetTelegramDetails(::google::protobuf::RpcController* controller,
                       const ::meteo::proto::MsgViewerDetailsRequest* request,
                       ::meteo::proto::MsgViewerDetailsResponce* response,
                       ::google::protobuf::Closure* done) override;

private:
  CustomViewerServiceHandler *method_;

};

}

#endif
