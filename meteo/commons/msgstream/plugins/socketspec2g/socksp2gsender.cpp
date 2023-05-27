#include "socksp2gsender.h"

#include <QtNetwork/qtcpsocket.h>
#include <commons/funcs/mnio.h>

#include <cross-commons/debug/tlog.h>

namespace meteo {

Socksp2gSender::Socksp2gSender(qintptr socket,
                             const msgstream::Options& opt,
                             TSList<tlg::MessageNew> *outgoing,
                             TSList<tlg::MessageNew> *incoming,
                             AppStatusThread *status,
                             QObject* parent):
  Socksp2gBase(opt,outgoing,incoming,status,parent),
  socket_descriptor_(socket)
{
}

Socksp2gSender::~Socksp2gSender()
{
  stopv();
}

void Socksp2gSender::slotStart()
{
  state_ = socksp2g::kStart;
  if(nullptr != socket_){
      deleteSocket();
    }
  socket_ = new QTcpSocket;
  if (!socket_->setSocketDescriptor(socket_descriptor_)){
      error_log<<tr("Ошибка создания сокета");
      stop();
    } else {
      info_log << tr("Установлено соединение с %1:%2").arg(socket_->peerAddress().toString()).arg(socket_->peerPort());
      MnCommon::setSocketOptions(socket_descriptor_);
      connect(socket_, SIGNAL( error(QAbstractSocket::SocketError) ),
              this, SLOT( slotSocketError(QAbstractSocket::SocketError) ) );
      connect( socket_, SIGNAL(readyRead()), SLOT(slotReadyRead()) );
      setMagic();
      if(nullptr != status_) status_->setParam(kLinkStatus, tr("Установлено"));
      slotSendRR();
      initTimers();
    }
}

void Socksp2gSender::stop()
{
  stopv();
}

void Socksp2gSender::stopv()
{
  deleteSocket();
  if(nullptr == outgoing_) return;
  foreach (auto tlg, sentList_) {
      outgoing_->prepend(tlg.info.msg());
    }
  sentList_.clear();
  clearData();
  if(nullptr != status_) status_->setParam(kLinkStatus, tr("Отсутствует"),app::OperationState_ERROR);
  emit finished();
}

void Socksp2gSender::slotReceiveTimeout(){
  stop();

}

void Socksp2gSender::slotReconnectTimeout(){
  stop();
}


} // meteo
