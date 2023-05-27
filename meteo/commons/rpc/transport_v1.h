#ifndef METEO_COMMONS_RPC_TRANSPORT_V1_H
#define METEO_COMMONS_RPC_TRANSPORT_V1_H

#include <meteo/commons/rpc/transport.h>

namespace meteo {
namespace rpc {

class Transport_v1 : public Transport
{
  Q_OBJECT
  public:
    Transport_v1();
    Transport_v1( TcpSocket* sendsock, TcpSocket* recvsock );
    ~Transport_v1();

    int protocol() const { return Transport::Novost; }
    bool connect( const QString& address );
};

}
}

#endif
