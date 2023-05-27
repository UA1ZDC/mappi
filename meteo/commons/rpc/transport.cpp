#include "transport.h"

#include <cross-commons/debug/tlog.h>

#include "tcpsocket.h"

namespace meteo {
namespace rpc {

Transport::Transport()
  : QObject()
{
}

Transport::Transport( TcpSocket* sendsock, TcpSocket* recvsock )
  : QObject(),
  sendsock_(sendsock),
  recvsock_(recvsock)
{
  address_ = sendsock_->address();
  QObject::connect( recvsock_, &TcpSocket::messageReceived, this, &Transport::messageReceived );
  QObject::connect( sendsock_, &TcpSocket::messageReceived, this, &Transport::messageReceived );
  QObject::connect( recvsock_, &TcpSocket::disconnected, this, &Transport::slotSockDisconnected );
  QObject::connect( sendsock_, &TcpSocket::disconnected, this, &Transport::slotSockDisconnected );
  QObject::connect( recvsock_, &QThread::finished, this, &Transport::slotThreadFinished );
  QObject::connect( sendsock_, &QThread::finished, this, &Transport::slotThreadFinished );
}

Transport::~Transport()
{
//  debug_log << "MNE KYRDYK =" << this;
  if ( nullptr != recvsock_ ) {
    recvsock_->quitThread();
    if ( true == recvsock_->isRunning() ) {
      recvsock_->wait();
    }
    delete recvsock_;
    recvsock_ = nullptr;
  }
  if ( nullptr != sendsock_ ) {
    sendsock_->quitThread();
    if ( true == sendsock_->isRunning() ) {
      sendsock_->wait();
    }
    delete sendsock_;
    sendsock_ = nullptr;
  }
}

bool Transport::connect( const QString& address )
{
  address_ = address;
  if ( true == isConnected() ) {
    return true;
  }
  if ( nullptr != sendsock_ ) {
    sendsock_->quitThread(); sendsock_ = nullptr;
  }
  if ( nullptr != recvsock_ ) {
    recvsock_->quitThread(); recvsock_ = nullptr;
  }
  sendsock_ = new TcpSocket(address);
  sendsock_->moveToThread(sendsock_);
  sendsock_->start();
  QObject::connect( sendsock_, &TcpSocket::messageReceived, this, &Transport::messageReceived );
  QObject::connect( sendsock_, &TcpSocket::disconnected, this, &Transport::slotSockDisconnected );
  QObject::connect( sendsock_, &QThread::finished, this, &Transport::slotThreadFinished );
  recvsock_ = sendsock_->connect();
  if ( nullptr == recvsock_ ) {
    if ( nullptr != sendsock_ ) {
      sendsock_->quitThread();
      sendsock_ = nullptr;
    }
    return false;
  }
  uuid_ = sendsock_->uuid();
  QObject::connect( recvsock_, &TcpSocket::messageReceived, this, &Transport::messageReceived );
  QObject::connect( recvsock_, &TcpSocket::disconnected, this, &Transport::slotSockDisconnected );
  QObject::connect( recvsock_, &QThread::finished, this, &Transport::slotThreadFinished );
  return true;
}

bool Transport::isConnected() const
{
  if ( nullptr == sendsock_ ) {
    return false; 
  }
  if ( TcpSocket::kConnected != sendsock_->status() ) {
    return false;
  }
  if ( nullptr == recvsock_ ) {
    return false; 
  }
  if ( TcpSocket::kConnected != recvsock_->status() ) {
    return false;
  }
  return true;
}

QByteArray Transport::uuid() const
{
  if ( nullptr == sendsock_ ) {
    return QByteArray();
  }
  return sendsock_->uuid();
}

bool Transport::getMessage( MethodPack* pack )
{
  if ( nullptr == recvsock_ ) {
    return false;
  }
  return recvsock_->getMessage(pack);
}

bool Transport::sendMessage( const MethodPack* pack )
{
  if ( false == isConnected() ) {
    error_log << QObject::tr("Нет соединения с сервисом");
    return false;
  }
  if ( nullptr == sendsock_ ) {
    return false;
  }
  return sendsock_->sendMessage(pack);
}

bool Transport::waitForAnswer( int32_t msec_timeout, const QByteArray& callid )
{
  return sendsock_->waitForAnswer(msec_timeout, callid );
}

uint8_t Transport::maclevel() const
{
  if ( false == isConnected() ) {
    return 0;
  }
  uint8_t lvl = 0;
  if ( nullptr != sendsock_ ) {
    lvl = sendsock_->maclevel();
  }
  if ( nullptr != recvsock_ && lvl < recvsock_->maclevel() ) {
    lvl = recvsock_->maclevel();
  }
  return lvl;
}

void Transport::slotThreadFinished()
{
  TcpSocket* sock = qobject_cast<TcpSocket*>( sender() );
  if ( nullptr == sock ) {
    error_log << QObject::tr("Не удалось преобразовать отправителя сигнала к типу TcpSocket. Непредвиденная ситуация.");
    return;
  }
  if ( sock == sendsock_ ) {
    sendsock_ = nullptr;
  }
  else if ( sock == recvsock_ ) {
    recvsock_ = nullptr;
  }
  QObject::disconnect(sock);
  sock->wait();
  delete sock;
}

void Transport::slotSockDisconnected( TcpSocket* sock )
{
  if ( nullptr == sock ) {
    error_log << QObject::tr("Непредвиденная ситуация");
    return;
  }
  if ( true == sock->isRunning() ) {
    sock->quitThread();
  }
  if ( true == disconnected_ ) {
    return;
  }
  disconnected_ = true;
  emit disconnected();
//  debug_log << "DISCONNECT TRANSPORT =" << this;
}

}
}
