#include "sriv512server.h"

#include <QtNetwork/QtNetwork>

#include <cross-commons/debug/tlog.h>
#include <meteo/commons/msgparser/msgmetainfo.h>


namespace meteo {


Sriv512Server::Sriv512Server(AppStatusThread* status, StreamModule* p)
  :QTcpServer(p),
    stream_module_(p),
    status_ (status)
{

}

Sriv512Server::~Sriv512Server()
{
  slotStreamFinihsed();
}

void Sriv512Server::init(const msgstream::Options& options,
                         TSList<tlg::MessageNew>* outgoing,
                         TSList<tlg::MessageNew>* incoming){
  opt_ = options;
  incoming_ = incoming;
  outgoing_ = outgoing;
}

void Sriv512Server::incomingConnection(qintptr socketDescriptor)
{
  if ( nullptr != sender_) {
    QTcpSocket tcpSocket;
    if  ( !tcpSocket.setSocketDescriptor(socketDescriptor) ) {
      error_log <<tcpSocket.error();
    }
    else {
      warning_log << tr("Попытка второго подключения с %1:%2").arg(tcpSocket.peerAddress().toString())
        .arg(tcpSocket.peerPort());
      tcpSocket.abort();
    }
    return;
  }
  QObject::disconnect(stream_module_, nullptr, nullptr,nullptr);

  slotStreamFinihsed();
  networkThread_ = new QThread;
  sender_ = new Sriv512Sender(socketDescriptor,opt_,outgoing_, incoming_, status_);
  sender_->moveToThread( networkThread_ );
  connect( networkThread_, SIGNAL(started()), sender_, SLOT(slotStart()) );
  connect( networkThread_, SIGNAL(finished()), this, SLOT(slotStreamFinihsed()) );

  QObject::connect(sender_, SIGNAL(finished()), this, SLOT(slotStreamFinihsed()));
  QObject::connect(stream_module_, SIGNAL(newOutgoingMsg()), sender_, SLOT(slotNewMessage()));
  QObject::connect(sender_, SIGNAL(signalMessagesRecv( qint64 )), stream_module_, SLOT(slotMsgReceived(qint64)));
  QObject::connect(sender_, SIGNAL(signalBytesRecv( qint64 )), stream_module_, SLOT( slotSizeReceived(qint64)));
  QObject::connect(sender_, SIGNAL(signalBytesSended( qint64 )), stream_module_, SLOT(slotSizeSended(qint64)));
  QObject::connect(sender_, SIGNAL(signalMessagesSended( qint64 )), stream_module_, SLOT( slotMsgSended(qint64)));
  QObject::connect(sender_, SIGNAL(signalConfirmSended( qint64 )), stream_module_, SLOT(slotConfirmSended(qint64)));
  QObject::connect(sender_, SIGNAL(signalConfirmRecv( qint64 )), stream_module_, SLOT( slotConfirmReceived(qint64)));
  networkThread_->start();
  if(nullptr != status_) status_->setParam(kLinkStatus, tr("Установлено"));
}


void Sriv512Server::slotStreamFinihsed(){
  if(nullptr != networkThread_){
      disconnect(networkThread_, nullptr, this, nullptr);
      networkThread_->quit();
      networkThread_->wait();
      networkThread_->deleteLater();
      networkThread_ =nullptr;
    }
  if(nullptr != sender_){
      disconnect(sender_, nullptr, this, nullptr);
      sender_->deleteLater();
      sender_ =nullptr;
  }

  if(nullptr != status_) status_->setParam(kLinkStatus, tr("Отсутствует"),app::OperationState_ERROR);
}

}
