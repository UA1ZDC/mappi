#include "channel.h"

#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/descriptor.pb.h>

#include <cross-commons/debug/tlog.h>
#include <meteo/commons/proto/spo_options.pb.h>

#include "transport.h"
#include "transport_v1.h"
#include "server.h"
#include "controller.h"

namespace meteo {
namespace rpc {

Channel::Channel()
  : QObject()
{
}

Channel::~Channel()
{
  delete handler_; handler_ = nullptr;
  QObject::disconnect(transport_);
  while ( 0 != controllers_.size() ) {
    delete controllers_[0];
  }
  delete transport_; transport_ = nullptr;
}

int Channel::protocol() const
{
  if ( nullptr == transport_ ) {
    return -1;
  }
  return transport_->protocol();
}

Channel* Channel::connect( const QString& address )
{
  Channel* ch = new Channel();
  Transport* t = new Transport();
  QObject::connect( t, SIGNAL( connected() ), ch, SLOT( slotConnected() ) );
  QObject::connect( t, SIGNAL( disconnected() ), ch, SLOT( slotDisconnected() ) );
  QObject::connect( t, SIGNAL( messageReceived() ), ch, SLOT( slotMessageReceived() ) );
  if ( false == t->connect(address) ) {
    debug_log << QObject::tr("Не удалось установить соединение %1").arg(address);
    delete t;
    delete ch;
    return nullptr;
  }
  ch->transport_ = t;
  return ch;
}

Channel* Channel::connect_v1( const QString& address )
{
  Channel* ch = new Channel();
  Transport* t = new Transport_v1();
  QObject::connect( t, SIGNAL( connected() ), ch, SLOT( slotConnected() ) );
  QObject::connect( t, SIGNAL( disconnected() ), ch, SLOT( slotDisconnected() ) );
  QObject::connect( t, SIGNAL( messageReceived() ), ch, SLOT( slotMessageReceived() ) );
  if ( false == t->connect(address) ) {
    debug_log << QObject::tr("Не удалось установить соединение");
    delete t;
    delete ch;
    return nullptr;
  }
  ch->transport_ = t;
  return ch;
}

Channel* Channel::create( Server* server, Transport* t )
{
  Channel* ch = new Channel();
  ch->transport_ = t;
  ch->server_ = server;
  ch->handler_ = new ClientHandler(ch);
  QObject::connect( t, SIGNAL( connected() ), ch, SLOT( slotConnected() ) );
  QObject::connect( t, SIGNAL( disconnected() ), ch, SLOT( slotDisconnected() ) );
  QObject::connect( t, SIGNAL( messageReceived() ), ch, SLOT( slotMessageReceived() ) );
  return ch;
}

bool Channel::isConnected() const
{
  if ( nullptr == transport_ ) {
    return false;
  }
  return transport_->isConnected();
}

bool Channel::isServerSide() const
{
  return ( nullptr != server_ );
}

QString Channel::address() const
{
  if ( nullptr == transport_ ) {
    return QString();
  }
  return transport_->address();
}

QString Channel::clientId() const
{
  if ( nullptr == transport_ ) {
    return QString();
  }
  return transport_->uuid();
}

bool Channel::sendMessage( const MethodPack& pack )
{
  return transport_->sendMessage(&pack);
}

void Channel::addClientClosure( MethodClosureBase* closure )
{
  closures_.append(closure);
}

void Channel::removeClientClosure( MethodClosureBase* closure )
{
  closures_.removeAll(closure);
}

void Channel::removeServerConroller( Controller* ctrl )
{
  controllers_.removeAll(ctrl);
  delete ctrl->request();
  delete ctrl->response();
  if ( true == ctrl->method()->server_streaming()
    || true == ctrl->method()->options().GetExtension(spo::protobuf::multiresponse) ) {
    delete ctrl->closure();
  }
  delete ctrl;
}

uint8_t Channel::maclevel() const
{
  if ( nullptr == transport_ ) {
    return 0;
  }
  return transport_->maclevel();
}

void Channel::CallMethod(const google::protobuf::MethodDescriptor* m,
    google::protobuf::RpcController* c,
    const google::protobuf::Message* req,
    google::protobuf::Message* resp,
    google::protobuf::Closure* d)
{
  if ( nullptr == transport_ || false == transport_->isConnected() ) {
    error_log << QObject::tr("Соединение с сервером отсутствует. Запрос не будет отправлен.");
    return;
  }
  Controller* ctrl = static_cast<Controller*>(c);
  controllers_.append(ctrl);
  ctrl->setRequest(req);
  ctrl->setResponse(resp);
  ctrl->setMethod(m);
  ctrl->setService( QByteArray( m->service()->full_name().c_str() ) );
  ctrl->setClosure(d);

  MethodPack pack( ctrl->callid() );

  std::string msg;
  if ( false == req->IsInitialized() ) {
    QString reqtype = QObject::tr("<не удалось определить тип>");
    const google::protobuf::Descriptor* descr = req->GetDescriptor();
    if ( 0 != descr ) {
      reqtype = QString::fromStdString(descr->full_name());
    }
    warning_log << QObject::tr("Сообщение с запросом заполнено не полностью. Тип сообщения = %1, ошибка заполнения поля %2")
      .arg(reqtype).arg( req->InitializationErrorString().c_str() );
    req->SerializePartialToString(&msg);
  }
  else {
    req->SerializeToString(&msg);
  }
  pack.setMessage( QByteArray( msg.data(), msg.size() ) );
  pack.setStub(true);
  pack.setMethod( QByteArray( m->name().c_str() ) );
  pack.setService( ctrl->service() );

  bool fl = false;
  if ( 0 < msecs_timeout_ ) {
    fl = sendBlockedClientMessage(&pack);
  }
  else {
    fl = sendClientMessage(&pack);
  }
  if ( false == fl ) {
    ctrl->SetFailed( pack.comment().toStdString() );
    error_log << pack.comment();
    handleResponse(pack);
  }
}

bool Channel::sendClientMessage( MethodPack* pack )
{
  bool fl = transport_->sendMessage(pack);
  if ( false == fl ) {
    QString err = QObject::tr("Не удалось отправить запрос");
    pack->setFailed(true);
    pack->setComment( err.toUtf8() );
    return false;
  }
  return true;
}

bool Channel::sendBlockedClientMessage( MethodPack* pack )
{
  bool fl = transport_->sendMessage(pack);
  if ( false == fl ) {
    QString err = QObject::tr("Не удалось отправить запрос");
    pack->setFailed(true);
    pack->setComment( err.toUtf8() );
    return false;
  }
  fl = transport_->waitForAnswer(msecs_timeout_, pack->id() );
  if ( true == fl ) {
    slotMessageReceived();
  }
  else {
    QString err = QObject::tr("Превышено время ожидания ответа от сервиса. Метод = %1. Время ожидания = %2")
      .arg(pack->method().data())
      .arg(msecs_timeout_);
    pack->setFailed(true);
    pack->setComment( err.toUtf8() );
  }
  return fl;
}

ClientHandler* Channel::handlerFromController( google::protobuf::RpcController* ctrl )
{
  Controller* c = static_cast<Controller*>(ctrl);
  if ( nullptr == c ) {
    error_log << QObject::tr("Нулевой указатель на Controller");
    return nullptr;
  }
  if ( nullptr == c->channel() ) {
    error_log << QObject::tr("Нулевой указатель на Channel");
    return nullptr;
  }
  return c->channel()->handler();
}

void Channel::handleMessage( const MethodPack& pack )
{
  if ( true == pack.stub() ) {
    handleRequest(pack);
  }
  else {
    handleResponse(pack);
  }
}

void Channel::handleRequest( const MethodPack& pack )
{
  const google::protobuf::ServiceDescriptor* sdescr = server_->service()->GetDescriptor();
  if ( nullptr == sdescr ) {
    error_log << QObject::tr("Не удалось получить дескриптор сервиса");
    return;
  }
  const google::protobuf::MethodDescriptor* method = sdescr->FindMethodByName( std::string( pack.method().data(), pack.method().size() ) );
  if ( nullptr == method ) {
    error_log << QObject::tr("Не удалось получить дескриптор метода %1")
      .arg( QString::fromUtf8( pack.method() ) );
    return;
  }
//  debug_log << "METHOD DiLYA VYPOLNENIA =" << method->full_name();
  google::protobuf::Message* request = server_->service()->GetRequestPrototype(method).New();
  google::protobuf::Message* response = server_->service()->GetResponsePrototype(method).New();  
  bool fl = request->ParsePartialFromString( std::string( pack.message().data(), pack.message().size() ) );
  if ( false == fl || false == request->IsInitialized() ) {
    QString reqtype = QString::fromUtf8("<не удалось определить тип>");
    const google::protobuf::Descriptor* descr  = request->GetDescriptor();
    if ( nullptr != descr ) {
      reqtype = QString::fromStdString( descr->full_name() );
    }
    warning_log << QObject::tr("Запрос не полностью заполнен клиентом. Тип ответа = %1")
      .arg(reqtype);
  }

  Controller* ctrl = new Controller(this);
  ctrl->setCallid( pack.id() );
  ctrl->setRequest(request);
  ctrl->setResponse(response);
  ctrl->setMethod( method );
  ctrl->setService( pack.service() );
  google::protobuf::Closure* done = nullptr;
  if ( true == method->server_streaming()
    || true == method->options().GetExtension(spo::protobuf::multiresponse) ) {
    done = google::protobuf::NewPermanentCallback( server_, &Server::sendAnswer, this, ctrl );
  }
  else {
    done = google::protobuf::NewCallback( server_, &Server::sendAnswer, this, ctrl );
  }
  ctrl->setClosure(done);
  controllers_.append(ctrl);

  if ( true == method->server_streaming()
    || true == method->options().GetExtension(spo::protobuf::multiresponse) ) {

    emit clientSubscribed(ctrl);
  }
  server_->service()->CallMethod( method, ctrl, request, response, done );
}

void Channel::handleResponse( const MethodPack& pack )
{
  //debug_log << "OBRABOTKA RESPA/ packid =" << pack.id();
  Controller* ctrl = nullptr;
  for ( auto c : controllers_ ) {
    if ( pack.id() == c->callid() ) {
      ctrl = c;
      break;
    }
  }
  if ( nullptr == ctrl ) {
    //error_log << QObject::tr("Не найден получатель сообщения!");
    return;
  }
  google::protobuf::Closure* done = nullptr;
  if ( nullptr != ctrl->closure() ) {
    done = ctrl->closure();
  }
  if ( false == ctrl->method()->server_streaming()
    && false == ctrl->method()->options().GetExtension( spo::protobuf::multiresponse ) ) {
    ctrl->setClosure(nullptr);
    controllers_.removeAll(ctrl);
  }
  bool fl = ctrl->response()->ParsePartialFromString( std::string( pack.message().data(), pack.message().size() ) );
  if ( false == fl || false == ctrl->response()->IsInitialized() ) {
    QString anstype = QString::fromUtf8("<не удалось определить тип>");
    const google::protobuf::Descriptor* descr = ctrl->response()->GetDescriptor();
    if ( nullptr != descr ) {
      anstype = QString::fromStdString( descr->full_name() );
    }
    warning_log << QObject::tr("Ответ не полностью заполнен сервером. Тип ответа = %1. Ошибка = %2")
      .arg(anstype)
      .arg(ctrl->response()->InitializationErrorString().c_str());
  }
  //debug_log << "RESP dlya id =" << pack.id() << "poluchen. dlina respa =" << pack.message().size();
//  debug_log << "RESP@ =" << ctrl->response()->Utf8DebugString();
  if ( nullptr != done ) {
    done->Run();
  }
}

void Channel::setBlockParams( int32_t msecs_timeout, bool blocked )
{
  msecs_timeout_ = msecs_timeout;
  blocked_ = blocked;
}

void Channel::slotConnected()
{
  emit connected();
}

void Channel::slotDisconnected()
{
//  debug_log << "OTKLUCHILSYA =" << this << "HANDLER =" << handler_;
  if ( nullptr != server_ ) {
    for ( auto c : controllers_ ) {
      if ( true == c->method()->server_streaming()
        || true == c->method()->options().GetExtension(spo::protobuf::multiresponse) ) {
        emit clientUnsubscribed(c);
      }
    }
  }
  while ( 0 != closures_.size() ) {
    auto c = closures_.takeFirst();
    delete c;
  }
  emit disconnected();
}

void Channel::slotMessageReceived() {
//  debug_log << "SRABOTAL!";
  if ( false == isConnected() ) {
    error_log << QObject::tr("Нет соединения. Полученные сообщения не обрабатываются");
    return;
  }
  MethodPack pack;
  if ( true == isConnected() && true == transport_->getMessage(&pack) ) {
    handleMessage(pack);
  }
}

}
}
