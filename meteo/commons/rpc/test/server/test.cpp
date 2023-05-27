#include <stdlib.h>

#include <qcoreapplication.h>
#include <qhostaddress.h>

#include "test.h"

#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/waitloop.h>
#include <meteo/commons/rpc/transport.h>
#include <meteo/commons/rpc/channel.h>
#include <meteo/commons/rpc/tcpsocket.h>

namespace meteo {
namespace rpc {

ServerTcp::ServerTcp( QObject* p )
  : QTcpServer(p)
{
}

ServerTcp::~ServerTcp()
{
}

void ServerTcp::incomingConnection( qintptr descr )
{
  auto sock = new TcpSocket(descr);
  sock->moveToThread(sock);
  QObject::connect( sock, SIGNAL( handshakeEvent( QByteArray ) ), this, SLOT( slotHandshakeEvent( QByteArray ) ) );
  meteo::WaitLoop loop;
  QObject::connect( sock, SIGNAL( threadInitialized() ), &loop, SLOT( slotQuit() ) );
  sock->start();
  loop.wait(4000);
  if ( false == sock->waitForRecvHandshake(10*1000) ) {
    error_log << QObject::tr("Не получен идентификатор соденинения от клиента");
    sock->quit();
    if ( false == sock->wait( 1000*10 ) ) {
      sock->deleteLater();
    }
    else {
      delete sock;
    }
  }
  else {
    if ( false == unkconnections_.contains(sock) || 0 == unkconnections_[sock].size() ) {
      error_log << QObject::tr("Получен идентификатор соединения, но оно не добавлено в список соединений");
    }
  }
}

void ServerTcp::slotHandshakeEvent( QByteArray uuid )
{
  if ( 0 == uuid.size() ) {
    warning_log << QObject::tr("Не получен идентифкатор соединения");
  }
  TcpSocket* sock = qobject_cast<TcpSocket*>( sender() );
  if ( nullptr == sock ) {
    warning_log << QObject::tr("Не известный отправитель события.");
    return;
  }
  if ( true == unkconnections_.contains(sock) ) {
    error_log << QObject::tr("Непонятная ситуация. Получен идентификатор нового соединения. Но оно уже есть в списки соединений");
    return;
  }

  for ( auto it = unkconnections_.begin(), end = unkconnections_.end(); it != end; ++it ) {
    if ( it.value() == uuid ) {
      Transport* t = new Transport( sock->address(), const_cast<TcpSocket*>( it.key() ), sock, uuid );
      unkconnections_.remove( it.key() );
      Channel* ch = new Channel(t);
      channels_.append(ch);
      debug_log << QObject::tr("Есть соединение с клиентом по адресу = %1")
        .arg( ch->address() );
      break;
    }
  }
  if ( 100 < unkconnections_.size() ) {
    warning_log << QObject::tr("Превышено количество неопознанных соединений. Самое старое соединеение будет удалено.");
    TcpSocket* sock = unkconnections_.begin().key();
    delete sock;
    unkconnections_.take(sock);
  }

  unkconnections_.insert( sock, uuid ); 
}

}
}

int main( int argc, char* argv[] )
{
  debug_log << "MAIN THReAD =" << QThread::currentThreadId();
  QCoreApplication app( argc, argv );
  meteo::rpc::ServerTcp srv;
  bool res = srv.listen( QHostAddress::Any, 34567 );
  debug_log << "ReS =" << res;
  app.exec();
  return EXIT_SUCCESS;
}
