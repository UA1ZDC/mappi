#include "tcpsocket_v1.h"

#include <quuid.h>

#include <cross-commons/debug/tlog.h>
#include <commons/funcs/mnio.h>
#include <meteo/commons/proto/rpc_v1.pb.h>

#include "parser_v1.h"
#include "methodpack.h"

namespace meteo {
namespace rpc {
TcpSocket_v1::TcpSocket_v1( const QString& addr )
  : TcpSocket(addr)
{
  delete parser_;
  parser_ = new Parser_v1();
}

TcpSocket_v1::TcpSocket_v1( const QString& addr, const QByteArray& uuid )
  : TcpSocket( addr, uuid )
{
  delete parser_;
  parser_ = new Parser_v1();
}

TcpSocket_v1::TcpSocket_v1( int32_t descr )
  : TcpSocket(descr)
{
  delete parser_;
  parser_ = new Parser_v1();
}

TcpSocket_v1::~TcpSocket_v1()
{
}

bool TcpSocket_v1::sendHandshakeInfo()
{
  setStatus(kSizeHandshing);
  if ( nullptr == sock_ || QAbstractSocket::ConnectedState != sock_->state() ) {
    error_log << QObject::tr("Нет соединения с сокетом");
    return false;
  }

  QByteArray arr;
  arr += uuid_;
  totalwritten_ += arr.size();
  qint64 writen = MnCommon::writeToSocket( sock_, arr );
  if ( arr.size() != writen ) {
    error_log << QObject::tr("Не удалось отправить идентификатор соединения");
    return false;
  }
  return true;
}

bool TcpSocket_v1::parseSizeHandshake()
{
  uuidsize_ = 38;
  return true;
}

bool TcpSocket_v1::parseIdentHandshake()
{
  return true;
}

bool TcpSocket_v1::parseHandshake()
{
  if ( uuidsize_ > recvbuf_.size() ) {
    return false;
  }
  QByteArray arr = recvbuf_.left( uuidsize_ );
  QUuid uid(arr);
  recvbuf_ = recvbuf_.mid(uuidsize_);
  if ( true == uid.isNull() ) {
    error_log << QObject::tr("Неверный формат Uuid");
    setStatus(kUnknown);
    return false;
  }
  if ( kServer == mode_ ) {
    uuid_ = uid.toByteArray();
  }
  return true;
}

bool TcpSocket_v1::writeMessage( const MethodPack& pack )
{
  ::rpc::Package oldpkg;
  oldpkg.set_id( pack.id().data(), pack.id().size() );
  oldpkg.set_stub( pack.stub() );
  oldpkg.set_failed( pack.failed() );
  oldpkg.set_error_text( pack.comment().data(), pack.comment().size() );
  oldpkg.set_service( pack.service().data(), pack.service().size() );
  oldpkg.set_method( pack.method().data(), pack.method().size() );
  oldpkg.set_message( pack.message().data(), pack.message().size() );

  std::string str;
  bool fl = oldpkg.SerializeToString(&str);
  if ( false == fl ) {
    error_log << QObject::tr("Ошибка. Не удалось сериализовать сообщение.");
    return false;
  }
  QByteArray arr;
  arr += RPCMSGSTART_v1;
  arr += '\0';
  QByteArray tmpbuf( sizeof(int64_t), 0 );
  int64_t arrsize = str.size();
  ::memcpy( tmpbuf.data(), &arrsize, sizeof(int64_t) );
  arr += tmpbuf;
  arr += QByteArray( str.data(), str.size() );
  arr += RPCMSGEND_v1;
  arr += '\0';
  totalwritten_ += arr.size();
  if ( arr.size() != MnCommon::writeToSocket( sock_, arr ) ) {
    error_log << QObject::tr("Не удалось отправить rpc-сообщение");
    return false;
  }
  return true;
}

bool TcpSocket_v1::startRecvThread()
{
  recvsock_ = new TcpSocket_v1( address_, uuid_ );
  recvsock_->moveToThread(recvsock_);
  recvsock_->start();
  recvsock_->connect();
  if ( kConnected != recvsock_->status() ) {
    recvsock_->exit(0);
    recvsock_->wait();
    delete recvsock_;
    recvsock_ = nullptr;
    return false;
  }
  return true;
}

}
}
