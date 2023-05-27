#include "unimassender.h"

#include <cstring>

#include <QtNetwork/qtcpsocket.h>
#include <commons/funcs/mnio.h>
#include <meteo/commons/msgstream/streammodule.h>
#include <cross-commons/debug/tlog.h>
#include <commons/mathtools/mnmath.h>

#include <meteo/commons/proto/msgparser.pb.h>
#include <meteo/commons/msgparser/common.h>

namespace meteo {

UnimasSender::UnimasSender(int socket,const msgstream::Options& opt,
                           TSList<tlg::MessageNew>* outgoing,
                           TSList<tlg::MessageNew>* incoming,
                           AppStatusThread* status,
                           QObject *p) :
UnimasBase(opt,outgoing,incoming,status, p ),
  socket_descriptor_(socket)
{
}

UnimasSender::~UnimasSender(){
}

void UnimasSender::slotStart()
{
  if(nullptr != socket_){
      deleteSocket();
    }
  socket_ = new QTcpSocket();
  if (!socket_->setSocketDescriptor(socket_descriptor_)){
      error_log << tr("Ошибка создания сокета");
    } else {
      info_log << tr("Установлено соединение с %1:%2").arg(socket_->peerAddress().toString()).arg(socket_->peerPort());
      MnCommon::setSocketOptions(socket_descriptor_);
      connect( socket_, SIGNAL(readyRead()), SLOT(slotReadyRead()) );
      connect( socket_, SIGNAL(error(QAbstractSocket::SocketError )), SLOT(slotSocketError(QAbstractSocket::SocketError)) );
      if(nullptr != status_) status_->setParam(kLinkStatus, tr("Установлено"));
      sendRR();
      initTimers();
    }
  //exec();
}

void UnimasSender::stop()
{
  deleteSocket();
  if(nullptr == outgoing_) return;
  foreach (TlgBox tlg, sentList_) {
      outgoing_->prepend(tlg.orig);
    }
  sentList_.clear();
  if(nullptr != status_) status_->setParam(kLinkStatus, tr("Отсутствует"),app::OperationState_ERROR);
  emit finished();
  isWriten = true;
}

void UnimasSender::slotReceiveTimeout()
{
  warning_log << tr("Превышено время ожидания приёма");
  stop();
}

void UnimasSender::slotReconnectTimeout()
{
  warning_log << tr("Превышено время ожидания соединения");
  stop();
}

void UnimasSender::slotResendTimeout()
{
  resend_timer_->stop();
  tryResendMsg();
  resend_timer_->start();
}

} // meteo
