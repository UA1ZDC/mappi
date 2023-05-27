#ifndef TEST_H
#define TEST_H

#include <qbytearray.h>
#include <qmap.h>
#include <qtcpserver.h>

namespace meteo {
namespace rpc {

class TcpSocket;
class Channel;

class ServerTcp : public QTcpServer
{
  Q_OBJECT
  public:
    ServerTcp( QObject* parent = 0 );
    ~ServerTcp();

  protected:
    virtual void incomingConnection( qintptr descr ) override ;

  private:
    QMap<TcpSocket*, QByteArray > unkconnections_;
    QList<Channel*> channels_;

    private slots:
      void slotHandshakeEvent( QByteArray uuid );
};

}
}

#endif
