#include "server.h"

#include <cross-commons/debug/tlog.h>
#include <meteo/commons/proto/spo_options.pb.h>

#include "transport.h"
#include "channel.h"

namespace meteo {
namespace rpc {

Server::Server( google::protobuf::Service* s, const QString& a )
  : QTcpServer(),
  service_(s),
  address_(a)
{
}

Server::~Server()
{
  shutdown();
}

bool Server::init()
{
  QString host;
  int32_t port;
  if ( false == TcpSocket::parseAddress( address_, &host, &port ) ) {
    error_log << QObject::tr("Неверный формат адреса хоста = %1. Формат = <адрес>:<порт>")
      .arg(address_);
    return false;
  }
  if ( false == QTcpServer::listen( QHostAddress::Any, port ) ) {
    error_log << QObject::tr("Ошибка. Не могу слушать порт = %1. Причина = %2")
      .arg(port)
      .arg( QTcpServer::errorString() );
    return false;
  }
  setMaxPendingConnections(100000000);
  return true;
}

void Server::shutdown()
{

  QTcpServer::close();
}

void Server::sendAnswer( Channel* ch, Controller* ctrl )
{
  if ( nullptr == ch ) {
    error_log << QObject::tr("Выполняется попытка отправить ответ по неустановленному каналу связи. Возврат");
//    debug_log << "STOP METHOD =" << ctrl->method()->full_name();
    delete ctrl->request();
    delete ctrl->response();
    if ( true == ctrl->method()->server_streaming()
      || true == ctrl->method()->options().GetExtension(spo::protobuf::multiresponse) ) {
      delete ctrl->closure();
    }
    delete ctrl;
    emit ansverSendedError();
    return;
  }
  if ( false == clients_.contains(ch) ) {
//    debug_log << "CLIENT NE NAYDEN! =" << ch;
    error_log << QObject::tr("Выполняется попытка отправить ответ по неустановленному каналу связи. Возврат");
//    debug_log << "STOP METHOD =" << ctrl->method()->full_name();
    ch->removeServerConroller(ctrl);
    if ( true == unconnectedclients_.contains(ch) ) {
      if ( 0 == ch->controllers_.size() ) {
//        debug_log << "UDALEN CANAL =" << ch;
        unconnectedclients_.removeAll(ch);
        delete ch;
      }
    }
    else {
      error_log << QObject::tr("Непредвиденная ситуация.");
    }
    emit ansverSendedError();
    return;
  }
  MethodPack pack;

  pack.setId( ctrl->callid() );
  pack.setStub(false);
  pack.setMethod( QByteArray( ctrl->method()->full_name().c_str() ) );
  pack.setService( QByteArray( ctrl->method()->service()->full_name().c_str() ) );
  google::protobuf::Message* resp = ctrl->response();
  std::string msg;
  if ( false == resp->IsInitialized() ) {
    QString resptype = QObject::tr("<не удалось определить тип>");
    const google::protobuf::Descriptor* descr = resp->GetDescriptor();
    if ( nullptr != descr ) {
      resptype = QString::fromStdString(descr->full_name());
    }
    warning_log << QObject::tr("Сообщение с запросом заполнено не полностью. Тип сообщения = %1, ошибка заполнения поля %2")
      .arg(resptype).arg( resp->InitializationErrorString().c_str() );
    resp->SerializePartialToString(&msg);
  }
  else {
    resp->SerializeToString(&msg);
  }
//  debug_log << "STOP METHOD =" << ctrl->method()->full_name();
  pack.setMessage( QByteArray( msg.data(), msg.size() ) );
  if ( false == ch->sendMessage(pack) ) {
    error_log << QObject::tr("Ошибка. Не удалось отправить ответ.");
    emit ansverSendedError();
  } else
  {
    emit ansverSendedOK();
  }

  if ( false == ctrl->method()->server_streaming()
    && false == ctrl->method()->options().GetExtension(spo::protobuf::multiresponse) ) {
    ch->removeServerConroller(ctrl);
  }
}

void Server::incomingConnection( qintptr descr )
{
  TcpSocket* sock = new TcpSocket( descr );
  unkconnections_.append( sock );
  QObject::connect( sock, &TcpSocket::connected, this, &Server::slotSocketConnected );
  QObject::connect( sock, &QThread::finished , this, &Server::slotSockFinished );
  sock->moveToThread(sock);
  sock->start();
}

void Server::slotSocketConnected( TcpSocket* sock )
{
  QByteArray uuid = sock->uuid();
  TcpSocket* sendsock = nullptr;
  TcpSocket* recvsock = nullptr;
  for ( auto s : unkconnections_ ) {
    if ( TcpSocket::kConnected != s->status() ) {
      continue;
    }
    if ( uuid == s->uuid() ) {
      if ( nullptr == recvsock ) {
        recvsock = s;
      }
      else if ( nullptr == sendsock ) {
        sendsock = s;
      }
    }
    if ( nullptr != sendsock && nullptr != recvsock ) {
      unkconnections_.removeAll(sendsock);
      unkconnections_.removeAll(recvsock);
      QObject::disconnect( sendsock, &QThread::finished , this, &Server::slotSockFinished );
      QObject::disconnect( recvsock, &QThread::finished , this, &Server::slotSockFinished );

      Transport* t = new Transport( sendsock, recvsock );
      Channel* ch = Channel::create( this, t );
      QObject::connect( ch, &Channel::connected, this, &Server::slotClientConnected );
      QObject::connect( ch, &Channel::disconnected, this, &Server::slotClientDisconnected );
      QObject::connect( ch, &Channel::clientSubscribed, this, &Server::clientSubscribed );
      QObject::connect( ch, &Channel::clientUnsubscribed, this, &Server::clientUnsubscribed );

//      debug_log << "PODKLUCHILSYA2 =" << ch;

      clients_.append(ch);
      emit clientConnected(ch);
      ch->slotMessageReceived();
      break;
    }
  }
}

void Server::slotClientConnected()
{
  Channel* ch = qobject_cast<Channel*>( sender() );
  if ( nullptr == ch ) {
    error_log << QObject::tr("Непредвиденная ситуация. Неопознан отправитель сигнала '%1'")
      .arg("clientConnected");
  }
  else {
    emit clientConnected(ch);
  }
//  debug_log << "PODKLUCHILSYA =" << ch;
}

void Server::slotClientDisconnected()
{
  Channel* ch = qobject_cast<Channel*>( sender() );
  if ( nullptr == ch ) {
    error_log << QObject::tr("Непредвиденная ситуация. Неопознан отправитель сигнала '%1'")
      .arg("clientDisconnected");
  }
  else {
    emit clientDisconnected(ch);
    if ( 0 != ch->controllers_.size() ) {
      clients_.removeAll(ch);
      if ( false == unconnectedclients_.contains(ch) ) {
        unconnectedclients_.append(ch);
      }
    }
    else {
      clients_.removeAll(ch);
//      debug_log << "UDALEN KANAL =" << ch;
      delete ch;
    }
  }
}

void Server::slotSockFinished()
{
  TcpSocket* sock = qobject_cast<TcpSocket*>( sender() );
  unkconnections_.removeAll(sock);
  QObject::disconnect( sock );
  sock->deleteLater();
}

}
}
