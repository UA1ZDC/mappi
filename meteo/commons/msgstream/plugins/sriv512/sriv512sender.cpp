#include "sriv512sender.h"

#include <QtNetwork/qtcpsocket.h>
#include <commons/funcs/mnio.h>

#include <cross-commons/debug/tlog.h>

namespace meteo {

Sriv512Sender::Sriv512Sender(qintptr socket,
                             const msgstream::Options& opt,
                             TSList<tlg::MessageNew> *outgoing,
                             TSList<tlg::MessageNew> *incoming,
                             AppStatusThread *status,
                             QObject* parent):
  Sriv512Base(opt,outgoing,incoming,status,parent),
  socket_descriptor_(socket)
{
}

Sriv512Sender::~Sriv512Sender()
{
  stopv();
}

void Sriv512Sender::slotStart()
{
  state_ = sriv512::kStart;
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

void Sriv512Sender::stop()
{
  stopv();
}

void Sriv512Sender::stopv()
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

void Sriv512Sender::slotReceiveTimeout(){
  stop();

}

void Sriv512Sender::slotReconnectTimeout(){
  stop();
}


} // meteo
