#ifndef METEO_MASLO_RPC_CHANNEL_H
#define METEO_MASLO_RPC_CHANNEL_H

#include <google/protobuf/service.h>
#include <google/protobuf/descriptor.h>

#include <qobject.h>
#include <qhash.h>


#include <meteo/commons/rpc/clienthandler.h>
#include <meteo/commons/rpc/controller.h>
#include <meteo/commons/rpc/methodpack.h>

namespace meteo {
namespace rpc {

class Transport;
class MethodPack;
class Server;
class Controller;
class Channel;
class MethodClosureBase;
class ClientHandler;

class Channel : public QObject, public google::protobuf::RpcChannel
{
  Q_OBJECT
  public:
    //! Вызывается на стороне клиента для соединения с сервером, предоставляющим сервис
    static rpc::Channel* connect( const QString& address );
    static rpc::Channel* connect_v1( const QString& address );
    //!Вызывается на стороне сервера при обработке входящего соединения
    static rpc::Channel* create( Server* server, Transport* transport );
    ~Channel();

    int protocol() const ;

    bool isConnected() const ;
    bool isServerSide() const;

    QString address() const ;

    QString clientId() const ;
    ClientHandler* handler() const { return handler_; }

    bool sendMessage( const MethodPack& pack );

    void addClientClosure( MethodClosureBase* closure );
    void removeClientClosure( MethodClosureBase* closure );
    void removeServerConroller( Controller* ctrl );

    uint8_t maclevel() const ;

  public:
    void CallMethod(const google::protobuf::MethodDescriptor* method,
        google::protobuf::RpcController* controller,
        const google::protobuf::Message* request,
        google::protobuf::Message* response,
        google::protobuf::Closure* done);

    static ClientHandler* handlerFromController( google::protobuf::RpcController* ctrl );


  private:
    Transport* transport_ = nullptr;
    Server* server_ = nullptr;
    ClientHandler* handler_ = nullptr;
    int32_t msecs_timeout_ = 0;
    bool blocked_ = false;
    QList<Controller*> controllers_;
    QList<google::protobuf::Closure*> closures_;

  private:
    Channel();

  private:
    //! Обработка запроса/ответа при  получении
    void handleMessage( const MethodPack& packet );
    //! Обработка запроса на стороне сервера
    void handleRequest( const MethodPack& packet );
    //! Обработка ответа на сотороне клиента
    void handleResponse( const MethodPack& pack );

    bool sendClientMessage( MethodPack* pack );
    bool sendBlockedClientMessage( MethodPack* pack );

  private:
    void setBlockParams( int32_t msecs_timeout, bool blocked );
    void resetBlockParams() { msecs_timeout_ = 0; blocked_ = false; }
    int32_t timeout() const { return msecs_timeout_; }
    bool blocked() const { return blocked_; }

  public:
    Q_SLOT void slotMessageReceived();

  private:
    Q_SLOT void slotConnected();
    Q_SLOT void slotDisconnected();
  public:
  Q_SIGNAL  void connected();
  Q_SIGNAL  void disconnected();
//
  Q_SIGNAL  void clientSubscribed( Controller* ctrl );
  Q_SIGNAL  void clientUnsubscribed( Controller* ctrl );


  public:
    template< class Service, class Request, class Response, class CallbackClass > bool remoteCall(
        void (Service::*method )(::google::protobuf::RpcController*, const Request*, Response*, ::google::protobuf::Closure* ),
        const Request& rqst,
        CallbackClass* clbclass,
        void (CallbackClass::*func)( Response* )
        );
    template < class Service, class Request, class Response, class CallbackClass >
    bool remoteCall(
            void (Service::*method )(::google::protobuf::RpcController*, const Request*, Response*, ::google::protobuf::Closure* ),
            const Request& rqst,
            CallbackClass* clbclass,
            void (CallbackClass::*func)( Response*, Channel* )
            );

    template< class Service, class Request, class Response > Response* remoteCall(
        void (Service::*method )(::google::protobuf::RpcController*, const Request*, Response*, ::google::protobuf::Closure* ),
        const Request& rqst,
        int32_t msecs_timeout,
        bool blocked = true
        );

    template< class Service, class Request, class Response, class CallbackClass > bool subscribe(
        void (Service::*method )(::google::protobuf::RpcController*, const Request*, Response*, ::google::protobuf::Closure* ),
        const Request& rqst,
        CallbackClass* clbclass,
        void (CallbackClass::*func)( Response* )
        );

    friend class Server;
    friend class Controller;
};

class MethodClosureBase : public google::protobuf::Closure
{
  public:
    virtual ~MethodClosureBase(){};

  protected:

