#include "socksp2greceiver.h"
#include "socksp2g.h"

#include <QtNetwork/qhostaddress.h>

#include <cross-commons/debug/tlog.h>
#include <meteo/commons/global/appstatusthread.h>
#include <commons/funcs/mnio.h>

namespace meteo {

Socksp2gReceiver::Socksp2gReceiver(const msgstream::Options& opt,
                                 TSList<tlg::MessageNew> *outgoing,
                                 TSList<tlg::MessageNew> *incoming,
                                 AppStatusThread *status,
                                 QObject* p ):
  Socksp2gBase(opt,outgoing,incoming,status,p)
{
initTimers();
}

Socksp2gReceiver::~Socksp2gReceiver()
{
  stopv();

}

void Socksp2gReceiver::slotStart()
{
  if(nullptr != reconnectTimer_) reconnectTimer_->stop();
  deleteSocket();
  if ( nullptr == socket_ ) {
      socket_ = new QTcpSocket(this);
      connect( socket_, SIGNAL(readyRead()), SLOT(slotReadyRead()) );
      connect( socket_, SIGNAL(connected()), SLOT(slotConnected()) );
      connect( socket_, SIGNAL(error(QAbstractSocket::SocketError)),
               SLOT(slotSocketError(QAbstractSocket::SocketError)) );
    }

  if ( QAbstractSocket::ConnectedState == socket_->state() ) {
      info_log << tr("<-- подключено к %1:%2").arg(host_).arg(port_);
      return;
    }
  socket_->abort();
  info_log << tr("Подключение к серверу %1:%2").arg(host_).arg(port_);
  socket_->connectToHost(host_, port_);
}

void Socksp2gReceiver::slotConnected()
{
  state_ = socksp2g::kStart;
  QTcpSocket* sock = qobject_cast<QTcpSocket*>(sender());
  if ( 0 == sock ) {
      return;
    }
  int s = 0;
  if(nullptr == socket_ || ( 0 > (s = socket_->socketDescriptor()) ) ) {
      error_log << QObject::tr("Непредвиденная ошибка. Дескриптор < 0");
      deleteSocket();
      slotStart();
      return;
    }
  MnCommon::setSocketOptions(s);
  info_log << tr("Установлено соединение с сервером %1:%2")
              .arg(sock->peerAddress().toString())
              .arg(sock->peerPort());
  setMagic();
  slotSendRR();
  if(nullptr != reconnectTimer_) reconnectTimer_->stop();
  if(nullptr != workTimer_) workTimer_->start();
  if(nullptr != status_) status_->setParam(kLinkStatus, tr("Установлено"));
}

void Socksp2gReceiver::stop()
{
  stopv();

}

void Socksp2gReceiver::stopv()
{
  deleteSocket();
  if(nullptr == outgoing_) return;
  foreach (auto tlg, sentList_) {
      outgoing_->prepend(tlg.info.msg());
    }
  sentList_.clear();
  clearData();
  if(nullptr != status_) status_->setParam(kLinkStatus, tr("Отсутствует"),app::OperationState_ERROR);
}

void Socksp2gReceiver::slotReceiveTimeout(){
  stop();
  QThread::msleep(1000);
  slotStart();
}

void Socksp2gReceiver::slotReconnectTimeout(){
  stop();
  QThread::msleep(1000);
  slotStart();
}

} // meteo
