#include "ptkppstreamservice.h"

#include "ptkppstreamrecv.h"

namespace meteo {

PtkppStreamService::PtkppStreamService()
{
}

void PtkppStreamService::GetSubscribeList(google::protobuf::RpcController* controller,
                                                   const msgstream::Dummy* request,
                                         msgstream::SubscribeListResponse* response,
                                                google::protobuf::Closure* done)
{
  Q_UNUSED( controller );
  Q_UNUSED( request );

  if ( nullptr == receiver_ ) {
    done->Run();
    return;
  }

  QList<msgstream::SubscribeData> list = receiver_->subscribeList();
  for ( const msgstream::SubscribeData& s : list ) {
    response->add_subscribe()->CopyFrom(s);
  }
  done->Run();
}

void PtkppStreamService::UpdateSubscribe(google::protobuf::RpcController* controller,
                                          const msgstream::SubscribeData* request,
                                                msgstream::SubscribeData* response,
                                               google::protobuf::Closure* done)
{
  Q_UNUSED( controller );

  if ( nullptr == receiver_ ) {
    done->Run();
    return;
  }

  switch ( request->action() ) {
    case msgstream::SubscribeData::kRunAction:
      response->CopyFrom(receiver_->startSubscribe(*request));
      break;
    case msgstream::SubscribeData::kStopAction:
      response->CopyFrom(receiver_->stopSubscribe(*request));
      break;
    case msgstream::SubscribeData::kRemoveAction:
      response->CopyFrom(receiver_->removeSubscribe(*request));
      break;
    default:
      break;
  }

  done->Run();
}

} // meteo
