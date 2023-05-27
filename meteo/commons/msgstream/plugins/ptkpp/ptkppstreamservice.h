#ifndef METEO_COMMONS_MSGSTREAM_PLUGINS_PTKPP_PTKPPSTREAMSERVICE_H
#define METEO_COMMONS_MSGSTREAM_PLUGINS_PTKPP_PTKPPSTREAMSERVICE_H

#include <meteo/commons/proto/ptkppstream.pb.h>

namespace meteo {

class PtkppStreamRecv;

} // meteo

namespace meteo {

class PtkppStreamService : public msgstream::PtkppStreamService
{
public:
  PtkppStreamService();

  void setReceiver(PtkppStreamRecv* receiver) { receiver_ = receiver; }

  virtual void GetSubscribeList(google::protobuf::RpcController* controller,
                                         const msgstream::Dummy* request,
                               msgstream::SubscribeListResponse* response,
                                      google::protobuf::Closure* done);

  virtual void UpdateSubscribe(google::protobuf::RpcController* controller,
                                const msgstream::SubscribeData* request,
                                      msgstream::SubscribeData* response,
                                     google::protobuf::Closure* done);

private:
  PtkppStreamRecv* receiver_ = nullptr;
};

} // meteo

#endif // METEO_COMMONS_MSGSTREAM_PLUGINS_PTKPP_PTKPPSTREAMSERVICE_H
