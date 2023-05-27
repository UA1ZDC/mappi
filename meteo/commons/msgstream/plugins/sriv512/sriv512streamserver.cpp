#include "sriv512streamserver.h"
#include "sriv512server.h"

#include <cross-commons/debug/tlog.h>

namespace meteo {
const int kMaxConnection = 1;

Sriv512StreamServer::Sriv512StreamServer(QObject* parent)
  : StreamModule(parent)
{
}

Sriv512StreamServer::~Sriv512StreamServer()
{
  if(nullptr != server_){
      server_->close();
      delete server_;
      server_ = nullptr;
    }
}

void Sriv512StreamServer::setOptions(const msgstream::Options& options)
{
  opt_ = options;

}

bool Sriv512StreamServer::slotInit()
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
      server_ = new Sriv512Server(status_, this);
      server_->setMaxPendingConnections( kMaxConnection );
      server_->init(opt_,outgoing_,incoming_);
      server_->listen(QHostAddress::Any,opt_.sriv512().port());
    }
  return true;
}

void Sriv512StreamServer::slotNewIncoming(){
  emit newOutgoingMsg();
}

} // meteo
