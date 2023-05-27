#include "socksp2gstreamserver.h"
#include "socksp2gserver.h"

#include <cross-commons/debug/tlog.h>

namespace meteo {
const int kMaxConnection = 1;

Socksp2gStreamServer::Socksp2gStreamServer(QObject* parent)
  : StreamModule(parent)
{
}

Socksp2gStreamServer::~Socksp2gStreamServer()
{
  if(nullptr != server_){
      server_->close();
      delete server_;
      server_ = nullptr;
    }
}

void Socksp2gStreamServer::setOptions(const msgstream::Options& options)
{
  opt_ = options;

}

bool Socksp2gStreamServer::slotInit()
{
  if ( nullptr != status_) {
      status_->setTitle(kRecvCount, tr("Принято сообщений сегодня/всего"),0);
      status_->setTitle(kConfirmSended, tr("Отправлено подтверждений сегодня/всего"),0);
      status_->setTitle(kSendCount, tr("Передано сообщений сегодня/всего"),0);
      status_->setTitle(kConfirmRecv, tr("Принято подтверждений сегодня/всего"),0);
      status_->setTitle(kRecvSize, tr("Принято данных сегодня/всего"),0);
      status_->setTitle(kSendSize, tr("Передано данных сегодня/всего"),0);
      status_->setTitle(kLinkStatus, tr("Подключение к абоненту"));
      status_->setParam(kLinkStatus, tr("Отсутствует"),app::OperationState_ERROR);
      startTimer(kTimeToLastActivity);
      resetChecker();
    }
  if ( nullptr == server_ ) {
      server_ = new Socksp2gServer(status_, this);
      server_->setMaxPendingConnections( kMaxConnection );
      server_->init(opt_,outgoing_,incoming_);
      server_->listen(QHostAddress::Any,opt_.socksp2g().port());
    }
  return true;
}

void Socksp2gStreamServer::slotNewIncoming(){
  emit newOutgoingMsg();
}

} // meteo
