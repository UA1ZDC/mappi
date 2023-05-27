#include "websocketransport.h"

#include <qwebsocket.h>
#include <qjsondocument.h>
#include <qjsonobject.h>

#include <cross-commons/debug/tlog.h>
#include <meteo/commons/global/global.h>

namespace meteo {
namespace internal {

WebCommunication::WebCommunication()
  : port_(40001)
{
}

WebCommunication::~WebCommunication()
{
  delete webserver_; webserver_ = nullptr;
}

bool WebCommunication::init()
{
  if ( nullptr != webserver_ && true == webserver_->isListening() ) {
    return true;
  }
  if ( nullptr == webserver_ ) {
    webserver_ = new QWebSocketServer( "meteowebserver", QWebSocketServer::NonSecureMode, this );
    QObject::connect( webserver_, SIGNAL( newConnection() ), this, SLOT( slotClientConnected() ) );
  }
  bool res = webserver_->listen( QHostAddress::LocalHost, port_);
  if ( false == res ) {
    error_log << QObject::tr("Не могу слушать порт = %1").arg(port_);
  }
  return res;
}

void WebCommunication::setPort( uint16_t port )
{
  port_ = port;
  if ( nullptr != webserver_ ) {
    webserver_->close();
  }
}

void WebCommunication::registerReceiver( const QString& id, QObject* obj )
{
  if ( true == receivers_.contains(id) ) {
    error_log << QObject::tr("Получатель для web-ресураса '%1' уже зарегистрирован");
    return;
  }
  QObject::connect( obj, SIGNAL( destroyed( QObject* ) ), this, SLOT( slotReceiverDestroyed( QObject* ) ) );
  QWebChannel* ch = new QWebChannel(this);
  ch->registerObject( id, obj );
  receivers_.insert( id, ch );
}

void WebCommunication::slotReceiverDestroyed( QObject* obj )
{
  auto it = receivers_.begin(), end = receivers_.end();
  QWebChannel* ch = nullptr;
  while ( it != end ) {
    QString id = it.key();
    ch = it.value();
    auto recvs = ch->registeredObjects();
    auto keyslist = recvs.keys(obj);
    if ( 0 == keyslist.size() ) {
      ch = nullptr;
      break;
    }
    else if ( 1 == keyslist.size() ) {
      break;
    }
    else {
      warning_log << QObject::tr("Непредвиденная ситуация. Обратитесь к администратору");
      break;
    }
    ++it;
  }
  if ( nullptr != ch ) {
    receivers_.erase(it);
    ch->deregisterObject(obj);
    delete ch;
  }
  else {
    warning_log << QObject::tr("Канал получателя не найден!");
  }
}

void WebCommunication::slotClientConnected()
{
  while ( true == webserver_->hasPendingConnections() ) {
    QWebSocket* sock = webserver_->nextPendingConnection();
    if ( nullptr == sock ) {
      warning_log << QObject::tr("Непредвиденная ситуация. Указатель на клиента == nullptr.");
      continue;
    }
    QString id = sock->resourceName();
    QStringList list = id.split("/");
    id = list.last();
    if ( false == receivers_.contains(id) ) {
      warning_log << QObject::tr("Обработчик событий для ресураса '%1' не зарегистрирован. Закрываем соединение с ресурсом '%1'.")
        .arg(id);
      sock->close();
      sock->deleteLater();
      continue;
    }
    QWebChannel* ch = receivers_[id];
    WebTransport* transport = new WebTransport(sock);
    ch->connectTo(transport);
  }
}

WebTransport::WebTransport( QWebSocket* sock )
  : sock_(sock)
{
  QObject::connect( sock, SIGNAL( disconnected() ), this, SLOT( deleteLater() ) );
  connect( sock_, SIGNAL( textMessageReceived( const QString& ) ), this, SLOT( textMessageReceived( const QString& ) ) );

}

WebTransport::~WebTransport()
{
  if ( nullptr != sock_ ) {
    sock_->deleteLater();
    sock_ = nullptr;
  }
}

void WebTransport::sendMessage( const QJsonObject& msg )
{
  QJsonDocument doc(msg);
  sock_->sendTextMessage( QString::fromUtf8( doc.toJson( QJsonDocument::Compact ) ) );
}

void WebTransport::textMessageReceived( const QString& msg )
{
  QJsonParseError error;
  QJsonDocument message = QJsonDocument::fromJson( msg.toUtf8(), &error);
  if ( true == error.error ) {
    error_log << QObject::tr("Не удалось превратить в JSON-документ сообщение '%1'. Ошибка = %2")
      .arg(msg).arg( error.errorString() );
    return;
   }
   else if (false == message.isObject() ) {
     error_log << QObject::tr("JSON не являетя объектом = '%1'")
       .arg(msg);
     return;
   }
   emit messageReceived( message.object(), this );
}

}
template<> ::meteo::internal::WebCommunication* meteo::WebCommunication::_instance = nullptr;
}
