#include "parser.h" 

#include <cross-commons/debug/tlog.h>

#include "methodpack.h"

namespace meteo {
namespace rpc {

Parser::Parser()
  : message_( new MethodPack )
{
}

Parser::~Parser()
{
  delete message_;
  message_ = nullptr;
}

bool Parser::searchStart()
{
  message_->clear();
  msgsize_ = 0;
  state_ = kStartSearch;
  if ( kStartSz > recvbuf_.size() ) {
    return false;
  }
  int indx = recvbuf_.indexOf(kRpcMsgStart);
  if ( -1 == indx ) {
    recvbuf_ = recvbuf_.right( kStartSz - 1 );
    return false;
  }
  recvbuf_ = recvbuf_.right( recvbuf_.size() - indx - kStartSz );
  state_ = kSizeSearch;
  return true;
}

bool Parser::searchSize()
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

bool Parser::recvMessage()
{
  if ( msgsize_ > recvbuf_.size() ) {
    return false;
  }
  if ( false == message_->setData(recvbuf_) ) {
    msgsize_ = 0;
    state_ = kStartSearch;
    return false;
  }
  recvbuf_ = recvbuf_.right( recvbuf_.size() - msgsize_ );
  state_ = kEndSearch;

  return true;
}

bool Parser::searchEnd()
{
  if ( kEndSz > recvbuf_.size() ) {
    return false;
  }
  int indx = recvbuf_.indexOf(kRpcMsgEnd);
  if ( -1 == indx ) {
    msgsize_ = 0;
    state_ = kStartSearch;
    return false;
  }
  recvbuf_ = recvbuf_.right( recvbuf_.size() - indx - kEndSz );
  msgsize_ = 0;
  state_ = kStartSearch;
  return true;
}

bool Parser::parseMessage( MethodPack* msg )
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

}
}
