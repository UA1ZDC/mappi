#ifndef METEO_COMMONS_RPC_TCPSOCKET_V1_H
#define METEO_COMMONS_RPC_TCPSOCKET_V1_H

#include <meteo/commons/rpc/tcpsocket.h>

namespace meteo {
namespace rpc {

class TcpSocket_v1 : public TcpSocket {
  Q_OBJECT
  public:
    explicit TcpSocket_v1( const QString& addr );
    explicit TcpSocket_v1( const QString& addr, const QByteArray& uuid );
    explicit TcpSocket_v1( int32_t descr );
    ~TcpSocket_v1();

  protected:
    bool sendHandshakeInfo();
    bool parseSizeHandshake();
    bool parseIdentHandshake();
    bool parseHandshake();
    bool writeMessage( const MethodPack& pack );
    bool startRecvThread();
};
}
}

#endif
