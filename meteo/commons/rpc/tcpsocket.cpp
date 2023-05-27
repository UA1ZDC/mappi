#include "tcpsocket.h"

#include <quuid.h>
#include <qhostaddress.h>

#include <cross-commons/debug/tlog.h>
#include <commons/funcs/mnio.h>

#include "methodpack.h"
#include "parser.h"

namespace meteo {
namespace rpc {

static const int32_t kChunkSz = 64;
static const int32_t kIntSz = sizeof(int32_t);
static const int32_t kSrvidentSz = kSrvident.size();

TcpSocket::TcpSocket( const QString& addr )
  : QThread(),
  parser_( new Parser ),
  address_(addr),
  mode_(kClientSender)
{
}

TcpSocket::TcpSocket( const QString& addr, const QByteArray& uuid )
  : QThread(),
  parser_( new Parser ),
  uuid_(uuid),
  address_(addr),
  mode_(kClientReceiver)
{
}

TcpSocket::TcpSocket( int32_t descr )
  : QThread(),
  parser_( new Parser ),
  mode_(kServer),
  sockdescr_(descr)
{
}

TcpSocket::~TcpSocket()
{
  delete parser_; parser_ = nullptr;
}

TcpSocket* TcpSocket::connect()
{
  if ( nullptr != recvsock_ && kConnected == recvsock_->status() ) {
    return recvsock_;
  }
  if ( kConnected == status() ) {
    return nullptr;
  }
  if ( false == synchromutex_.tryLock() ) {
    error_log << QObject::tr("Мьютекс синхронизации событий уже заблокирован. Непредвиденная ошибка");
    return nullptr;
  }
  if ( nullptr != recvsock_ && kConnected == recvsock_->status() ) {
    synchromutex_.unlock();
    return recvsock_;
  }
  if ( kConnected == status() ) {
    synchromutex_.unlock();
    return nullptr;
  }
  synchrocond_.wait(&synchromutex_, 50000 ); //Ожидание коннекта к серверу, включая handshake-обмен
  synchromutex_.unlock();
  if ( kClientSender == mode_ ) {
    return recvsock_;
  }
  return nullptr;
}

TcpSocket::Status TcpSocket::status() const {
  QMutex* m = const_cast<QMutex*>(&mutex_);
  QMutexLocker lock(m);
  return status_;
}

void TcpSocket::setStatus( Status st )
{
  mutex_.lock();
  if ( status_ == st ) {
    mutex_.unlock();
    return;
  }
  status_ = st;
  mutex_.unlock();
  if ( kConnected == st ) {
    emit connected(this);
  }
  else if ( kUnknown == st ) {
    emit disconnected(this);
//    Q_ASSERT(false);
  }
}

bool TcpSocket::parseAddress( const std::string& a, QString* h, int32_t* port )
{
  return parseAddress( QString::fromStdString(a), h, port );
}

bool TcpSocket::parseAddress( const QString& a, QString* h, int32_t* port )
{
  QStringList list = a.split( QChar(':') );
  if ( 2 != list.size() ) {
    error_log << QObject::tr("Неверный формат адреса хоста = %1. Формат = <адрес>:<порт> ")
      .arg(a);
    return false;
  }
  bool ok = false;
  *port =  list[1].toInt(&ok);
  if ( false == ok || 0 >= *port ) {
    error_log << QObject::tr("Неверный формат адреса хоста = %1. Формат = <адрес>:<порт>")
      .arg(a);
    return false;
  }
  *h = list[0];
  return true;
}

QString TcpSocket::stringAddress( const QString& host, int32_t port )
{
  return QString("%1:%2").arg(host).arg(port);
}

void TcpSocket::run()
{
  bool fl = false;
  QObject::connect( this, &TcpSocket::messageAdded, this, &TcpSocket::slotMessageAdded );
  switch ( mode_ ) {
    case kServer:
      fl = runServer();
      break;
    case kClientSender:
    case kClientReceiver:
      fl = runClient();
      break;
  }
  if ( false == fl ) {
    error_log << QObject::tr("Не удалось инициализировать канал приема/передачи информации. Поток завершается.");
    return;
  }
  QThread::exec();
  if ( true == synchromutex_.tryLock() ) {
    synchrocond_.wakeAll();
    synchromutex_.unlock();
  }
  if ( kClientReceiver == mode_ ) {
    delete tmwake_; tmwake_ = nullptr;
  }
  QObject::disconnect(sock_);
  delete sock_; sock_ = nullptr;
}

bool TcpSocket::runServer()
{
  if ( nullptr != sock_ ) {
    delete sock_; sock_ = nullptr;
  }

  if ( 0 < sockdescr_ ) {
    MnCommon::setSocketOptions( sockdescr_, 10, 3, 2 );
  }

  maclevel_ = MnCommon::socket_mac_level(sockdescr_);

  sock_ = new QTcpSocket();
  setStatus(kSizeHandshing);
  QObject::connect( sock_, &QAbstractSocket::connected, this, &TcpSocket::slotSockConnected );
  QObject::connect( sock_, &QAbstractSocket::disconnected, this, &TcpSocket::slotSockDisconnected );
  QObject::connect( sock_, SIGNAL( error( QAbstractSocket::SocketError ) ), this, SLOT( slotSockError( QAbstractSocket::SocketError ) ) );
  QObject::connect( sock_, &QAbstractSocket::stateChanged, this, &TcpSocket::slotSockStateChanged );
  QObject::connect( sock_, &QIODevice::readyRead, this, &TcpSocket::slotReadyRead );
  QObject::connect( sock_, &QIODevice::bytesWritten, this, &TcpSocket::slotBytesWritten );
  sock_->setSocketDescriptor( sockdescr_ );
  address_ = sock_->peerAddress().toString() + ':' + QString::number( sock_->localPort() );
  return true;
}

bool TcpSocket::runClient()
{
  QString host;
  int32_t port;
  if ( false == TcpSocket::parseAddress( address_, &host, &port ) ) {
    error_log << QObject::tr("Не верный формат адреса в строеке = %1").arg(address_);
    return false;
  }
  if ( nullptr != sock_ ) {
    delete sock_; sock_ = nullptr;
  }
  if ( 0 == uuid_.size() ) {
    uuid_ = QUuid::createUuid().toByteArray();
  }
  if ( kClientReceiver == mode_ ) {
    tmwake_ = new QTimer();
    QObject::connect( tmwake_, &QTimer::timeout, this, &TcpSocket::slotWakeClient );
    tmwake_->setInterval(50);
    QObject::connect( this, &TcpSocket::startWakingUntilRecv, this, &TcpSocket::slotStartWakingUntilRecv );
    QObject::connect( this, &TcpSocket::stopWakingUntilRecv, this, &TcpSocket::slotStopWakingUntilRecv);
  }
  sock_= new QTcpSocket();
  QObject::connect( sock_, &QAbstractSocket::connected, this, &TcpSocket::slotSockConnected );
  QObject::connect( sock_, &QAbstractSocket::disconnected, this, &TcpSocket::slotSockDisconnected );
  QObject::connect( sock_, SIGNAL( error( QAbstractSocket::SocketError ) ), this, SLOT( slotSockError( QAbstractSocket::SocketError ) ) );
  QObject::connect( sock_, &QAbstractSocket::stateChanged, this, &TcpSocket::slotSockStateChanged );
  QObject::connect( sock_, &QIODevice::readyRead, this, &TcpSocket::slotReadyRead );
  QObject::connect( sock_, &QIODevice::bytesWritten, this, &TcpSocket::slotBytesWritten );

  setStatus(kConnecting);
  sock_->connectToHost( host, port );
  return true;
}

bool TcpSocket::sendHandshakeInfo()
{
  setStatus(kSizeHandshing);
  if ( nullptr == sock_ || QAbstractSocket::ConnectedState != sock_->state() ) {
    error_log << QObject::tr("Нет соединения с сокетом");
    return false;
  }

  QByteArray arr( kIntSz, 0 );
  int32_t sz = (kSrvident + uuid_).size();
  ::memcpy( arr.data(), &sz, kIntSz );
  arr += kSrvident + uuid_;
  totalwritten_ += arr.size();
  qint64 writen = MnCommon::writeToSocket( sock_, arr );
  if ( arr.size() != writen ) {
    error_log << QObject::tr("Не удалось отправить идентификатор соединения");
    return false;
  }
  return true;
}

bool TcpSocket::startRecvThread()
{
  recvsock_ = new TcpSocket( address_, uuid_ );
  recvsock_->moveToThread(recvsock_);
  recvsock_->start();
  recvsock_->connect();
  if ( kConnected != recvsock_->status() ) {
    recvsock_->exit(0);
    recvsock_->wait();
    delete recvsock_;
    recvsock_ = nullptr;
    return false;
  }
  return true;
}

bool TcpSocket::getMessage( MethodPack* pack )
{
  if ( nullptr == pack ) {
    error_log << QObject::tr("Нулевой указатель");
    return false;
  }
  { 

    QMutexLocker lock(&mutex_);
    if ( 0 == packlist_.size() ) {
      return false;
    }
    pack->copyFrom( packlist_.takeFirst() );
  }
  return true;
}

bool TcpSocket::sendMessage( const MethodPack* pack )
{
  if ( nullptr == pack ) {
    error_log << QObject::tr("Нулевой указатель");
    return false;
  }
  if ( kConnected != status() ) {
    error_log << QObject::tr("Соединение не установлено. Сообщение не будет отправлено");
    return false;
  }
  if ( false == QThread::isRunning() ) {
    error_log << QObject::tr("Поток передачи сообщений не запущен. Сообщение не будет отправлено");
    return false;
  }
  { 
    QMutexLocker lock(&mutex_);
    packlist_.append(*pack);
  }
  emit messageAdded();
  return true;
}

bool TcpSocket::waitForAnswer( int32_t msecs_timeout, const QByteArray& callid )
{
  if ( nullptr != recvsock_ && kConnected == recvsock_->status() ) {
    recvsock_->setWaitId(callid);
  }
  if ( false == recvsock_->synchromutex_.tryLock() ) {
    error_log << QObject::tr("Критическая ошибка. Мьютекс удаленного вызова уже заблокирован.");
    return false;
  }
  bool fl = recvsock_->synchrocond_.wait( &recvsock_->synchromutex_, msecs_timeout);
  recvsock_->synchromutex_.unlock();
  recvsock_->resetWaitId();
  return fl;
}

void TcpSocket::setWaitId( const QByteArray& id )
{
  QMutexLocker lock(&mutex_);
  waitid_ = id;
  //debug_log << "ustanovlen waitid =" << waitid_;
  if ( kClientReceiver == mode_ ) {
    emit startWakingUntilRecv();
  }
}

void TcpSocket::resetWaitId()
{
  QMutexLocker lock(&mutex_);
  //debug_log << "sbroshen waitid =" << waitid_;
  waitid_ = QByteArray();
  if ( kClientReceiver == mode_ ) {
    emit stopWakingUntilRecv();
  }
}

void TcpSocket::quitThread()
{
  if ( false == QThread::isRunning() ) {
    return;
  }
  if ( true == isRunning() ) {
    this->exit(0);
    this->wait();
  }
}

void TcpSocket::slotSockConnected()
{
  sockdescr_ = sock_->socketDescriptor();

  maclevel_ = MnCommon::socket_mac_level(sockdescr_);
  if ( 0 < sockdescr_ ) {
    MnCommon::setSocketOptions( sockdescr_, 10, 3, 2 );
  }
  mutex_.lock();
  Status st = status_;
  mutex_.unlock();
  if ( kConnecting != st ) {
    error_log << QObject::tr("Ожидаемое состояние = kConnecting. Текущее = %1. Непредвиденная ошибка. ПОток завершается")
      .arg( st );
    if ( true == isRunning() ) {
      QThread::exit(0);
    }
    return;
  }
  if ( false == sendHandshakeInfo() ) {
    error_log << QObject::tr("Непредвиденная ошибка. Поток завершается");
    if ( true == isRunning() ) {
      QThread::exit(0);
    }
    return;
  }
}

void TcpSocket::slotSockDisconnected()
{
//  error_log << QObject::tr("Соединение разорвано. Поток завершается.");
  QObject::disconnect(sock_);
  setStatus(kUnknown);
  if ( true == isRunning() ) {
    QThread::exit(0);
  }
}

void TcpSocket::slotSockError( QAbstractSocket::SocketError err )
{
  QString errstr = QObject::tr("Неизвестная ошибка");
  if ( nullptr != sock_ ) {
    errstr = sock_->errorString();
  }
  bool flfinish = false;
  switch ( err ) {
    case QAbstractSocket::ConnectionRefusedError:
    case QAbstractSocket::RemoteHostClosedError:
    case QAbstractSocket::HostNotFoundError:
    case QAbstractSocket::SocketAccessError:
    case QAbstractSocket::SocketResourceError:
    case QAbstractSocket::SocketTimeoutError:
    case QAbstractSocket::NetworkError:
    case QAbstractSocket::AddressInUseError:
    case QAbstractSocket::SocketAddressNotAvailableError:
    case QAbstractSocket::UnsupportedSocketOperationError:
    case QAbstractSocket::OperationError:
    case QAbstractSocket::UnknownSocketError:
      flfinish = true;
      break;
    default:
      break;
  }
//  error_log << QObject::tr("В сетевом сокете возникла ошибка = %1").arg(errstr);
  if ( true == flfinish ) {
    setStatus(kUnknown);
//    error_log << QObject::tr("Возникшая ошибка является критической. Поток завершает работу");
    if ( true == isRunning() ) {
      QThread::exit(0);
    }
  }
}

void TcpSocket::slotSockStateChanged( QAbstractSocket::SocketState st )
{
  return;
  QString strstate = QObject::tr("Неизвестное остояние сокета");
  switch ( st ) {
    case QAbstractSocket::UnconnectedState:
      strstate = QObject::tr("Соединение разорвано");
      break;
    case QAbstractSocket::HostLookupState:
      strstate = QObject::tr("Поиск хоста");
      break;
    case QAbstractSocket::ConnectingState:
      strstate = QObject::tr("Установка соединения");
      break;
    case QAbstractSocket::ConnectedState:
      strstate = QObject::tr("Соединение установлено");
      break;
    case QAbstractSocket::BoundState:
      strstate = QObject::tr("Состояние = BOUND");
      break;
    case QAbstractSocket::ClosingState:
      strstate = QObject::tr("Соединение закрывается");
      break;
    case QAbstractSocket::ListeningState:
      strstate = QObject::tr("Сокет слушает");
      break;
  }
  info_log << QObject::tr("Состояние сокета = %1").arg(strstate);
}

void TcpSocket::slotReadyRead()
{
  if ( nullptr == sock_ || QAbstractSocket::ConnectedState != sock_->state() ) {
    error_log << QObject::tr("Нет соединения с сокетом") << sock_->state();
    return;
  }
  parseArray();
  mutex_.lock();
  Status st = status_;
  mutex_.unlock();
  if ( kUnknown == st ) {
    error_log << QObject::tr("Непредвиденная ошибка во время получения данных. Поток завершается");
    if ( true == isRunning() ) {
      QThread::exit(0);
    }
  }
}

void TcpSocket::slotBytesWritten( qint64 bytes )
{
  //debug_log << "OSTALOS OTPRAVIT =" << (totalwritten_ -= bytes);
}

void TcpSocket::slotMessageAdded()
{
  if ( kConnected != status() ) {
    error_log << QObject::tr("Соединение не установлено. Сообщение не будет отправлено");
    return;
  }
  if ( false == QThread::isRunning() ) {
    error_log << QObject::tr("Поток передачи сообщений не запущен. Сообщение не будет отправлено");
    return;
  }
  int packsize = 0;
  {
    QMutexLocker lock(&mutex_);
    if ( 0 == packlist_.size() ) {
//      error_log << QObject::tr("Нет сообщений для отправки. Непредвиденная ошибка");
      return;
    }
    packsize = packlist_.size();
  }
  while ( 0 < packsize ) {
    mutex_.lock();
    const MethodPack& pack = packlist_[0];
    mutex_.unlock();
    if ( false == writeMessage(pack) ) {
      continue;
    }

    if ( QAbstractSocket::ConnectedState != sock_->state() ) {
      error_log << QObject::tr("Нет соединения. Не могу отправить сообщение Package");
      return;
    }

    {
      QMutexLocker lock(&mutex_);
      packlist_.removeFirst();
      packsize = packlist_.size();
    }
  }
}

void TcpSocket::slotStartWakingUntilRecv()
{
  tmwake_->start();
}

void TcpSocket::slotStopWakingUntilRecv()
{
  tmwake_->stop();
}

void TcpSocket::slotWakeClient()
{
  QMutexLocker lock(&mutex_);
  if ( 0 != waitid_.size() && 0 != packlist_.size() ) {
    for ( auto p : packlist_ ) {
      if ( p.id() == waitid_ ) {
//        debug_log << "srabotala proverka id ==" << waitid_;
        synchromutex_.lock();
        synchrocond_.wakeAll();
        synchromutex_.unlock();
        return;
      }
//      else {
//        debug_log << "levy id. moy =" << waitid_ << "prishel =" << p.id();
//      }
    }
    packlist_.clear();
  }
}

void TcpSocket::parseArray()
{
  if ( nullptr == sock_ || QAbstractSocket::ConnectedState != sock_->state() ) {
    error_log << QObject::tr("Непредвиденная ошибка.");
    return;
  }
  while ( 0 < MnCommon::readFromSocket( sock_, &recvbuf_, kChunkSz ) ) {
    switch ( status() ) {
      case kConnecting:
        error_log << QObject::tr("Ошибка. Неопределенное состояние");
        setStatus(kUnknown);
        return;
        break;
      case kSizeHandshing:
        if ( false == parseSizeHandshake() ) {
          if ( kSizeHandshing != status_ ) {
            error_log << QObject::tr("Ошибка получения идентификатора соединения.");
            setStatus(kUnknown);
            return;
          }
          break;
        }
        setStatus(kIdentHandshaking);
      case kIdentHandshaking:
        if ( false == parseIdentHandshake() ) {
          if ( kIdentHandshaking != status_ ) {
            error_log << QObject::tr("Ошибка получения идентификатора соединения.");
            setStatus(kUnknown);
            return;
          }
          break;
        }
        setStatus(kHandshaking);
      case kHandshaking:
        if ( false == parseHandshake() ) {
          if ( kHandshaking!= status_ ) {
            error_log << QObject::tr("Ошибка получения идентификатора соединения.");
            setStatus(kUnknown);
            return;
          }
          break;
        }
        if ( kServer == mode_ ) {
          if ( false == sendHandshakeInfo() ) {
            error_log << QObject::tr("Непредвиденная ошибка. Поток завершается");
            setStatus(kUnknown);
            return;
            break;
          }
        }
        else if ( kClientSender == mode_ ) {
          if ( false == startRecvThread() ) {
            error_log << QObject::tr("Неудалось создать канал приема сообщений");
            synchromutex_.lock();
            synchrocond_.wakeAll();
            synchromutex_.unlock();
            setStatus(kUnknown);
            return;
            break;
          }
        }
        setStatus(kConnected);
        synchromutex_.lock();
        synchrocond_.wakeAll();
        synchromutex_.unlock();
      case kConnected:
        if ( false == parseRpcMessages() ) {
          if ( kConnected!= status_ ) {
            error_log << QObject::tr("Ошибка получения rpc-сообщения.");
            setStatus(kUnknown);
            return;
          }
          break;
        }
        break;
      case kUnknown:
        error_log << QObject::tr("Неизвестная ошибка");
        break;
    }
  }
}
bool TcpSocket::parseSizeHandshake()
{
  if ( kIntSz > recvbuf_.size() ) {
    return false;
  }
  ::memcpy( &uuidsize_, recvbuf_.data(), kIntSz );
  recvbuf_ = recvbuf_.mid(kIntSz);
  if ( 256 < uuidsize_ ) {
    error_log << QObject::tr("Ошибка. Слишком большой размер для идентификационной строки.");
    setStatus(kUnknown);
    return false;
  }
  return true;
}

bool TcpSocket::parseIdentHandshake()
{
  if ( kSrvidentSz > recvbuf_.size() ) {
    return false;
  }
  if ( recvbuf_.left(kSrvidentSz) != kSrvident ) {
    error_log << QObject::tr("Постороннее соединение");
    setStatus(kUnknown);
    return false;
  }
  recvbuf_ = recvbuf_.mid(kSrvidentSz);
  return true;
}

bool TcpSocket::parseHandshake()
{
  if ( uuidsize_ - kSrvidentSz > recvbuf_.size() ) {
    return false;
  }
  QByteArray arr = recvbuf_.left( uuidsize_ - kSrvidentSz );
  QUuid uid(arr);
  recvbuf_ = recvbuf_.mid(uuidsize_ - kSrvidentSz);
  if ( true == uid.isNull() ) {
    error_log << QObject::tr("Неверный формат Uuid");
    setStatus(kUnknown);
    return false;
  }
  if ( kServer == mode_ ) {
    uuid_ = uid.toByteArray();
  }
  return true;
}

bool TcpSocket::parseRpcMessages()
{
  if ( 0 == recvbuf_.size() ) {
    return false;
  }
  parser_->appendData( recvbuf_.data(), recvbuf_.size() );
  recvbuf_ = QByteArray();
  MethodPack pack;

  while ( true == parser_->parseMessage(&pack) ) {
    {
      QMutexLocker lock(&mutex_);
      packlist_.append(pack);
    }
    emit messageReceived();
    QByteArray id;
    mutex_.lock();
    id = pack.id();
    mutex_.unlock();
    //debug_log << "poluchen msg id =" << id << "moy waitid =" << waitid_;
    if ( pack.id() == waitid_ ) {
      waitid_ = QByteArray();
      synchromutex_.lock();
      synchrocond_.wakeAll();
      synchromutex_.unlock();
    }
  }
  return true;
}

bool TcpSocket::writeMessage( const MethodPack& pack )
{
  QByteArray arr;
  arr += kRpcMsgStart; 
  int64_t arrsize = pack.size();
  QByteArray tmp( sizeof(arrsize), 0 );
  ::memcpy( tmp.data(), &arrsize, sizeof(arrsize) );
  arr += tmp;
  tmp = QByteArray( arrsize, '\0' );
  if ( false == pack.getData(&tmp) ) {
    error_log << QObject::tr("Не удалось сериализовать параметры удаленного вызова");
    return false;
  }
  arr += tmp;
  arr += kRpcMsgEnd;
  totalwritten_ += arr.size();
  if ( arr.size() != MnCommon::writeToSocket( sock_, arr ) ) {
    error_log << QObject::tr("Не удалось отправить rpc-сообщение");
    return false;
  }
  return true;
}

}
}
