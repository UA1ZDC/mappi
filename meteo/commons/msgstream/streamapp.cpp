#include "streamapp.h"

#include <cmath>

#include <qtimer.h>
#include <qthread.h>
#include <qpluginloader.h>

#include <cross-commons/app/paths.h>
#include <cross-commons/app/targ.h>
#include <cross-commons/debug/tlog.h>

#include <commons/textproto/pbtools.h>
#include <commons/textproto/tprototext.h>

#include <meteo/commons/global/log.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/proto/msgcenter.pb.h>
#include <meteo/commons/global/appstatusthread.h>
#include <meteo/commons/rpc/rpc.h>
#include <meteo/commons/msgparser/common.h>

#include "streammodule.h"

namespace meteo {

const int kRunTimeout = 500;

StreamApp::StreamApp(QObject* parent) :
  QObject(parent)
{

}

StreamApp::~StreamApp()
{
  delete channel_;
  channel_ = nullptr;
}

void StreamApp::setStatus(AppStatusThread* status)
{
  status_ = status;
  status_->setTitle(kInQueueSize, tr("Сообщений в очереди приёма"));
  status_->setTitle(kOutQueueSize, tr("Сообщений в очереди передачи"));
  status_->setTitle(kCenterQueueSize, tr("Сообщений в очереди ЦКС"));
  status_->setTitle(kCenterConnection, tr("Подключение к ЦКС"));
//status_->setParam(opt_.direction() == msgstream::kReceiver ? kInQueueSize : kOutQueueSize, 0);
  status_->setParam(kCenterQueueSize, 0);
}

bool StreamApp::init(const msgstream::Options& opt)
{
  opt_ = opt;
  maxQueueSize_ = opt_.center_window();
  slotConnectToCenter();
  QTimer* timer = new QTimer(this);
  connect( timer, SIGNAL(timeout()), SLOT(slotSendToCenter()) );
  timer->start(kRunTimeout);
  return true;
}

void StreamApp::slotSendToCenter()
{
  //debug_log<<"rslotSendToCenter"<<incoming_->size();

  if ( true == processIncoming_ ) { return; }
  if ( nullptr != outgoing_ ) status_->setParam(kOutQueueSize, outgoing_->size());
  if ( nullptr == incoming_ ) { return; }

  app::OperationState state = incoming_->size() >= maxQueueSize_ ? app::OperationState_WARN : app::OperationState_NORM;
  status_->setParam(kInQueueSize, incoming_->size(), state);
  if(0 == incoming_->size()) return;
  //state = toCenter_.size() >= maxQueueSize_ ? app::OperationState_WARN : app::OperationState_NORM;
  //status_->setParam(kCenterQueueSize, toCenter_.size(), state);
  state = notConfirmed_ >= maxQueueSize_ ? app::OperationState_WARN : app::OperationState_NORM;
  status_->setParam(kCenterQueueSize, notConfirmed_, state);
  processIncoming_ = true;

  while ( nullptr != channel_ && channel_->isConnected()  )
  {
//if ( incoming_->size() == 0 || toCenter_.size() >= maxQueueSize_ ) { break; }
    if ( incoming_->size() == 0 || notConfirmed_ >= maxQueueSize_ ) { break; }

    tlg::MessageNew tmp = incoming_->takeFirst();
    msgcenter::ProcessMsgRequest req;
    req.mutable_msg()->CopyFrom(tmp);
    if ( !channel_->remoteCall(&msgcenter::MsgCenterService::ProcessMsg, req, this, &StreamApp::msgProcessed) ) {
      break;
    }

//    toCenter_.insert(tlg::ahd(tmp), req.msg());
    ++notConfirmed_;

    state = incoming_->size() >= maxQueueSize_ ? app::OperationState_WARN : app::OperationState_NORM;
    status_->setParam(kInQueueSize, incoming_->size(), state);
//    state = toCenter_.size() >= maxQueueSize_ ? app::OperationState_WARN : app::OperationState_NORM;
//    status_->setParam(kCenterQueueSize, toCenter_.size(), state);
    state = notConfirmed_ >= maxQueueSize_ ? app::OperationState_WARN : app::OperationState_NORM;
    status_->setParam(kCenterQueueSize, notConfirmed_, state);
  }
  processIncoming_ = false;
}

void StreamApp::slotConnectToCenter()
{
  if ( nullptr == channel_ ) {
    channel_ = meteo::global::serviceChannel(settings::proto::kMsgCenter);
    if ( nullptr == channel_ ) {
      error_log << msglog::kServiceNotFound.arg(global::serviceTitle(settings::proto::kMsgCenter));
      QTimer::singleShot( opt_.reconnect_timeout()*1000, this, SLOT(slotConnectToCenter()) );
      return;
    }
    QObject::connect( channel_, SIGNAL( disconnected() ), this, SLOT( slotCenterDisconnected() ) );
  }
  if ( nullptr == channel_ || false == channel_->isConnected() ) {
    delete channel_;
    channel_ = nullptr;
    QTimer::singleShot( opt_.reconnect_timeout()*1000, this, SLOT(slotConnectToCenter()) );
    return;
  }
  info_log << tr("Соединение с ЦКС установлено.");
  if ( msgstream::kReceiver != opt_.direction() ) { subscribeToReceiveMsg(); }
  status_->setParam(kCenterConnection, tr("Есть"));
}

void StreamApp::msgProcessed(msgcenter::ProcessMsgReply* r)
{
  if ( r->has_error() ) {
    error_log << tr("Ошибка при обработке сообщения.");
  }

//  toCenter_.remove(*r);
  --notConfirmed_;

//  app::OperationState state = toCenter_.size() >= maxQueueSize_ ? app::OperationState_WARN : app::OperationState_NORM;
//  status_->setParam(kCenterQueueSize, toCenter_.size(), state);
  app::OperationState state = notConfirmed_ >= maxQueueSize_ ? app::OperationState_WARN : app::OperationState_NORM;
  status_->setParam(kCenterQueueSize, notConfirmed_, state);
  slotSendToCenter();
  delete r;
}

void StreamApp::receiveOutboundMsg(tlg::MessageNew* r)
{
  if ( nullptr == outgoing_ ) {
    return;
  }

  outgoing_->append(*r);

  while ( outgoing_->size() > maxQueueSize_ ) {
    outgoing_->removeFirst();
  }

  status_->setParam(kOutQueueSize, outgoing_->size());
  emit newOutgoingMsg();
}

void StreamApp::subscribeToReceiveMsg()
{
  if ( nullptr == channel_ ) { return; }

  if ( !channel_->isConnected() ) { return; }

  msgcenter::DistributeMsgRequest req;
  req.set_uid(opt_.id());
  if ( !channel_->subscribe(&msgcenter::MsgCenterService::DistributeMsg, req, this, &StreamApp::receiveOutboundMsg) ) {
    error_log << tr("Не удалось подписаться на получение телеграмм");
  }
}

void StreamApp::slotCenterDisconnected()
{
  warning_log << tr("Разрыв связи с ЦКС.");

  if ( nullptr != incoming_ ) {
//    incoming_->prepend(toCenter_.values());
  }

  // удаляем канал и отключаемся от сигналов, чтобы избежать повторного slotCenterDisconnected()
  channel_->disconnect(this);
  channel_->deleteLater();
  channel_ = nullptr;

//  toCenter_.clear();
  notConfirmed_ = 0;

  status_->setParam(kCenterConnection, tr("Нет"), app::OperationState_ERROR);

  QTimer::singleShot( opt_.reconnect_timeout()*1000, this, SLOT(slotConnectToCenter()) );
}

} // meteo
