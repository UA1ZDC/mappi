#include "unimasbase.h"
#include "unimasstreamserver.h"

#include <arpa/inet.h>
#include <cstring>

#include <qtimer.h>
#include <qprocess.h>
#include <qdatetime.h>

#include <cross-commons/app/paths.h>
#include <cross-commons/debug/tlog.h>
#include <commons/funcs/mnio.h>
#include <meteo/commons/msgparser/msgmetainfo.h>
#include <meteo/commons/msgparser/tlgparser.h>
#include <meteo/commons/msgparser/common.h>
#include <meteo/commons/global/global.h>
#include <commons/compresslib/mcompress.h>

namespace meteo {

const int kMaxSize = std::numeric_limits<uint16_t>::max();
//const int kChunkSz = 32;
const int kResendInterval = 60*10;

UnimasBase::UnimasBase(const msgstream::Options& opt,
                       TSList<tlg::MessageNew>* outgoing,
                       TSList<tlg::MessageNew>* incoming,
                       AppStatusThread* status,
                       QObject *p) :
  QObject( p ),
  host_( opt.sockspec().host().c_str() ),
  port_( opt.sockspec().port() ),
  socket_( nullptr ),
  status_(status),
  headPacket_( ::meteo::msgstream::SOCKSPEC_TCP == opt.sockspec().mode() ? TServicePacket::TCP : TServicePacket::UTCP ),
  endPacket_( ::meteo::msgstream::SOCKSPEC_TCP == opt.sockspec().mode() ? TServicePacket::TCP : TServicePacket::UTCP ),
  outgoing_(outgoing),
  incoming_(incoming),
  compressionMode_( opt.sockspec().compress_mode() )
{
  stream_id_ = QString::fromStdString(opt.id());
  reconnectTimeout_ = 5*1000;
  receiveTimeout_ = opt.sockspec().ack_timeout() *0.5*1000 ;
  rr_timeout_ = receiveTimeout_*0.5;
  isTcpMode_ = (msgstream::SOCKSPEC_TCP == opt.sockspec().mode()) ;
  sendWithoutAck_ = opt.sockspec().window() ;
  state_ = kStart;
  resendTimeout_ = 60*10*1000;
  parser_ = new TlgParser( QByteArray(nullptr), opt.split_stucked() );
  initTimers();
}

UnimasBase::~UnimasBase()
{
  if ( nullptr != work_timer_ ) {
      disconnect(work_timer_, nullptr, this, nullptr);
      work_timer_->deleteLater();
      work_timer_ = nullptr;
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

  deleteSocket();
  if(nullptr != status_) status_->setParam(kLinkStatus, QObject::tr("Отсутствует"),app::OperationState_ERROR);
}

void UnimasBase::slotResendTimeout()
{
 if(nullptr != resend_timer_) resend_timer_->stop();
  tryResendMsg();
 if(nullptr != resend_timer_) resend_timer_->start();
}

void UnimasBase::slotRRTimeout()
{
  if(nullptr != rr_timer_) rr_timer_->stop();
  sendRR();
  if(nullptr != rr_timer_) rr_timer_->start();
}

void UnimasBase::slotReadyRead()
{
  if(nullptr != work_timer_) work_timer_->stop();
  if(nullptr != rr_timer_) rr_timer_->stop();
  readProcess();
  sendProcess();
  if(nullptr != rr_timer_) rr_timer_->start();
  if(nullptr != work_timer_) work_timer_->start();
}

void UnimasBase::slotNewMessage(){
  if(nullptr != work_timer_) work_timer_->stop();
  sendProcess();
  readProcess();
  if(nullptr != work_timer_) work_timer_->start();
}

void UnimasBase::readProcess()
{
  const qint64 kChunk = 1024;
  qint64 rsz = 0;
  do{
      QByteArray incomingBuffer;
      if ( !checkSocket() ) {state_ = kStart;return;}
      rsz = MnCommon::readFromSocket(socket_, &incomingBuffer,kChunk);
      emit signalBytesRecv(rsz);
      raw_.append(incomingBuffer);
      while ( receiveProcess() ){ }
    } while ( rsz > 0 );
  if ( -1 == rsz ) {
      error_log << tr("Ошибка при чтении данных из сокета.");
      return;
    }
}

void UnimasBase::sendProcess() {

  if ( 0 == outgoing_ ) { return; }
  if ( !checkSocket() ) { return; }
  if(false == isWriten ) return;

  while ( outgoing_->size() > 0 && sentList_.size() < sendWithoutAck_ ) {
    auto firstTlg = outgoing_->takeFirst();
    TlgBox box = prepareMessage(firstTlg);
    if( box.data.size() > kMaxSize ) {
      warning_log << QObject::tr("Невозможно передать телеграмму больше %1 байт, размер телеграмы %2.").arg(kMaxSize)
                     .arg(box.data.size());
      debug_log << firstTlg.metainfo().Utf8DebugString();
    } else if(false == sendTlg(box))
    {
      state_ = kError;
      stop();
      return;
    }
  }
}

void UnimasBase::tryResendMsg() {
  if(0 > sentList_.size()) return;
  //debug_log<<"tryResendMsg"<<sentList_.size();
  QMapIterator<quint16, TlgBox> i(sentList_);
  while (i.hasNext()) {
      i.next();
      int sec_to = i.value().sentDt.secsTo(QDateTime::currentDateTime());
      if(kResendInterval < sec_to ){
          outgoing_->append(i.value().orig);
          sentList_.remove(i.key());
        }
    }
}

bool UnimasBase::processAck()
{
  QMapIterator<quint16, TlgBox> i(sentList_);
  while (i.hasNext()) {
      i.next();
      if ( headPacket_ == i.value().end ) {
          emit signalConfirmRecv(sentList_.remove(i.key()));
          //debug_log << "podtverdil" << i.value().crc<<sentList_.size();
          break;
        }
    }
  state_ = kStart;
  return true;
}

void UnimasBase::closeSession()
{
  raw_.clear();
  deleteSocket();
  if(nullptr != status_) status_->setParam(kLinkStatus, QObject::tr("Отсутствует"),app::OperationState_ERROR);
  reconnectTimer_->start();
}

void UnimasBase::deleteSocket(){
  if ( nullptr != socket_ ) {
      if ( QAbstractSocket::UnconnectedState != socket_->state() ) {
          if ( nullptr != socket_ ) socket_->disconnectFromHost();
          if ( QAbstractSocket::UnconnectedState != socket_->state()
               &&!socket_->waitForDisconnected()) {
              if ( nullptr != socket_ ) socket_->abort();
            }
        }
      if ( nullptr != socket_ ) {
          disconnect(socket_, nullptr, this, nullptr);
          socket_->abort();
          socket_->deleteLater();
          socket_ = nullptr;
        }
    }
}


bool UnimasBase::getPacket()
{
  if(raw_.size() < headPacket_.packetSize()){
      state_ = kStart;
      return false;
    }

  int rsz = headPacket_.parse( raw_ );
  if ( 0 == rsz ) {
      return false;
    }
  removeData(rsz);

  switch (headPacket_.type) {
    case kTcpIpAck:
      state_ = kAck;
      break;
    case kTcpIpRR:
      state_ = kRR;
      break;
    case kTcpIpEnd:
      if(false == writeEndPacket(headPacket_)){
          state_ = kError;
          return false;
        }
      state_ = kStart;
      break;
    case kTcpIpData:
    case kTcpIpDataZ:
    case (kTcpIpDataZ | kTcpCompress):
      state_ = kInfo;
      break;
    default:
      error_log <<state_<< headPacket_.type<<QObject::tr("Ожидался TCP_IP_DATA, "
                                                     "TCP_IP_DATA_Z, TCP_IP_ACK,"
                                                     "TCP_IP_END или TCP_IP_RR.");
      stop();
      state_ = kStart;
      break;
    }
  return true;
}

bool UnimasBase::getInfoPacket()
{
  if ( raw_.size() < headPacket_.len ) {
      state_ = kInfo;
      return false;
    }
  QByteArray  ba;
  ba = raw_.left( headPacket_.len );
  removeData(headPacket_.len);

  if ( msgstream::COMPRESS_ALWAYS == compressionMode_ ||
       (msgstream::COMPRESS_OPTIM == compressionMode_ && (headPacket_.type & kTcpCompress)) )
    {
      compress::Compress compressor;
      bool ok=true;
      ba = compressor.decompress( ba ,&ok);
      //ba = decompressPacket( ba );
      if (!ok || ba.isNull() ) {
          state_ = kError;
          error_log << QObject::tr("Ошибка при распаковке сообщения.");
          return false;
        }
    }
  if(parseRaw(ba)){
      state_ = kEnd;

      return true;
    }
  //debug_log << "no data";
  //headPacket_.clear();
  //?? endPacket_.clear();
  state_ = kStart;
  return true;
}


bool UnimasBase::writeEndPacket(const TServicePacket &end)
{
  if(false == checkSocket()) {
      return false;
    }
  const QByteArray &ack = end.toAck().toNet();
  const qint64 wsz = MnCommon::writeToSocket(socket_,ack);
  emit signalBytesSended(wsz);

  if ( ack.size() != wsz ) {
      error_log << tr("Ошибка при отправке пакета TCP_IP_ACK");
      state_ = kError;
      return true;
    }
  emit signalConfirmSended(1);
  //tlg::MessageNew msg = info_.msg();
  //msg.mutable_metainfo()->set_from(id_.toUtf8().constData());//FIXME
   //info_log << tr("Отправлено подтверждение для") << headPacket_.num;
  return true;
}


bool UnimasBase::receiveProcess()
{
  bool k = false ;
  switch ( state_ ) {
    case kError:
      stop();
      reconnectTimer_->start();
      return false;
    case kStart:
      k = getPacket();
      break;
    case kEnd:
      k = getPacket();
      isWriten = true;
      break;
    case kInfo:
      k = getInfoPacket();
      isWriten = false;
      break;
    case kAck:
      k = processAck();
      break;
    case kRR:
      //debug_log <<"kRR";
      sendProcess();
      state_ = kStart;
      return true;
    }
  return k;
}

bool UnimasBase::parseRaw(const QByteArray& ba)
{
  if(nullptr == incoming_){
      error_log<<tr("Обработчик не установлен!");
      return false;
    }
  parser_->setData( ba );
  tlg::MessageNew msg;
  if( true == parser_->parseNextMessage(&msg))
     {
      MsgMetaInfo meta(msg);
      QDateTime dt = meta.calcConvertedDt();
      msg.mutable_metainfo()->set_converted_dt(dt.toString(Qt::ISODate).toUtf8().constData());
      msg.mutable_metainfo()->set_from(stream_id_.toStdString());
      incoming_->append(msg);
      emit signalMessagesRecv(1);
    }
  return true;
}

bool UnimasBase::sendTlg(TlgBox tlg)
{
  if(false == writeTlg(tlg)){
      state_ = kError;
      return false;
    }
  quint16 crc = tlg.crc;
  tlg.state = TlgBox::kWaitAck;
  tlg.sentDt = QDateTime::currentDateTime();
  tlg.end = tlg.head;
  tlg.end.type = kTcpIpEnd;
  sentList_.insertMulti(crc, tlg);
  state_ = kStart;
  return true;
}

bool UnimasBase::writeTlg(TlgBox tlg){
  if ( !checkSocket() ) {
      return false;
    }
  QByteArray ba = tlg.head.toNet();
  ba += tlg.data;
  tlg.end = tlg.head;
  tlg.end.type = kTcpIpEnd;
  ba +=  tlg.end.toNet();
  qint64 wsz = MnCommon::writeToSocket(socket_,ba);
  emit signalBytesSended(wsz);
  if ( ba.size() != wsz ) {
      error_log << QObject::tr("Ошибка при отправке сообщения");
      tlg.head.printDebug();
      return false;
    }
  emit signalMessagesSended(1);
  return true;
}


TlgBox UnimasBase::prepareMessage(const tlg::MessageNew& msg)const{

  TlgBox box( isTcpMode_ ? TServicePacket::TCP : TServicePacket::UTCP );
  box.md5 = tlg::md5hash(msg);
  box.orig = msg;
  box.data = tlg::raw2tlg(box.orig);
  box.crc = MnMath::makeCrc(box.data.constData(), box.data.size());

  QByteArray ahd;
  ahd += box.orig.header().has_t1() ? box.orig.header().t1().c_str() : "";
  ahd += box.orig.header().has_t2() ? box.orig.header().t2().c_str() : "";
  ahd += box.orig.header().has_a1() ? box.orig.header().a1().c_str() : "";
  ahd += box.orig.header().has_a2() ? box.orig.header().a2().c_str() : "";
  ahd += box.orig.header().has_ii() ? QString::number(box.orig.header().ii()).toUtf8() : "";
  ahd += box.orig.header().has_cccc() ? box.orig.header().cccc().c_str() : "";

  if ( msgstream::COMPRESS_ALWAYS == compressionMode_ ) {
      box.head.type = kTcpIpData;
      compress::Compress compressor;
      box.data = compressor.compress(box.data );
    }
  else if ( msgstream::COMPRESS_OPTIM == compressionMode_ ) {
      box.head.type = kTcpIpDataZ;
     // meteo::Compress cmp;
     // QByteArray ba = cmp.compress( &box.data);
      compress::Compress compressor;
      QByteArray ba = compressor.compress(box.data );
      if ( double(ba.size()) / box.data.size() < 0.7 ) {
        box.head.type |= kTcpCompress;
        box.data = ba;
      }
  }
  else {
    box.head.type = kTcpIpData;
  }

  static int ijp = 0;
  static int num = 0;

  if ( ijp > 999 ) { ijp = 0; }
  if ( num > 999 ) { num = 0; }

  box.head.num = num++;
  box.head.len = box.data.size();
  box.head.pri = tlg::priority(box.orig);
  std::memcpy( box.head.ahd, ahd.data(), ahd.size() );

  if ( isTcpMode_ ) {
    box.head.tcp_ijp = ijp++;
  }
  else {
    if ( box.orig.format() == tlg::kWMO ) {
      box.head.utcp_format = 1;
    }
    else {
      box.head.utcp_format = 2;
    }
    box.head.utcp_ijp.ijp = ijp++;
    box.head.utcp_text_offs = 0;
    box.head.utcp_ijp.jno = 0;
    box.head.utcp_ijp.textno = 0;
  }

  box.state = TlgBox::kSending;

  return box;
}

bool UnimasBase::sendRR()
{
 // debug_log <<"sendRR";
  // if ( rrPacketDt_.secsTo(QDateTime::currentDateTime()) < rrTimeout_ ) {
  //     return true;
  //   }
  if ( !checkSocket() ) {
      state_ = kError;
      return false;
    }
  if(false == isWriten ) return true;
  TServicePacket rr( isTcpMode_ ? TServicePacket::TCP : TServicePacket::UTCP );
  rr.type = kTcpIpRR;
  const QByteArray &ba = rr.toNet();
  qint64 wsz = MnCommon::writeToSocket(socket_,ba);
  emit signalBytesSended(wsz);

  if ( ba.size() != wsz ) {
      state_ = kError;
      //debug_log<<"sendRR() stop";
      stop();
      return false;
    }
  // rrPacketDt_ = QDateTime::currentDateTime();
  return true;
}


bool UnimasBase::checkSocket() const
{
  if ( nullptr != socket_ && socket_->isValid() &&
       socket_->state() == QAbstractSocket::ConnectedState )
    {
      return true;
    }
  //info_log << QObject::tr("Клиент отключился.");
  return false;
}

void UnimasBase::initTimers()
{
  if ( nullptr != work_timer_ ) {
      disconnect(work_timer_, nullptr, this, nullptr);
      work_timer_->deleteLater();
      work_timer_ = nullptr;
    }
  work_timer_ = new QTimer(this);
  work_timer_->setInterval(receiveTimeout_);
  connect( work_timer_, SIGNAL(timeout()), SLOT(slotReceiveTimeout()) );

  if ( nullptr != reconnectTimer_ ) {
      disconnect(reconnectTimer_, nullptr, this, nullptr);
      reconnectTimer_->deleteLater();
      reconnectTimer_ = nullptr;
    }
  reconnectTimer_ = new QTimer(this);
  reconnectTimer_->setInterval(reconnectTimeout_);
  connect( reconnectTimer_, SIGNAL(timeout()), SLOT(slotReconnectTimeout()) );

  if ( nullptr != resend_timer_ ) {
      disconnect(resend_timer_, nullptr, this, nullptr);
      resend_timer_->deleteLater();
      resend_timer_ = nullptr;
    }
  resend_timer_ = new QTimer(this);
  resend_timer_->setInterval(resendTimeout_);
  connect( resend_timer_, SIGNAL(timeout()), SLOT(slotResendTimeout()) );
  resend_timer_->start();

  if ( nullptr != resend_timer_ ) {
      disconnect(resend_timer_, nullptr, this, nullptr);
      resend_timer_->deleteLater();
      resend_timer_ = nullptr;
    }
  rr_timer_ = new QTimer(this);
  rr_timer_->setInterval(rr_timeout_);
  connect( rr_timer_, SIGNAL(timeout()), SLOT(slotRRTimeout()) );
  rr_timer_->start();


}


void UnimasBase::slotSocketError(QAbstractSocket::SocketError socketError)
{

  switch ( socketError ) {
    case QAbstractSocket::ConnectionRefusedError:
      warning_log << tr("Сервер недоступен");
      break;
    case QAbstractSocket::RemoteHostClosedError:
      warning_log << tr("Сервер разорвал соединение");
      break;
    case QAbstractSocket::HostNotFoundError:
      warning_log << tr("Сервер недоступен");
      break;
    case QAbstractSocket::SocketAccessError:
      error_log << tr("Ошибка SocketAccessError");
      break;
    case QAbstractSocket::SocketResourceError:
    case QAbstractSocket::SocketTimeoutError:
    case QAbstractSocket::DatagramTooLargeError:
    case QAbstractSocket::NetworkError:
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
      error_log << tr("Ошибка ")<< socket_->errorString();
      break;
    case QAbstractSocket::TemporaryError:
    case QAbstractSocket::UnknownSocketError:
      if(nullptr == socket_) return;
      error_log << tr("Неизвестная ошибка сокета")<< socket_->errorString();
      break;
    }
  stop();
  if(nullptr != status_) status_->setParam(kLinkStatus, QObject::tr("Отсутствует"),app::OperationState_ERROR);
  reconnectTimer_->start();
}

void UnimasBase::appendData(const QByteArray& data)
{
  raw_ += data;
}

void UnimasBase::removeData(int size)
{
  raw_.remove(0, size);
}

void UnimasBase::clearData()
{
  raw_.clear();
}

}
