#include "parser_v1.h"

#include <cross-commons/debug/tlog.h>

#include "methodpack.h"

#include <meteo/commons/proto/rpc_v1.pb.h>

namespace meteo {
namespace rpc {

Parser_v1::Parser_v1()
  : Parser()
{
}

Parser_v1::~Parser_v1()
{
}

bool Parser_v1::parseMessage( MethodPack* msg )
{
  if ( nullptr == msg ) {
    error_log << QObject::tr("Нулевой указатель");
    return false;
  }
  switch (state_)
  {
    case Parser::kStartSearch:
      if ( false == searchStart() ) {
        return false;
      }
      if ( Parser::kSizeSearch != state_ ) {
        break;
      }
    case Parser::kSizeSearch:
      if ( false == searchSize() ) {
        return false;
      }
      if ( Parser::kMessageRecv!= state_ ) {
        break;
      }
    case Parser::kMessageRecv:
      if ( false == recvMessage() ) {
        return false;
      }
      if ( Parser::kEndSearch!= state_ ) {
        break;
      }
    case Parser::kEndSearch:
      if ( false == searchEnd() ) {
        return false;
      }
      msg->copyFrom(*message_);
      return true;
    default:
      break;
  }
  return false;
}

bool Parser_v1::searchStart()
{
  message_->clear();
  msgsize_ = 0;
  state_ = kStartSearch;
  int32_t startsz = sizeof(RPCMSGSTART_v1);
  if ( startsz > recvbuf_.size() ) {
    return false;
  }
  int indx = recvbuf_.indexOf(RPCMSGSTART_v1);
  if ( -1 == indx ) {
    recvbuf_ = recvbuf_.right( startsz - 1 );
    return false;
  }
  recvbuf_ = recvbuf_.right( recvbuf_.size() - indx - startsz );
  state_ = kSizeSearch;
  return true;
}

bool Parser_v1::searchSize()
{
  if ( kSizeSz > recvbuf_.size() ) {
    return false;
  }
  ::memcpy( &msgsize_, recvbuf_.data(), kSizeSz );
  if ( 0 >= msgsize_ ) {
    msgsize_ = 0;
    state_ = kStartSearch;
    error_log << "Unknown error. Size of message =" << msgsize_;
    return false;
  }
  recvbuf_ = recvbuf_.right( recvbuf_.size() - kSizeSz );
  state_ = kMessageRecv;
  return true;
}

bool Parser_v1::recvMessage()
{
  if ( msgsize_ > recvbuf_.size() ) {
    return false;
  }

  ::rpc::Package pkg;
  if ( false == pkg.ParseFromString( std::string( recvbuf_.data(), msgsize_ ) ) ) {
    state_ = kStartSearch;
    return false;
  }
  message_->setId( QByteArray( pkg.id().data(), pkg.id().size() ) );
  message_->setStub( pkg.stub() );
  message_->setFailed( pkg.failed() );
  message_->setComment( QByteArray( pkg.error_text().data(), pkg.error_text().size() ) );
  message_->setService( QByteArray( pkg.service().data(), pkg.service().size() ) );
  message_->setMethod( QByteArray( pkg.method().data(), pkg.method().size() ) );
  message_->setMessage( QByteArray( pkg.message().data(), pkg.message().size() ) );

  recvbuf_ = recvbuf_.right( recvbuf_.size() - msgsize_ );
  state_ = kEndSearch;

  return true;
}

bool Parser_v1::searchEnd()
{
  int32_t endsz = sizeof(RPCMSGEND_v1);
  if ( endsz > recvbuf_.size() ) {
    return false;
  }
  int indx = recvbuf_.indexOf(RPCMSGEND_v1);
  if ( -1 == indx ) {
    msgsize_ = 0;
    state_ = kStartSearch;
    return false;
  }
  recvbuf_ = recvbuf_.right( recvbuf_.size() - indx - endsz );
  msgsize_ = 0;
  state_ = kStartSearch;
  return true;
}

}
}
