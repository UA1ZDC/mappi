#include "socksp2gstreamclient.h"
#include "socksp2greceiver.h"

#include <cstring>

#include <cross-commons/debug/tlog.h>

#include <meteo/commons/msgparser/common.h>
#include <meteo/commons/msgstream/plugins/socketspec2g/socksp2g.h>
#include <meteo/commons/msgstream/plugins/socketspec2g/socksp2greceiver.h>
#include <commons/funcs/mnio.h>

namespace meteo {

Socksp2gStreamClient::Socksp2gStreamClient(QObject* parent)
  : StreamModule(parent)
{
}


Socksp2gStreamClient::~Socksp2gStreamClient()
{
  slotStreamFinihsed();
}


void Socksp2gStreamClient::setOptions(const msgstream::Options& options)
{
  opt_ = options;
}


bool Socksp2gStreamClient::slotInit()
{
  if(nullptr != status_){
      status_->setTitle(kRecvCount, tr("Принято сообщений сегодня/всего"),0);
      status_->setTitle(kConfirmSended, tr("Отправлено подтверждений сегодня/всего"),0);
      status_->setTitle(kSendCount, tr("Передано сообщений сегодня/всего"),0);
      status_->setTitle(kConfirmRecv, tr("Принято подтверждений сегодня/всего"),0);
      status_->setTitle(kRecvSize, tr("Принято данных сегодня/всего"),0);
      status_->setTitle(kSendSize, tr("Передано данных сегодня/всего"),0);
      status_->setTitle(kLinkStatus, tr("Подключение к серверу"));
      status_->setParam(kLinkStatus, tr("Отсутствует"),app::OperationState_ERROR);
      startTimer(kTimeToLastActivity);
      resetChecker();
    }
  if ( nullptr == client_ ) {
      slotStreamFinihsed();
      networkThread_ = new QThread;
      client_ = new Socksp2gReceiver(opt_,outgoing_,incoming_,status_);
      client_->moveToThread( networkThread_ );
      connect( networkThread_, SIGNAL(started()), client_, SLOT(slotStart()) );
      connect( networkThread_, SIGNAL(finished()), this, SLOT(slotStreamFinihsed()) );

      QObject::connect(client_, SIGNAL(signalMessagesRecv(qint64 )), this, SLOT(slotMsgReceived(qint64)));
      QObject::connect(client_, SIGNAL(signalBytesRecv(qint64)), this, SLOT( slotSizeReceived(qint64)));
      QObject::connect(client_, SIGNAL(signalBytesSended(qint64)), this, SLOT(slotSizeSended(qint64)));
      QObject::connect(client_, SIGNAL(signalMessagesSended(qint64)), this, SLOT( slotMsgSended(qint64)));
      QObject::connect(client_, SIGNAL(signalConfirmSended(qint64)), this, SLOT(slotConfirmSended(qint64)));
      QObject::connect(client_, SIGNAL(signalConfirmRecv(qint64)), this, SLOT( slotConfirmReceived(qint64)));
      QObject::connect( this, SIGNAL(newOutgoingMsg()),client_, SLOT(slotNewMessage()) );
      //QObject::connect( this, SIGNAL(newIncomingMsg()),client_, SLOT(slotMessageToServer()) );
      networkThread_->start();
    }
  return true;
}


void Socksp2gStreamClient::slotStreamFinihsed(){
  if(nullptr != networkThread_){
      disconnect(networkThread_, nullptr, this, nullptr);
      networkThread_->quit();
      networkThread_->wait();
      networkThread_->deleteLater();
      networkThread_ =nullptr;
    }
  if(nullptr != client_){
      disconnect(client_, nullptr, this, nullptr);
      client_->deleteLater();
      client_ =nullptr;
  }
  if(nullptr != status_) status_->setParam(kLinkStatus, tr("Отсутствует"),app::OperationState_ERROR);
}

void Socksp2gStreamClient::slotNewIncoming(){
  emit newOutgoingMsg();
}




} // meteo
