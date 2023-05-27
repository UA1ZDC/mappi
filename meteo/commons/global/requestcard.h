#ifndef METEO_NOVOST_GLOBAL_REQUESTCARD_H
#define METEO_NOVOST_GLOBAL_REQUESTCARD_H

#include <qglobal.h>
#include <qhash.h>

namespace google {
namespace protobuf {
class Closure;
class RpcController;
} // protobuf
} // google

//! Шаблон карточки запроса
template<class Request, class Response> class RequestCard
{
public:
  RequestCard(::google::protobuf::RpcController* c = 0,
                                        Request* req = 0,
                                       Response* resp = 0,
                    ::google::protobuf::Closure* d = 0)
  {
    request = req;
    response = resp;
    done = d;
    controller = c;
  }

  Request* request;
  Response* response;
  ::google::protobuf::Closure* done;
  ::google::protobuf::RpcController* controller;
};

//!
template<class T1, class T2> uint qHash(const RequestCard<T1,T2>& card) { return qHash(quint64(card.done)); }
//!
template<class T1, class T2> bool operator==(const RequestCard<T1,T2>& c1, const RequestCard<T1,T2>& c2) { return c1.done == c2.done; }
//!
template<class T1, class T2> bool operator!=(const RequestCard<T1,T2>& c1, const RequestCard<T1,T2>& c2) { return c1.done != c2.done; }


#endif // METEO_NOVOST_GLOBAL_REQUESTCARD_H
