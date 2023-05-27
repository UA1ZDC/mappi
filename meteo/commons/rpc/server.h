#ifndef METEO_COMMONS_RPC_SERVER_H
#define METEO_COMMONS_RPC_SERVER_H

#include <qtcpserver.h>

#include <meteo/commons/rpc/tcpsocket.h>

namespace google {
namespace protobuf {
class Service;
class RpcChannel;
class Closure;
}
}

namespace meteo {
namespace rpc {

class Channel;
class MethodPack;
class Controller;

class Server : public QTcpServer
{
  Q_OBJECT
  public:
    Server( google::protobuf::Service* service, const QString& address );
    ~Server();

    bool init();
    void shutdown();

    google::protobuf::Service* service() const { return service_; }

    void sendAnswer( rpc::Channel* ch, Controller* ctrl );
    const QString& address() const { return address_; }

  private:
    google::protobuf::Service* service_;
    QString address_;
    QList<TcpSocket*> unkconnections_;
    QList<Channel*> clients_;
    QList<Channel*> unconnectedclients_;

  private:
    void incomingConnection( qintptr descr );

  private :
    Q_SLOT void slotSocketConnected( TcpSocket* sock );
    Q_SLOT void slotClientConnected();
    Q_SLOT void slotClientDisconnected();
    Q_SLOT void slotSockFinished();
  public:
    Q_SIGNAL void clientConnected( meteo::rpc::Channel* channel );
    Q_SIGNAL void clientDisconnected( meteo::rpc::Channel* channel );

    Q_SIGNAL void clientSubscribed( meteo::rpc::Controller* ctrl );
    Q_SIGNAL void clientUnsubscribed( meteo::rpc::Controller* ctrl );
    Q_SIGNAL void ansverSendedOK();
    Q_SIGNAL void ansverSendedError();
};

}
}

#endif
