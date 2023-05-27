#include "unimasreceiver.h"
#include "unimasstreamserver.h"

#include <arpa/inet.h>

#include <qtimer.h>
#include <qprocess.h>
#include <qdatetime.h>

#include <cross-commons/app/paths.h>
#include <cross-commons/debug/tlog.h>
#include <commons/funcs/mnio.h>
#include <meteo/commons/msgparser/msgmetainfo.h>
#include <meteo/commons/global/global.h>


namespace meteo {

UnimasReceiver::UnimasReceiver(const msgstream::Options& opt,
                               TSList<tlg::MessageNew>* outgoing,
                               TSList<tlg::MessageNew>* incoming,
                               AppStatusThread* status,
                               QObject *p) :
  UnimasBase(opt,outgoing,incoming,status, p )
{
}

UnimasReceiver::~UnimasReceiver()
{
}

void UnimasReceiver::slotStart()
{
  reconnectTimer_->stop();
  if( nullptr == socket_ ){
      socket_ = new QTcpSocket;
      connect( socket_, SIGNAL(readyRead()), SLOT(slotReadyRead()) );
      connect( socket_, SIGNAL(connected()), SLOT(slotConnected()) );
      connect( socket_, SIGNAL(error(QAbstractSocket::SocketError )), SLOT(slotSocketError(QAbstractSocket::SocketError)) );
    }
  state_  = kStart;
  headPacket_.clear();
  endPacket_.clear();
  raw_.clear();
  if ( QAbstractSocket::ConnectedState == socket_->state() ) {
    info_log << tr("<-- подключено к %1:%2").arg(host_).arg(port_);
    return ;
  }
  socket_->abort();
  info_log << tr("Подключение к серверу %1:%2").arg(host_).arg(port_);
  socket_->connectToHost(host_, port_);
  return ;
}

void UnimasReceiver::slotReceiveTimeout()
{
  warning_log << tr("Превышено время ожидания приёма");
  closeSession();
  QThread::msleep(1000);
  slotStart();
}

void UnimasReceiver::slotReconnectTimeout()
{
  warning_log << tr("Превышено время ожидания соединения");
  closeSession();
  QThread::msleep(1000);
  slotStart();
}

void UnimasReceiver::slotConnected()
{
  QTcpSocket* sock = qobject_cast<QTcpSocket*>(sender());
  if ( 0 == sock ) {
    return;
  }
  int s = 0;
  if(nullptr == socket_ || ( 0 > (s = socket_->socketDescriptor()) ) ) {
      error_log << QObject::tr("Непредвиденная ошибка. Дескриптор < 0");
      closeSession();
      slotStart();
      return;
    }
  MnCommon::setSocketOptions(s);
  info_log << tr("Установлено соединение с сервером %1:%2")
                .arg(sock->peerAddress().toString())
                .arg(sock->peerPort());
  sendRR();
  if(nullptr != status_) status_->setParam(kLinkStatus, tr("Установлено"));
  if(nullptr != reconnectTimer_) reconnectTimer_->stop();
  if(nullptr != work_timer_) work_timer_->start();
}

void UnimasReceiver::stop()
{
  deleteSocket();
  if(nullptr == outgoing_) return;
  foreach (auto tlg, sentList_) {
      outgoing_->prepend(tlg.orig);
    }
  sentList_.clear();
  headPacket_.clear();
  endPacket_.clear();
  raw_.clear();
  if(nullptr != status_) status_->setParam(kLinkStatus, tr("Отсутствует"),app::OperationState_ERROR);
  isWriten = true;
}

} // meteo
