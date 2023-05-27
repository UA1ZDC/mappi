#ifndef METEO_COMMONS_APPMANAGER_SUBSCRIBECARD_H
#define METEO_COMMONS_APPMANAGER_SUBSCRIBECARD_H

#include <qglobal.h>
#include <qhash.h>
#include <qlist.h>

namespace rpc {

class Channel;

} // rpc

namespace google {
namespace protobuf {

class Closure;
class RpcController;

} // protobuf
} // google

//! Шаблон карточки запроса
template<class Request, class Response> class SubscribeCard
{
public:
  SubscribeCard(::rpc::Channel* ch = 0,
                      Request  req = 0,
                    Response  resp = 0,
          ::google::protobuf::Closure* d = 0,
    ::google::protobuf::RpcController* c = 0)
  {
    channel = ch;
    request = req;
    response = resp;
    done = d;
    controller = c;
  }

  Request  request;
  Response response;
  ::rpc::Channel* channel;
  ::google::protobuf::Closure* done;
  ::google::protobuf::RpcController* controller;
};

//!
template<class T1, class T2> uint qHash(const SubscribeCard<T1,T2>& card)
{ return qHash(QString("%1.%2").arg(quint64(card.done)).arg(quint64(card.channel))); }
//!
template<class T1, class T2> bool operator==(const SubscribeCard<T1,T2>& c1, const SubscribeCard<T1,T2>& c2)
{ return c1.done == c2.done && c1.channel == c2.channel; }
//!
template<class T1, class T2> bool operator!=(const SubscribeCard<T1,T2>& c1, const SubscribeCard<T1,T2>& c2)
{ return c1.done != c2.done && c1.channel != c2.channel; }

#endif // METEO_COMMONS_APPMANAGER_SUBSCRIBECARD_H
