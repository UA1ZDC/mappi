#ifndef METEO_COMMONS_RPC_CLIENTHANDLER_H
#define METEO_COMMONS_RPC_CLIENTHANDLER_H

#include <google/protobuf/service.h>
#include <google/protobuf/descriptor.h>

#include <qobject.h>
#include <qmap.h>
#include <qpair.h>
#include <qmutex.h>
#include <qfuturewatcher.h>
#include <qfuture.h>
#include <QtConcurrent/qtconcurrentrun.h>

#include <meteo/commons/rpc/controller.h>

namespace meteo {

namespace rpc {

template<typename Thread, typename Request, typename Response>
class DefferedCallback;
class DefferedCallbackBase;

class ClientHandler : public QObject
{
  Q_OBJECT
public:
  ClientHandler( Channel* ch );
  ~ClientHandler();

  template <typename Thread, typename Request, typename Response >
  bool runMethod( google::protobuf::RpcController* ctrl, Thread* h,
                  void (Thread::*method)( Request, Response ), Request, Response, bool need_run = true );

  Channel* channel() const { return channel_; }
  bool finished() const ;

public:
  void removeCall( DefferedCallbackBase* );

private:
  rpc::Channel* channel_;
  QList<DefferedCallbackBase*> calls_;

//signals:
  Q_SIGNAL void methodsFinished( meteo::rpc::ClientHandler* hndlr );

  friend class StatusSender;
};

template <typename Thread, typename Request, typename Response>
bool ClientHandler::runMethod( google::protobuf::RpcController* ctrl, Thread* h,
                               void(Thread::*method)( Request, Response),
                               Request req, Response resp, bool need_run )
{
  Controller* c = static_cast<Controller*>(ctrl);
  if ( nullptr == c ) {
    //error_log << QObject::tr("Неизвестный контроллер вызова");
    return false;
  }
  DefferedCallback<Thread, Request,Response>* dcb
      = new DefferedCallback<Thread, Request, Response>(h, method, req, resp, c->closure() );
  dcb->setClientHandler(this);
  dcb->setNeedRun(need_run);
  calls_.append(dcb);
  dcb->Run();
  return true;
}

class DefferedCallbackBase : public QObject, public google::protobuf::Closure
{
  Q_OBJECT
public:
  DefferedCallbackBase( google::protobuf::Closure* done );
  ~DefferedCallbackBase();

public:
  void setClientHandler( ClientHandler* h );
  void setNeedRun( bool nr );

protected:
  google::protobuf::Closure* done_ = nullptr;
  QFutureWatcher<void>* watcher_ = nullptr;
  ClientHandler* clienthandler_ = nullptr;
  bool need_run_ = true;
private :
  Q_SLOT void slotFutureFinished();
};

template<typename Thread, typename Request, typename Response>
class DefferedCallback : public DefferedCallbackBase
{
public:
  typedef void (Thread::*Method)( Request request, Response response );
  DefferedCallback( Thread* h, Method m, Request request, Response response, Closure* done )
    : DefferedCallbackBase(done),
      handler_(h),
      method_(m),
      request_(request),
      response_(response)
  {
  }

  ~DefferedCallback()
  {
  }

  void Run()
  {
    if ( true == hasConnection() ) {
      QFuture<void> future = QtConcurrent::run( handler_, method_, request_, response_ );
      watcher_->setFuture(future);
    }
  }

  bool hasConnection() const
  {
    return true;
  }

private:
  Thread* handler_ = nullptr;
  Method method_;
  Request request_;
  Response response_;

};

}
}

#endif
