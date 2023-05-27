#include "serverthread.h"
#include <cross-commons/debug/tlog.h>

ServerThread::ServerThread()
{
  this->moveToThread(this);
  QObject::connect(this, &QThread::started, this, &ServerThread::init );
}

ServerThread::~ServerThread()
{
  if ( nullptr != server_ ){
    delete server_;
  }

  if ( nullptr != service_ ){
    delete service_;
  }
}

bool ServerThread::isServerStarted()
{
  if ( nullptr == server_ ){
    return false;
  }

  return (true == server_->isListening()) ;
}

void ServerThread::init()
{
  debug_log << __FUNCTION__;
  service_ = new meteo::test::TestService();
  server_ = new meteo::rpc::Server(service_, serviceAddress);

  if ( false == server_->init() ){
    this->exit(EXIT_FAILURE);
  }

  debug_log << "SERVER STARTED";
}
