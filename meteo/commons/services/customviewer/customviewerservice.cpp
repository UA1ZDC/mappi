#include "customviewerservice.h"

namespace meteo {

CustomViewerService::CustomViewerService():
  method_(new CustomViewerServiceHandler())
{

}

CustomViewerService::~CustomViewerService()
{
  if ( nullptr != method_ ) {
    delete method_;
  }
}

void CustomViewerService::LoadData(::google::protobuf::RpcController* controller,
                     const ::meteo::proto::CustomviewerDataRequest* request,
                     ::meteo::proto::CustomviewerDataResponce* response,
                     ::google::protobuf::Closure* done)
{
  rpc::ClientHandler* handler = rpc::Channel::handlerFromController(controller);
   if ( nullptr == handler ){
     static const QString emessage = QObject::tr("Не удается получить handler для контроллера");
     error_log << emessage;
     done->Run();
     return;
   }
   handler->runMethod( controller,
                       method_, &CustomViewerServiceHandler::LoadData,
                       request, response );

}


void CustomViewerService::LoadConfig(::google::protobuf::RpcController* controller,
                                     const ::meteo::proto::ViewerConfigRequest* request,
                                     ::meteo::proto::ViewerConfig* response,
                                     ::google::protobuf::Closure* done)
{
  rpc::ClientHandler* handler = rpc::Channel::handlerFromController(controller);
   if ( nullptr == handler ){
     static const QString emessage = QObject::tr("Не удается получить handler для контроллера");
     error_log << emessage;
     done->Run();
     return;
   }
   handler->runMethod( controller,
                       method_, &CustomViewerServiceHandler::LoadConfig,
                       request, response );

}

void CustomViewerService::CountData(::google::protobuf::RpcController* controller,
                                    const ::meteo::proto::CustomViewerCountRequest* request,
                                    ::meteo::proto::CustomViewerCountResponce* response,
                                    ::google::protobuf::Closure* done)
{
  rpc::ClientHandler* handler = rpc::Channel::handlerFromController(controller);
   if ( nullptr == handler ){
     static const QString emessage = QObject::tr("Не удается получить handler для контроллера");
     error_log << emessage;
     done->Run();
     return;
   }
   handler->runMethod( controller,
                       method_, &CustomViewerServiceHandler::CountData,
                       request, response );

}

void CustomViewerService::GetLastTlgId(::google::protobuf::RpcController* controller,
                                       const ::meteo::proto::Dummy* request,
                                       ::meteo::proto::GetLastTlgIdResponce* response,
                                       ::google::protobuf::Closure* done)
{
  rpc::ClientHandler* handler = rpc::Channel::handlerFromController(controller);
   if ( nullptr == handler ){
     static const QString emessage = QObject::tr("Не удается получить handler для контроллера");
     error_log << emessage;
     done->Run();
     return;
   }
   handler->runMethod( controller,
                       method_, &CustomViewerServiceHandler::GetLastTlgId,
                       request, response );
}

void CustomViewerService::GetAvailableTT(::google::protobuf::RpcController* controller,
                                         const ::meteo::proto::Dummy* request,
                                         ::meteo::proto::GetAvailableTTResponce* response,
                                         ::google::protobuf::Closure* done)
{
  rpc::ClientHandler* handler = rpc::Channel::handlerFromController(controller);
   if ( nullptr == handler ){
     static const QString emessage = QObject::tr("Не удается получить handler для контроллера");
     error_log << emessage;
     done->Run();
     return;
   }
   handler->runMethod( controller,
                       method_, &CustomViewerServiceHandler::GetAvailableTT,
                       request, response );
}

void CustomViewerService::GetTypecAreas(::google::protobuf::RpcController* controller,
                                        const ::meteo::proto::Dummy* request,
                                        ::meteo::proto::GetTypecAreasResponce* response,
                                        ::google::protobuf::Closure* done)
{
  rpc::ClientHandler* handler = rpc::Channel::handlerFromController(controller);
   if ( nullptr == handler ){
     static const QString emessage = QObject::tr("Не удается получить handler для контроллера");
     error_log << emessage;
     done->Run();
     return;
   }
   handler->runMethod( controller,
                       method_, &CustomViewerServiceHandler::GetTypecAreas,
                       request, response );
}

void CustomViewerService::GetIcon(::google::protobuf::RpcController* controller,
                                  const ::meteo::proto::CustomViewerIconRequest* request,
                                  ::meteo::proto::CustomViewerIconResponce* response,
                                  ::google::protobuf::Closure* done) {
  rpc::ClientHandler* handler = rpc::Channel::handlerFromController(controller);
   if ( nullptr == handler ){
     static const QString emessage = QObject::tr("Не удается получить handler для контроллера");
     error_log << emessage;
     done->Run();
     return;
   }
   handler->runMethod( controller,
                     method_, &CustomViewerServiceHandler::GetIcon,
                     request, response );
}

void CustomViewerService::GetTelegramDetails(::google::protobuf::RpcController* controller,
                                             const ::meteo::proto::MsgViewerDetailsRequest* request,
                                             ::meteo::proto::MsgViewerDetailsResponce* response,
                                             ::google::protobuf::Closure* done)
{
  rpc::ClientHandler* handler = rpc::Channel::handlerFromController(controller);
   if ( nullptr == handler ){
     static const QString emessage = QObject::tr("Не удается получить handler для контроллера");
     error_log << emessage;
     done->Run();
     return;
   }
   handler->runMethod( controller,
                     method_, &CustomViewerServiceHandler::GetTelegramDetails,
                     request, response );
}

}
