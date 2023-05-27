#ifndef METEO_COMMONS_RPC_TRANSPORT_H
#define METEO_COMMONS_RPC_TRANSPORT_H

#include <qobject.h>

namespace meteo {
namespace rpc {

class MethodPack;
class TcpSocket;

class Transport : public QObject
{
  Q_OBJECT
public:
  enum Protocol {
    Maslo             = 0,
    Novost            = 1
  };
  Transport();
  Transport( TcpSocket* sendsock, TcpSocket* recvsock );
  ~Transport();

  virtual int protocol() const { return Maslo; }

  virtual bool connect( const QString& address );
  bool isConnected() const ;

  const QString& address() const { return address_; }
  QByteArray uuid() const ;

  bool getMessage( MethodPack* pack );
  bool sendMessage( const MethodPack* pack);
  bool waitForAnswer( int32_t msec_timeout, const QByteArray& callid );

  uint8_t maclevel() const ;

protected:
  TcpSocket* sendsock_ = nullptr;
  TcpSocket* recvsock_ = nullptr;
  QString address_;
  QByteArray uuid_;
  bool disconnected_ = false;

//public slots:
public:
  Q_SLOT void slotThreadFinished();
  Q_SLOT void slotSockDisconnected( TcpSocket* sock );

  //  signals:
  Q_SIGNAL void connected();
  Q_SIGNAL void disconnected();
  Q_SIGNAL void messageReceived();
};

}
}

#endif
