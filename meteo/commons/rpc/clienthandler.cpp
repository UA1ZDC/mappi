#include "clienthandler.h"

#include <qdir.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qpainter.h>
#include <qimage.h>
#include <qbuffer.h>
#include <qprocess.h>

#include <sql/dbi/gridfs.h>
#include <sql/nosql/nosqlquery.h>

#include <commons/textproto/tprototext.h>

#include <meteo/commons/rpc/rpc.h>
#include <meteo/commons/rpc/channel.h>

namespace meteo {
namespace rpc {

ClientHandler::ClientHandler( Channel* ch )
  : QObject(),
  channel_(ch)
{
  if ( nullptr != QThreadPool::globalInstance() ) {
    QThreadPool::globalInstance()->setMaxThreadCount(1024);
  }
}

ClientHandler::~ClientHandler()
{
  for ( auto c : calls_ ) {
    c->setClientHandler(nullptr);
  }
//  debug_log << "UDALYATSYA HANDLER =" << this << "CHANNEL ="<< channel_;
//  emit methodsFinished(this);
  channel_ = nullptr;
}

void ClientHandler::removeCall( DefferedCallbackBase* call )
{
  calls_.removeAll(call);
  if ( 0 == calls_.size() ) {
    emit methodsFinished(this);
  }
}

bool ClientHandler::finished() const
{
  return 0 == calls_.size();
}

DefferedCallbackBase::DefferedCallbackBase( google::protobuf::Closure* done )
  : done_(done),
  watcher_( new QFutureWatcher<void>() )
{
//  debug_log << "YA RODILSYA!" << this;
  QObject::connect( watcher_, SIGNAL( finished() ), this, SLOT( slotFutureFinished() ) );
}

DefferedCallbackBase::~DefferedCallbackBase()
{
//  debug_log << "UDALYAETSYA METHOD =" << this;
  QObject::disconnect(watcher_);
  delete watcher_;
  watcher_ = nullptr;
  if ( nullptr != done_ ) {
    if(true == need_run_){
      done_->Run();
    }
    done_ = nullptr;
  }
  if ( nullptr != clienthandler_ ) {
    clienthandler_->removeCall(this);
  }
}

void DefferedCallbackBase::setClientHandler( ClientHandler* h )
{
  clienthandler_ = h;
}

void DefferedCallbackBase::setNeedRun( bool nr )
{
  need_run_ = nr;
}

void DefferedCallbackBase::slotFutureFinished()
{
//  debug_log << "ZAVERSHILSYA METHOD =" << this;
  this->deleteLater();
}

}
}
