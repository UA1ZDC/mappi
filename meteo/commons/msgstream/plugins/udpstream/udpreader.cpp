#include "udpreader.h"

#include <cmath>
#include <climits>
#include <cross-commons/debug/tlog.h>

namespace meteo {

UdpReader::UdpReader(QObject* p)
  : QObject(p)
{
  qRegisterMetaType<QAbstractSocket::SocketState>();
  qRegisterMetaType<QAbstractSocket::SocketError>();
  udpSocket_ = new QUdpSocket(this);

}

UdpReader::~UdpReader()
{
  udpSocket_->close();
  delete udpSocket_;
  udpSocket_ = nullptr;
}

void UdpReader::setMulticast(bool ism){
  is_multicast_ = ism;
}


void UdpReader::setParams(const QString & h, int p, int datagram_size)
{
  host_=h;
  port_=p;
  datagram_size_ = datagram_size*2;
  datagram_.resize(datagram_size_);
}

bool UdpReader::reconnect()
{
  udpSocket_->close();
  udpSocket_->deleteLater();
  udpSocket_ = nullptr;
  return slotInit();
}

bool UdpReader::slotInit()
{
  if(nullptr == udpSocket_) return false;
  connect(udpSocket_, SIGNAL(error(QAbstractSocket::SocketError)),
          this, SLOT(slotSocketError(QAbstractSocket::SocketError)));
  connect(udpSocket_, SIGNAL(stateChanged(QAbstractSocket::SocketState)),
          this, SLOT(slotSocketStateChanged(QAbstractSocket::SocketState)));
  connect(udpSocket_, SIGNAL(readyRead()),
          this, SLOT(slotProcessPendingDatagrams()));
  if(true == is_multicast_){
    isb_ = (udpSocket_->bind(QHostAddress::AnyIPv4, port_, QUdpSocket::ShareAddress) &&
            udpSocket_->joinMulticastGroup(QHostAddress(host_)));
  } else {
    isb_ = udpSocket_->bind(QHostAddress(host_), port_);
  }
  return isb_ ;
}

QAbstractSocket::SocketState UdpReader::socketState()
{
  if(nullptr == udpSocket_){
    return QAbstractSocket::UnconnectedState;
  }
  QAbstractSocket::SocketState st = udpSocket_->state();
  slotSocketStateChanged(st);
  return st;
}

bool UdpReader::isBind() {
  return (QAbstractSocket::BoundState == udpSocket_->state());
};


void UdpReader::slotSocketStateChanged(QAbstractSocket::SocketState st)
{
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
  emit stateSignal(strstate);

}

void UdpReader::slotSocketError(QAbstractSocket::SocketError )
{
  if(nullptr != udpSocket_) {
    error_log << "SOCKET ERROR" << udpSocket_->errorString();
    emit errorSignal(udpSocket_->errorString());
  } else {
    error_log << "SOCKET ERROR" ;
    emit errorSignal("SOCKET ERROR");
  }

}

bool UdpReader::takeFirst(QByteArray*ba){
  if(packets_list_.empty()) return false;
  *ba = packets_list_.takeFirst();
  return true;
}

void UdpReader::slotProcessPendingDatagrams()
{
  while (nullptr != udpSocket_ && udpSocket_->hasPendingDatagrams()) {
    int ds = udpSocket_->pendingDatagramSize();
    /* if(ds > datagram_size_){
        datagram_size_ = ds;
        datagram_.resize(datagram_size_);
      }*/
    udpSocket_->readDatagram(datagram_.data(), ds);
    packets_list_.append(datagram_.left(ds));
  }
  emit newData();
}


} // meteo
