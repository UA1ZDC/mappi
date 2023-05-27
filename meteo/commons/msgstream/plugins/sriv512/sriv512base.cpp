#include "sriv512base.h"

#include <cstring>

#include <cross-commons/debug/tlog.h>

#include <meteo/commons/msgparser/common.h>
#include <meteo/commons/msgstream/plugins/sriv512/sriv512.h>
#include <commons/funcs/mnio.h>
#include <commons/compresslib/mcompress.h>

namespace meteo {
const int kResendInterval = 60*10;
const int SERV_PACKET_SIZE = int(sizeof(sriv512::ServicePackData));

Sriv512Base::Sriv512Base(const msgstream::Options& opt,
                         TSList<tlg::MessageNew>* outgoing,
                         TSList<tlg::MessageNew>* incoming,
                         AppStatusThread* status, QObject *p):
  QObject(p),
  port_( opt.sriv512().port() ),
  host_( opt.sriv512().host().c_str() ),
  socket_( nullptr ),
  status_( status ),
  incoming_( incoming ),
  outgoing_( outgoing )
{
  setMagic();
  std::memset(&magic_, '\0', 5);
  sendWithoutAck_ = opt.sriv512().window();
  id_ = QString::fromStdString(opt.id());
  num_ = 0;
  kChunk = SERV_PACKET_SIZE;
  state_ = sriv512::kStart;
  //reconnectTimeout_  = opt.sriv512().ack_waiting()/2;
  receiveTimeout_    = opt.sriv512().ack_waiting()*0.25*1000;
  reconnectTimeout_  = 5*1000;
  resendTimeout_ = 61*10*1000;
  rr_timeout_ = receiveTimeout_*0.5;
  setOptimCompress(opt.sriv512().compress_optim());

  //initTimers();
}

Sriv512Base::~Sriv512Base()
{

  deleteTimers();
  deleteSocket();
  if(nullptr != status_) status_->setParam(kLinkStatus, QObject::tr("Отсутствует"),app::OperationState_ERROR);
}

void Sriv512Base::deleteTimers()
{
    if ( nullptr != workTimer_ ) {
        disconnect(workTimer_, nullptr, this, nullptr);
        workTimer_->deleteLater();
        workTimer_ = nullptr;
      }
    if ( nullptr != reconnectTimer_ ) {
        disconnect(reconnectTimer_, nullptr, this, nullptr);
        reconnectTimer_->deleteLater();
        reconnectTimer_ = nullptr;
      }
    if ( nullptr != resend_timer_ ) {
        disconnect(resend_timer_, nullptr, this, nullptr);
        resend_timer_->deleteLater();
        resend_timer_ = nullptr;
      }
    if ( nullptr != rr_timer_ ) {
        disconnect(rr_timer_, nullptr, this, nullptr);
        rr_timer_->deleteLater();
        rr_timer_ = nullptr;
      }
}

void Sriv512Base::slotResendTimeout()
{
 if(nullptr != resend_timer_) resend_timer_->stop();
  tryResendMsg();
 if(nullptr != resend_timer_) resend_timer_->start();
}


void Sriv512Base::slotReadyRead()
{
  if(nullptr != rr_timer_) rr_timer_->stop();
  if(nullptr != workTimer_) workTimer_->stop();
  readyRead();
  sendProcess();
  if(nullptr != rr_timer_) rr_timer_->start();
  if(nullptr != workTimer_) workTimer_->start();
}


void Sriv512Base::slotNewMessage()
{
  if(nullptr != workTimer_) workTimer_->stop();
  sendProcess();
  readyRead();
  if(nullptr != workTimer_) workTimer_->start();
}

void Sriv512Base::initTimers()
{
  deleteTimers();
  workTimer_ = new QTimer(this);
  workTimer_->setInterval(receiveTimeout_);
  connect( workTimer_, SIGNAL(timeout()), SLOT(slotReceiveTimeout()) );

  reconnectTimer_ = new QTimer(this);
  reconnectTimer_->setInterval(reconnectTimeout_);
  connect( reconnectTimer_, SIGNAL(timeout()), SLOT(slotReconnectTimeout()) );

  resend_timer_ = new QTimer(this);
  resend_timer_->setInterval(resendTimeout_);
  connect( resend_timer_, SIGNAL(timeout()), SLOT(slotResendTimeout()) );
  resend_timer_->start();

  rr_timer_ = new QTimer(this);
  rr_timer_->setInterval(rr_timeout_);
  connect( rr_timer_, SIGNAL(timeout()), SLOT(slotSendRR()) );
  rr_timer_->start();
}


void Sriv512Base::deleteSocket()
{
  if ( nullptr != socket_ ) {
    if ( QAbstractSocket::UnconnectedState != socket_->state() ) {
      socket_->disconnectFromHost();
      if ( QAbstractSocket::UnconnectedState != socket_->state()
           &&!socket_->waitForDisconnected()) {
        if ( nullptr != socket_ ) socket_->abort();
      }
    }
    if ( nullptr != socket_ ) {
      QObject::disconnect(socket_, nullptr, nullptr, nullptr);
      socket_->abort();
      socket_->deleteLater();
      socket_ = nullptr;
    }
  }
}

void Sriv512Base::readyRead()
{
  qint64 rsz = 0;
  do{
      QByteArray incomingBuffer;
      if ( false == checkSocket() ) { return; }
      rsz = MnCommon::readFromSocket(socket_, &incomingBuffer,kChunk);
      emit signalBytesRecv(rsz);
      appendData(incomingBuffer);
      while (receiveProcess()) { }
    }while ( rsz > 0 );

  if ( -1 == rsz ) {
      debug_log << QObject::tr("Ошибка при чтении данных из сокета.");
      return;
    }
  if ( sriv512::kError == state_ ) {
      state_ = sriv512::kStart;
    }
}


void Sriv512Base::setMagic()
{
  QByteArray magic(1, 'U');
  magic += QString::number(port_).right(4).rightJustified(4, '0', true).toUtf8();
  std::memcpy(&magic_, magic.data(), 5);
}

bool Sriv512Base::writeEndPacket(const sriv512::ServicePacket &end)
{
  sriv512::ServicePacket ack = end;
  ack.setType(sriv512::kTcpIpAck);
  const QByteArray &ba = ack.toNet();
  if ( false == checkSocket() ) { return false; }
  qint64 wsz = MnCommon::writeToSocket(socket_,ba);
  emit signalBytesSended(wsz);
  if ( ba.size() != wsz ) {
      debug_log << QObject::tr("Не удалось отправить служебный пакет TCP_IP_ACK.");
      return false;
    }
  emit signalConfirmSended(1);
  tlg::MessageNew* msg = info_.mutableMsg();
  msg->mutable_metainfo()->set_from(id_.toStdString());
  return true;
}

bool Sriv512Base::getStartPacket()
{
  if(raw_.size() < SERV_PACKET_SIZE){
      state_ = sriv512::kStart;
      return false;
    }
  state_ = start_.setPacket(&raw_);
  removeData(SERV_PACKET_SIZE);
  if(sriv512::kInfo == state_){
      isWriten = false;
      return true;
    }
  return true; //???? return false;
}


bool Sriv512Base::getInfoPacket()
{
  if(int(start_.data().len + start_.data().info_len) > raw_.size()){
      state_ = sriv512::kInfo;
      return false;
    }

  bool res = info_.setPacket(&start_, &raw_);
  removeData(start_.data().info_len+start_.data().len);
  if(true == res){
      state_ = sriv512::kEnd;
      if(nullptr != incoming_){
          info_.mutableMsg()->mutable_metainfo()->set_from(id_.toStdString());
          incoming_->append(info_.msg());
          emit signalMessagesRecv(1);
        } else {
          debug_log<<QObject::tr("Обработчик не установлен!");
        }
      return true;
    } else {
      state_ = sriv512::kInfo;
      return false;
    }
  state_ = sriv512::kStart;
  return true;
}

bool Sriv512Base::getEndPacket()
{
  if(raw_.size() < SERV_PACKET_SIZE){
      state_ = sriv512::kEnd;
      return false;
    }
  sriv512::ServicePacket end;
  state_ = end.setPacket(&raw_);
  removeData(SERV_PACKET_SIZE);
  if(sriv512::kEnd != state_
     ||false == start_.match(end)){
      state_ = sriv512::kStart;
      return true;
    }
  if(false == writeEndPacket(end)){
      state_ = sriv512::kError;
      return false;
    }
  state_ = sriv512::kStart;
  return true;
}

bool Sriv512Base::processAck(const sriv512::ServicePacket& ack)
{

  QMapIterator<quint16, sriv512::SendBox> i(sentList_);
  while (i.hasNext()) {
      i.next();
      if ( i.value().start.match(ack)) {
          emit signalConfirmRecv(sentList_.remove(i.key()));

          //debug_log << "podtverdil" << i.value().crc<<sentList_.size();
          break;
        }

    }
  state_ = sriv512::kStart;
  return true;
}


bool Sriv512Base::receiveProcess()
{
  bool res = false;
  switch ( state_ ) {
    case sriv512::kError:
      stop();
     if(nullptr != reconnectTimer_) reconnectTimer_->start();
      return false;
      break;
    case sriv512::kStart:
      return getStartPacket();
      break;
    case sriv512::kInfo:
      return getInfoPacket();
      break;
    case sriv512::kEnd:
      if(true == (res = getEndPacket()))
        {isWriten = true;}
      return res;
      break;
    case sriv512::kAck:
      return processAck(start_);
      break;
    case sriv512::kRR:
      sendProcess();
      state_ = sriv512::kStart;
      return true;
      break;
    }
  return true;
}

void Sriv512Base::slotSendRR()
{
  if(nullptr != rr_timer_) rr_timer_->stop();
  if(false == isWriten ){
      if(nullptr != rr_timer_) rr_timer_->start();
      return;
    }
  if ( false == checkSocket() ) { return; }
  const QByteArray &ba = sriv512::ServicePacket::createRR().toNet();
  qint64 wsz = MnCommon::writeToSocket(socket_,ba);
  emit signalBytesSended(wsz);
  if ( ba.size() != wsz ) {
      debug_log << QObject::tr("Не удалось отправить служебный пакет TCP_IP_RR");
    }
  if(nullptr != rr_timer_) rr_timer_->start();
}

void Sriv512Base::sendProcess()
{
  if ( 0 == outgoing_ ) { return; }
  if(false == isWriten ){ return; }
  if ( !checkSocket() ) { return; }
  compress::Compress compressor;

  while ( outgoing_->size() > 0 && sentList_.size() < sendWithoutAck_ ) {

      sriv512::PackType t = sriv512::kTcpIpData;

      tlg::MessageNew msg = outgoing_->first();
      QByteArray rawTlg = tlg::raw2tlg(msg);
      if ( optimCompress() ) {
          //debug_log << "hetr";
          t = sriv512::kTcpIpDataZ;
          QByteArray ba = compressor.compress(rawTlg);
          double k = double(ba.size()) / rawTlg.size();
          if ( k < 0.7 ) {
              rawTlg = ba;
              t = sriv512::kTcpIpDataZC;
            }
        }

      if ( rawTlg.size() > (sriv512::kMaxLen) ) {
          warning_log << QObject::tr("Невозможно передать телеграмму. Максимальный размер %1 Кбайт.").arg(sriv512::kMaxLen/1024);
          outgoing_->removeFirst();
          continue;
        }

      sriv512::ServicePacket pack(t, magic_);
      pack.setNum(++num_);
      pack.setLen(rawTlg.size());
      pack.setInfoLen(int(sizeof(sriv512::InfoPackHead)));

      QByteArray ba = pack.toNet();
      sriv512::SendBox box;
      box.start = pack;
      sriv512::InfoPacket info = sriv512::InfoPacket::fromMessage(msg);
      ba += info.toNet();
      ba += rawTlg;
      box.info = info;
      pack.setType(sriv512::kTcpIpEnd);
      ba += pack.toNet();
      if ( false == checkSocket() ) { return; }
      qint64 wsz = MnCommon::writeToSocket(socket_,ba);
      emit signalBytesSended(wsz);
      if ( ba.size() != wsz ) {
          debug_log << QObject::tr("Не удалось отправить информационный пакет.");
          outgoing_->removeFirst();
          return;
        }
      emit signalMessagesSended(1);
      box.end = pack;
      outgoing_->removeFirst();
      box.date_recv = QDateTime::currentDateTime();
      sentList_.insertMulti(num_,box);//todo надо бы ключ другой какой
    }
}

void Sriv512Base::writeBox(const sriv512::SendBox &box){
  QByteArray ba = box.start.toNet();
  ba += box.info.toNet();
  QByteArray rawTlg = tlg::raw2tlg(box.info.msg());
  ba += rawTlg;
  ba += box.end.toNet();
  if ( false == checkSocket() ) { return; }
  qint64 wsz = MnCommon::writeToSocket(socket_,ba);
  emit signalBytesSended(wsz);
  if ( ba.size() != wsz ) {
      debug_log << QObject::tr("Не удалось отправить информационный пакет.");
      return;
    }
}

void Sriv512Base::tryResendMsg() {
  //debug_log<<"tryResendMsg"<<sentList_.size();

  QMapIterator<quint16, sriv512::SendBox> i(sentList_);
  while (i.hasNext()) {
      i.next();
      int sec_to = i.value().date_recv.secsTo(QDateTime::currentDateTime());
      if(kResendInterval < sec_to ){
          outgoing_->append(i.value().info.msg());
          sentList_.remove(i.key());
        }
    }
}

void Sriv512Base::slotSocketError(QAbstractSocket::SocketError socketError)
{
  switch ( socketError ) {
    case QAbstractSocket::ConnectionRefusedError:
      // error_log << QObject::tr("Адрес недоступен");
      // break;
    case QAbstractSocket::RemoteHostClosedError:
      warning_log << QObject::tr("Абонент разорвал соединение");
      break;
    case QAbstractSocket::HostNotFoundError:
    case QAbstractSocket::NetworkError:
      error_log << QObject::tr("Адрес %1:%2 недоступен").arg(host_).arg(port_);
      break;
    case QAbstractSocket::SocketAccessError:
      debug_log << QObject::tr("Ошибка SocketAccessError");
      break;
    case QAbstractSocket::SocketResourceError:
    case QAbstractSocket::SocketTimeoutError:
    case QAbstractSocket::DatagramTooLargeError:
    case QAbstractSocket::AddressInUseError:
    case QAbstractSocket::SocketAddressNotAvailableError:
    case QAbstractSocket::UnsupportedSocketOperationError:
    case QAbstractSocket::UnfinishedSocketOperationError:
    case QAbstractSocket::ProxyAuthenticationRequiredError:
    case QAbstractSocket::SslHandshakeFailedError:
    case QAbstractSocket::ProxyConnectionRefusedError:
    case QAbstractSocket::ProxyConnectionClosedError:
    case QAbstractSocket::ProxyConnectionTimeoutError:
    case QAbstractSocket::ProxyNotFoundError:
    case QAbstractSocket::ProxyProtocolError:
    case QAbstractSocket::OperationError:
    case QAbstractSocket::SslInternalError:
    case QAbstractSocket::SslInvalidUserDataError:
      if(nullptr == socket_) return;
      debug_log << QObject::tr("Ошибка ")<< socket_->errorString()<<socketError;
      break;
    case QAbstractSocket::TemporaryError:
    case QAbstractSocket::UnknownSocketError:
      if(nullptr == socket_) return;
      debug_log << QObject::tr("Неизвестная ошибка сокета")<< socket_->errorString();
      break;
    }
  stop();
  if(nullptr != status_) status_->setParam(kLinkStatus, QObject::tr("Отсутствует"),app::OperationState_ERROR);
  if(nullptr != reconnectTimer_) reconnectTimer_->start();

}

bool Sriv512Base::checkSocket() const
{
  return (nullptr != socket_ &&
      socket_->isValid() &&
      socket_->state() == QAbstractSocket::ConnectedState );
}

void Sriv512Base::appendData(const QByteArray& data)
{
  raw_ += data;
}

void Sriv512Base::removeData(int size)
{
  raw_.remove(0, size);
}

void Sriv512Base::clearData()
{
  raw_.clear();
}


} // meteo
