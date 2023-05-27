#ifndef METEO_COMMONS_GLOBAL_WEBSOCKETRANSPORT_H
#define METEO_COMMONS_GLOBAL_WEBSOCKETRANSPORT_H

#include <qobject.h>
#include <qhash.h>
#include <qlist.h>
#include <qpair.h>
#include <qwebsocketserver.h>
#include <qwebchannel.h>
#include <qwebchannelabstracttransport.h>
#include <cross-commons/singleton/tsingleton.h>

class QWebSocket;

namespace meteo {
namespace internal {

class WebCommunication : public QObject
{
  Q_OBJECT
  public:
    WebCommunication();
    ~WebCommunication();

    void setPort( uint16_t port );
    bool init();

  public:
    void registerReceiver( const QString& id, QObject* receiver );

  private:
    QWebSocketServer* webserver_ = nullptr;
    QHash< QString, QWebChannel* > receivers_;
    uint16_t port_;


  private slots:
    void slotReceiverDestroyed( QObject* obj );
    void slotClientConnected();

  friend class TSingleton<WebCommunication>;
};

class WebTransport : public QWebChannelAbstractTransport
{
  Q_OBJECT
  public:
    WebTransport( QWebSocket* sock );
    ~WebTransport();

    void sendMessage( const QJsonObject& msg );

  private slots:
    void textMessageReceived( const QString& msg );

  private:
    QWebSocket* sock_ = nullptr;
};

}
using WebCommunication = TSingleton<internal::WebCommunication>;

}

#endif
