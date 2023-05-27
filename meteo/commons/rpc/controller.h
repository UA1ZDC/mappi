#ifndef METEO_MASLO_RPC_CONTROLLER_H
#define METEO_MASLO_RPC_CONTROLLER_H

#include <google/protobuf/service.h>
#include <google/protobuf/message.h>

#include <string>

#include <qobject.h>
#include <qtimer.h>
#include <qeventloop.h>
#include <qcoreapplication.h>

namespace meteo {
namespace rpc {

class Controller;
class Channel;

class Controller : public google::protobuf::RpcController
{
  public:
    Controller( Channel* ch );
    ~Controller();

    Channel* channel () const { return channel_;}
    const QByteArray& callid() const { return callid_; }
    void setCallid( const QByteArray& id ) { callid_ = id; }

    bool isServerSide() const ;
    void setResponse( google::protobuf::Message* msg ) { response_ = msg; }
    void setRequest( const google::protobuf::Message* msg ) { request_ = msg; }
    void setService( const QByteArray& service ) { service_ = service; }
    void setMethod( const google::protobuf::MethodDescriptor* method ) { method_ = method; }
    void setClosure(  google::protobuf::Closure* done ) { closure_ = done; }

    const google::protobuf::Message* request() const { return request_; }
    google::protobuf::Message* response() const { return response_; }
    const google::protobuf::MethodDescriptor* method() const { return method_; }
    const QByteArray& service() const { return service_; }
    google::protobuf::Closure* closure() const  { return closure_; }

    void Reset() {}
    bool Failed() const { return failed_; }
    std::string ErrorText() const { return failreason_; }
    void StartCancel() {}


    void SetFailed( const std::string& reason ) { failed_ = true; failreason_ = reason; }
    bool IsCanceled() const { return false; }
    void NotifyOnCancel( google::protobuf::Closure* closure ) { Q_UNUSED(closure); }

  private:
    Channel* channel_ = nullptr; //!< канал передачи rpc-вызова
    QByteArray callid_;
    bool failed_ = false;
    std::string failreason_;
    const google::protobuf::Message* request_ = nullptr;
    google::protobuf::Message* response_ = nullptr;
    google::protobuf::Closure* closure_ = nullptr;
    QByteArray service_;
    const google::protobuf::MethodDescriptor* method_;
};

}
}

#endif 