  friend class Channel;
};

template< class Request, class Response, class CallbackClass >
class MethodClosure : public MethodClosureBase
{
  public:
    typedef void (CallbackClass::*ClientMethod1)( Response* response );
    typedef void (CallbackClass::*ClientMethod2)( Response* response, Channel* ch );
    MethodClosure( CallbackClass* o, ClientMethod1 m, Request* r, Response* a, Channel* ch, Controller* ctrl )
      : type_(1), object_(o), method1_(m), arg_(a), req_(r), channel_(ch), ctrl_(ctrl) { channel_->addClientClosure(this); }
    MethodClosure( CallbackClass* o, ClientMethod2 m, Request* r, Response* a, Channel* ch, Controller* ctrl )
      : type_(2),object_(o), method2_(m), arg_(a), req_(r), channel_(ch), ctrl_(ctrl) { channel_->addClientClosure(this); }
    ~MethodClosure() {
      delete req_; req_ = nullptr;
      delete ctrl_; ctrl_ = nullptr;
      delete arg_; arg_ = nullptr;
    }
    void Run()
    {
      channel_->removeClientClosure(this);
      if ( 1 == type_ ) {
        (object_->*method1_)(arg_);
      }
      else {
        (object_->*method2_)( arg_, channel_ );
      }
      arg_ = nullptr;
      delete this;
    }

  private:
    int32_t type_ = 0;
    CallbackClass* object_ = nullptr;
    ClientMethod1 method1_;
    ClientMethod2 method2_;
    Response* arg_;
    Request* req_ = nullptr;
    Channel* channel_ = nullptr;
    Controller* ctrl_ = nullptr;
};

template < class Service, class Request, class Response, class CallbackClass >
bool Channel::remoteCall(
        void (Service::*method )(::google::protobuf::RpcController*, const Request*, Response*, ::google::protobuf::Closure* ),
        const Request& rqst,
        CallbackClass* clbclass,
        void (CallbackClass::*func)( Response* )
        )
{
  if ( false == isConnected() ) {
   // error_log << QObject::tr("Нет соединения");
    return false;
  }
  Controller* ctrl = new Controller(this);
  Request* req = new Request;
  req->CopyFrom(rqst);
  Response* resp = new Response;
  ::google::protobuf::Closure* done = new MethodClosure< Request, Response, CallbackClass >( clbclass, func, req, resp, this, ctrl );

  typename Service::Stub service(this);
  (service.*method)( ctrl, req, resp, done );
  return true;
}

template < class Service, class Request, class Response, class CallbackClass >
bool Channel::remoteCall(
        void (Service::*method )(::google::protobuf::RpcController*, const Request*, Response*, ::google::protobuf::Closure* ),
        const Request& rqst,
        CallbackClass* clbclass,
        void (CallbackClass::*func)( Response*, Channel* )
        )
{
  if ( false == isConnected() ) {
  //  error_log << QObject::tr("Нет соединения");
    return false;
  }
  Controller* ctrl = new Controller(this);
  Request* req = new Request;
  req->CopyFrom(rqst);
  Response* resp = new Response;
  ::google::protobuf::Closure* done = new MethodClosure< Request, Response, CallbackClass >( clbclass, func, req, resp, this, ctrl );

  typename Service::Stub service(this);
  (service.*method)( ctrl, req, resp, done );
  return true;
}

template < class Service, class Request, class Response >
Response* Channel::remoteCall(
        void (Service::*method )(::google::protobuf::RpcController*, const Request*, Response*, ::google::protobuf::Closure* ),
        const Request& rqst,
        int32_t msecs_timeout,
        bool blocked
        )
{
  if ( false == isConnected() ) {
   // error_log << QObject::tr("Нет соединения");
    return nullptr;
  }
  typename Service::Stub service(this);
  Controller* ctrl = new Controller(this);
  Request* req = new Request;
  req->CopyFrom(rqst);
  Response* resp = new Response;
  setBlockParams( msecs_timeout, blocked );
  (service.*method)( ctrl, req, resp, nullptr );
  resetBlockParams();
  if ( true == ctrl->Failed() ) {
  //  error_log << QObject::tr("Ошибка выполнения rpc-процедуры. Причина = %1")
  //    .arg( QString::fromStdString( ctrl->ErrorText() ) );
    delete resp;
    resp = nullptr;
  }
  delete req;
  delete ctrl;

  return resp;
}

template < class Service, class Request, class Response, class CallbackClass >
bool Channel::subscribe(
        void (Service::*method )(::google::protobuf::RpcController*, const Request*, Response*, ::google::protobuf::Closure* ),
        const Request& rqst,
        CallbackClass* clbclass,
        void (CallbackClass::*func)( Response* )
        )
{
  if ( false == isConnected() ) {
  //  error_log << QObject::tr("Нет соединения");
    return false;
  }
  typename Service::Stub service(this);
  Controller* ctrl = new Controller(this);
  Request* req = new Request();
  req->CopyFrom(rqst);
  Response* resp = new Response();
  ::google::protobuf::Closure* done = ::google::protobuf::NewPermanentCallback( clbclass, func, resp );
  (service.*method)( ctrl, req, resp, done );

  return true;
}

}
}

#endif
