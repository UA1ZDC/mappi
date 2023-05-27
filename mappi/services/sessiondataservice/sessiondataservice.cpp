#include <meteo/commons/rpc/rpc.h>

#include "sessiondataservice.h"
#include "methodbag.h"

namespace mappi {

  SessionDataService::SessionDataService():
    proto::SessionDataService(),
    methodbag_( new MethodBag(this) )
  {

  }

  SessionDataService::~SessionDataService()
  {
    delete methodbag_; methodbag_ = 0;
  }

  void SessionDataService::SaveSession(::google::protobuf::RpcController* ctrl, const proto::SessionData* req, proto::Response* resp, ::google::protobuf::Closure* done)
  {
    trc;
    meteo::rpc::ClientHandler* handler = meteo::rpc::Channel::handlerFromController(ctrl);
    if ( nullptr == handler ) {
      error_log << QObject::tr("Не удалось получить обработчик из контроллера");
      resp->set_result(false);
      done->Run();
      return;
    }

    handler->runMethod( ctrl, methodbag_, &MethodBag::SaveSession, qMakePair( handler, req ), resp );
  }

  void SessionDataService::SavePretreatment(::google::protobuf::RpcController* ctrl, const proto::Pretreatment* req, proto::Response* resp, ::google::protobuf::Closure* done)
  {
    trc;
    meteo::rpc::ClientHandler* handler = meteo::rpc::Channel::handlerFromController(ctrl);
    if ( nullptr == handler ) {
      error_log << QObject::tr("Не удалось получить обработчик из контроллера");
      resp->set_result(false);
      done->Run();
      return;
    }

    handler->runMethod( ctrl, methodbag_, &MethodBag::SavePretreatment, qMakePair( handler, req ), resp );
  }

  void SessionDataService::GetAvailableThematic(google::protobuf::RpcController *ctrl, const mappi::proto::ThematicData *req, mappi::proto::ThematicList *resp, google::protobuf::Closure *done)
  {
    meteo::rpc::ClientHandler* handler = meteo::rpc::Channel::handlerFromController(ctrl);
    if ( nullptr == handler ) {
      error_log << QObject::tr("Не удалось получить обработчик из контроллера");;
      resp->set_result(false);
      done->Run();
      return;
    }
  
    handler->runMethod( ctrl, methodbag_, &MethodBag::GetAvailableThematic, qMakePair( handler, req ), resp );
  }


  void SessionDataService::GetGeotiffInfo(google::protobuf::RpcController *ctrl, const mappi::proto::ThematicData *req,
					  mappi::proto::GeotifHeader *resp, google::protobuf::Closure *done)
  {
    meteo::rpc::ClientHandler* handler = meteo::rpc::Channel::handlerFromController(ctrl);
    if ( nullptr == handler ) {
      error_log << QObject::tr("Не удалось получить обработчик из контроллера");;
      resp->set_result(false);
      done->Run();
      return;
    }
    
    handler->runMethod( ctrl, methodbag_, &MethodBag::GetGeotiffInfo, qMakePair( handler, req ), resp );
  }
  
  void SessionDataService::GetAvailableSessions(google::protobuf::RpcController *ctrl, const proto::SessionData *req, proto::SessionList *resp, google::protobuf::Closure *done)
  {
    meteo::rpc::ClientHandler* handler = meteo::rpc::Channel::handlerFromController(ctrl);
    if ( nullptr == handler ) {
      error_log << QObject::tr("Не удалось получить обработчик из контроллера");;
      resp->set_result(false);
      done->Run();
      return;
    }

    handler->runMethod( ctrl, methodbag_, &MethodBag::GetAvailableSessions, qMakePair( handler, req ), resp );   
  }

  
  void SessionDataService::GetAvailablePretreatment(google::protobuf::RpcController *ctrl, const proto::Pretreatment *req, proto::PretreatmentList *resp, google::protobuf::Closure *done)
  {
    meteo::rpc::ClientHandler* handler = meteo::rpc::Channel::handlerFromController(ctrl);
    if ( nullptr == handler ) {
      error_log << QObject::tr("Не удалось получить обработчик из контроллера");
      resp->set_result(false);
      done->Run();
      return;
    }

    handler->runMethod( ctrl, methodbag_, &MethodBag::GetAvailablePretreatment, qMakePair( handler, req ), resp );  
  }

  
  void SessionDataService::SaveThematic(google::protobuf::RpcController *ctrl, const proto::ThematicData *req, proto::Response *resp, google::protobuf::Closure *done)
  {
    trc;
    meteo::rpc::ClientHandler* handler = meteo::rpc::Channel::handlerFromController(ctrl);
    if ( nullptr == handler ) {
      error_log << QObject::tr("Не удалось получить обработчик из контроллера");
      resp->set_result(false);
      done->Run();
      return;
    }

    handler->runMethod( ctrl, methodbag_, &MethodBag::SaveThematic, qMakePair( handler, req ), resp );  
  }


  
  void SessionDataService::MarkSessionThematicDone(google::protobuf::RpcController *ctrl, const proto::SessionData *req, proto::Response *resp, google::protobuf::Closure *done)
  {
    meteo::rpc::ClientHandler* handler = meteo::rpc::Channel::handlerFromController(ctrl);
    if ( nullptr == handler ) {
      error_log << QObject::tr("Не удалось получить обработчик из контроллера");
      resp->set_result(false);
      done->Run();
      return;
    }

    handler->runMethod( ctrl, methodbag_, &MethodBag::MarkSessionThematicDone, qMakePair( handler, req ), resp );
  }

  
  void SessionDataService::GetSession(google::protobuf::RpcController *ctrl, const proto::SessionData *req, proto::SessionList *resp, google::protobuf::Closure *done)
  {
    meteo::rpc::ClientHandler* handler = meteo::rpc::Channel::handlerFromController(ctrl);
    if ( nullptr == handler ) {
      error_log << QObject::tr("Не удалось получить обработчик из контроллера");
      resp->set_result(false);
      done->Run();
      return;
    }

    handler->runMethod( ctrl, methodbag_, &MethodBag::GetSession, qMakePair( handler, req ), resp );
  }

  
  void SessionDataService::GetAvailableThematicTypes(google::protobuf::RpcController *ctrl, const Dummy *req, proto::ThematicList *resp, google::protobuf::Closure *done)
  {
    meteo::rpc::ClientHandler* handler = meteo::rpc::Channel::handlerFromController(ctrl);
    if ( nullptr == handler ) {
      error_log << QObject::tr("Не удалось получить обработчик из контроллера");
      resp->set_result(false);
      done->Run();
      return;
    }

    handler->runMethod( ctrl, methodbag_, &MethodBag::GetAvailableThematicTypes, qMakePair( handler, req ), resp );
  }

  
  void SessionDataService::GetGrayscaleChannels(google::protobuf::RpcController *ctrl, const Dummy *req, proto::ThematicList *resp, google::protobuf::Closure *done)
  {
    meteo::rpc::ClientHandler* handler = meteo::rpc::Channel::handlerFromController(ctrl);
    if ( nullptr == handler ) {
      error_log << QObject::tr("Не удалось получить обработчик из контроллера");
      resp->set_result(false);
      done->Run();
      return;
    }

    handler->runMethod( ctrl, methodbag_, &MethodBag::GetGrayscaleChannels, qMakePair( handler, req ), resp );
  }

} //mappi
