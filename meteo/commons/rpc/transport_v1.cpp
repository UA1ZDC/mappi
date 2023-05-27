#include "transport_v1.h"

#include <cross-commons/debug/tlog.h>

#include "tcpsocket_v1.h"

namespace meteo {
namespace rpc {

Transport_v1::Transport_v1()
  : Transport()
{
}

Transport_v1::Transport_v1( TcpSocket* sendsock, TcpSocket* recvsock )
  : Transport( sendsock, recvsock )
{
}

Transport_v1::~Transport_v1()
{
}

bool Transport_v1::connect( const QString& address )
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
  sendsock_ = new TcpSocket_v1(address);
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

}
}
