#include "socksp2gbase.h"

#include <cstring>

#include <cross-commons/debug/tlog.h>

#include <meteo/commons/msgparser/common.h>
#include <meteo/commons/msgstream/plugins/socketspec2g/socksp2g.h>
#include <commons/funcs/mnio.h>
#include <commons/compresslib/mcompress.h>

namespace meteo {
const int kResendInterval = 60*10;

Socksp2gBase::Socksp2gBase(const msgstream::Options& opt,
                         TSList<tlg::MessageNew>* outgoing,
                         TSList<tlg::MessageNew>* incoming,
                         AppStatusThread* status, QObject *p):
  QObject(p),
  port_( opt.socksp2g().port() ),
  host_( opt.socksp2g().host().c_str() ),
  socket_( nullptr ),
  status_( status ),
  incoming_( incoming ),
  outgoing_( outgoing )
{
  setMagic();
  //std::memset(&magic_, '\0', 5);
  sendWithoutAck_ = opt.socksp2g().window();
  id_ = QString::fromStdString(opt.id());
  num_ = 0;
  kChunk = 1024;//socksp2g::SERV_PACKET_SIZE;
  state_ = socksp2g::kStart;
  //reconnectTimeout_  = opt.socksp2g().ack_waiting()/2;
  receiveTimeout_    = opt.socksp2g().ack_waiting()*0.25*1000;
  reconnectTimeout_  = 5*1000;
  resendTimeout_ = 61*10*1000;
  rr_timeout_ = receiveTimeout_*0.5;
  setOptimCompress(opt.socksp2g().compress_optim());
  if(opt.socksp2g().has_path())
  {
    info_.setOutDir(QString::fromStdString(opt.socksp2g().path()));
  }
  //initTimers();
}

Socksp2gBase::~Socksp2gBase()
{

  deleteTimers();
  deleteSocket();
  if(nullptr != status_) status_->setParam(kLinkStatus, QObject::tr("Отсутствует"),app::OperationState_ERROR);
}

void Socksp2gBase::deleteTimers()
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

void Socksp2gBase::slotResendTimeout()
{
  if(nullptr != resend_timer_) resend_timer_->stop();
  tryResendMsg();
  if(nullptr != resend_timer_) resend_timer_->start();
}


void Socksp2gBase::slotReadyRead()
{
  if(nullptr != rr_timer_) rr_timer_->stop();
  if(nullptr != workTimer_) workTimer_->stop();
  readyRead();
  sendProcess();
  if(nullptr != rr_timer_) rr_timer_->start();
  if(nullptr != workTimer_) workTimer_->start();
}


void Socksp2gBase::slotNewMessage()
{
  if(nullptr != workTimer_) workTimer_->stop();
  sendProcess();
  readyRead();
  if(nullptr != workTimer_) workTimer_->start();
}

void Socksp2gBase::initTimers()
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


void Socksp2gBase::deleteSocket()
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

void Socksp2gBase::readyRead()
{
  qint64 rsz = 0;
  do{
    QByteArray incomingBuffer;
    if ( false == checkSocket() ) { return; }
    rsz = MnCommon::readFromSocket(socket_, &incomingBuffer,kChunk);
    emit signalBytesRecv(rsz);
    appendData(incomingBuffer);
   // debug_log << incomingBuffer;
    while (receiveProcess()) { }
  }while ( rsz > 0 );

  if ( -1 == rsz ) {
    debug_log << QObject::tr("Ошибка при чтении данных из сокета.");
    return;
  }
  if ( socksp2g::kError == state_ ) {
    state_ = socksp2g::kStart;
  }
}


void Socksp2gBase::setMagic()
{
  QByteArray magic(1, 'U');
  magic += QString::number(port_).right(4).rightJustified(4, '0', true).toUtf8();
  std::memcpy(&magic_, magic.data(), sizeof(magic_));
}

bool Socksp2gBase::writeEndPacket(const socksp2g::ServicePacket &end)
{
  socksp2g::ServicePacket ack = end;
  ack.setType(socksp2g::kTcpIpAck);
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

bool Socksp2gBase::getStartPacket()
{
  if(raw_.size() < socksp2g::SERV_PACKET_SIZE){
    state_ = socksp2g::kStart;
    return false;
  }
  state_ = start_.setPacket(&raw_);
  removeData(socksp2g::SERV_PACKET_SIZE);
  if(socksp2g::kInfoFile == state_ ||
     socksp2g::kInfoMsg == state_){
    isWriten = false;
    //return true;
  }
  return true; //???? return false;
}


bool Socksp2gBase::getInfoPacket()
{
  if(int(start_.data().len + start_.data().info_len) > raw_.size())
  {
    return false;
  }

  int res = info_.setPacket(&start_, &raw_, optimCompress());
  removeData(start_.data().info_len+start_.data().len);
  if(-2 == res ){
    state_ = socksp2g::kEnd;
    return true;

  }
  if(0 == res){
    state_ = socksp2g::kEnd;
    if(nullptr != incoming_){
      info_.mutableMsg()->mutable_metainfo()->set_from(id_.toStdString());
      incoming_->append(info_.msg());
      emit signalMessagesRecv(1);
    }
    else
    {
      debug_log<<QObject::tr("Обработчик не установлен!");
    }
    return true;
  } else {
    return false;
  }
  state_ = socksp2g::kStart;
  return true;
}

bool Socksp2gBase::getEndPacket()
{
  if(raw_.size() < socksp2g::SERV_PACKET_SIZE){
    state_ = socksp2g::kEnd;
    return false;
  }
  socksp2g::ServicePacket end;
  state_ = end.setPacket(&raw_);
  removeData(socksp2g::SERV_PACKET_SIZE);
  if(socksp2g::kEnd != state_
     ||false == start_.match(end)){
    state_ = socksp2g::kStart;
    return true;
  }
  if(false == writeEndPacket(end)){
    state_ = socksp2g::kError;
    return false;
  }
  state_ = socksp2g::kStart;
  return true;
}

bool Socksp2gBase::processAck(const socksp2g::ServicePacket& ack)
{

  QMapIterator<quint16, socksp2g::SendBox> i(sentList_);
  while (i.hasNext()) {
    i.next();
    if ( i.value().start.match(ack)) {
      emit signalConfirmRecv(sentList_.remove(i.key()));

      //debug_log << "podtverdil" << i.value().crc<<sentList_.size();
      break;
    }

  }
  state_ = socksp2g::kStart;
  return true;
}


bool Socksp2gBase::receiveProcess()
{
  bool res = false;
  switch ( state_ ) {
    case socksp2g::kError:
      stop();
      if(nullptr != reconnectTimer_) reconnectTimer_->start();
    return false;
    break;
    case socksp2g::kStart:
    return getStartPacket();
    break;
    case socksp2g::kInfoFile:
      if(info_.outDir().isEmpty())
      {
        warning_log << QObject::tr("Необходимо задать путь для сохранения файлов. Файл не будет сохранен");
        state_ = socksp2g::kStart;
        return true;
      }
    case socksp2g::kInfoMsg:
    return getInfoPacket();
    break;
    case socksp2g::kEnd:
      if(true == (res = getEndPacket()))
      {isWriten = true;}
    return res;
    break;
    case socksp2g::kAck:
    return processAck(start_);
    break;
    case socksp2g::kRR:
      sendProcess();
      state_ = socksp2g::kStart;
    return true;
    break;
  }
  return true;
}

void Socksp2gBase::slotSendRR()
{
  if(nullptr != rr_timer_) rr_timer_->stop();
  if(false == isWriten ){
    if(nullptr != rr_timer_) rr_timer_->start();
    return;
  }
  if ( false == checkSocket() ) { return; }
  const QByteArray &ba = socksp2g::ServicePacket::createRR(magic_).toNet();
  qint64 wsz = MnCommon::writeToSocket(socket_,ba);
  emit signalBytesSended(wsz);
  if ( ba.size() != wsz ) {
    debug_log << QObject::tr("Не удалось отправить служебный пакет TCP_IP_RR");
  }
  if(nullptr != rr_timer_) rr_timer_->start();
}

void Socksp2gBase::sendProcess()
{
  if ( 0 == outgoing_ ) { return; }
  if(false == isWriten ){ return; }
  if ( !checkSocket() ) { return; }
  compress::Compress compressor;

  while ( outgoing_->size() > 0 && sentList_.size() < sendWithoutAck_ ) {

    socksp2g::PackType t = socksp2g::kTcpIpData;

    tlg::MessageNew msg = outgoing_->first();
    QByteArray rawTlg = tlg::raw2tlg(msg);
    if ( optimCompress() ) {
      //debug_log << "hetr";
      //t = socksp2g::kTcpIpDataZ;
      QByteArray ba = compressor.compress(rawTlg);
      double k = double(ba.size()) / rawTlg.size();
      if ( k < 0.7 ) {
        rawTlg = ba;
        //t = socksp2g::kTcpIpDataZC;
      }
    }

    if ( rawTlg.size() > (socksp2g::kMaxLen) ) {
      warning_log << QObject::tr("Невозможно передать телеграмму. Максимальный размер %1 Кбайт.").arg(socksp2g::kMaxLen/1024);
      outgoing_->removeFirst();
      continue;
    }

    socksp2g::ServicePacket pack(t, magic_);
    pack.setNum(++num_);
    pack.setLen(rawTlg.size());
    pack.setInfoLen(socksp2g::INFO_MSG_PACKET_SIZE);

    QByteArray ba = pack.toNet();
    socksp2g::SendBox box;
    box.start = pack;
    socksp2g::InfoPacket info = socksp2g::InfoPacket::fromMessage(msg);
    ba += info.toNet();
    ba += rawTlg;
    box.info = info;
    pack.setType(socksp2g::kTcpIpEnd);
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

void Socksp2gBase::writeBox(const socksp2g::SendBox &box){
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

void Socksp2gBase::tryResendMsg() {
  //debug_log<<"tryResendMsg"<<sentList_.size();

  QMapIterator<quint16, socksp2g::SendBox> i(sentList_);
  while (i.hasNext()) {
    i.next();
    int sec_to = i.value().date_recv.secsTo(QDateTime::currentDateTime());
    if(kResendInterval < sec_to ){
      outgoing_->append(i.value().info.msg());
      sentList_.remove(i.key());
    }
  }
}

void Socksp2gBase::slotSocketError(QAbstractSocket::SocketError socketError)
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

bool Socksp2gBase::checkSocket() const
{
  return (nullptr != socket_ &&
                     socket_->isValid() &&
                     socket_->state() == QAbstractSocket::ConnectedState );
}

void Socksp2gBase::appendData(const QByteArray& data)
{
  raw_ += data;
}

void Socksp2gBase::removeData(int size)
{
  raw_.remove(0, size);
}

void Socksp2gBase::clearData()
{
  raw_.clear();
}


} // meteo
