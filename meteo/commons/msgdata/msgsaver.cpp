#include "msgsaver.h"

#include <qtimer.h>
#include <qthread.h>

#include <cross-commons/debug/tlog.h>

#include <meteo/commons/global/global.h>

#include "types.h"

namespace meteo {

MsgSaver::MsgSaver(QObject* parent)
  : QObject(parent)
{
}

void MsgSaver::slotInit()
{
  if ( nullptr != timer_ ) {
    timer_->stop();
    timer_->deleteLater();
    disconnect(timer_, 0, this, 0);
  }

  timer_ = new QTimer(this);
  timer_->setInterval(500);
  connect( timer_, &QTimer::timeout, this, &MsgSaver::slotRun );

  slotConnectToSrcData();
}

void MsgSaver::slotRun()
{
  if ( nullptr == channel_ ) { return; }

  if ( true == run_ ) { return; }
  run_ = true;

//  MsgQueueItem item = queue_->head(kReceiveBySrcData);
//  while ( item.isValid() ) {
//    if ( sendMsg_.size() > 1000 ) {
//      break;
//    }

//    if ( !channel_->isConnected() ) {
//      delete channel_;
//      channel_ = nullptr;
//      QTimer::singleShot(0, this, SLOT(slotConnectToSrcData()));
//      break;
//    }

//    surf::SaveMsgReply* reply = new surf::SaveMsgReply;
//    tlg::MessageNew mes = item.msg();

//    google::protobuf::RpcController* c = rpc::TController::createController();
//    google::protobuf::Closure* done = google::protobuf::NewCallback(this, &MsgSaver::msgProcessed, c, reply);
//    surf::SurfaceService_Stub stub(channel_);
//    stub.SaveMsg(c, &mes, reply, done);

//    sendMsg_[reply] = item.id();

//    controllers_[c] = done;

//    if ( false == item.nextStage() ) {
//      emit msgProcessed(item.id());
//    }

//    item = queue_->head(kReceiveBySrcData);
//  }

  run_ = false;
}

void MsgSaver::slotConnectToSrcData()
{
  if ( nullptr == channel_ ) {
    channel_ = meteo::global::serviceChannel(settings::proto::kSrcData);
    if ( nullptr == channel_ ) {
      QTimer::singleShot( 2000, this, SLOT(slotConnectToSrcData()) );
      return;
    }
    QObject::connect( channel_, SIGNAL( disconnected() ), this, SLOT( slotSrcDataDisconnected()) );
  }

  if ( !channel_->isConnected() ) {
    delete channel_;
    channel_ = nullptr;
    QTimer::singleShot( 2000, this, SLOT(slotConnectToSrcData()) );
    return;
  }

  info_log << tr("Соединение с srcdata.service установлено.");

  resendMsg();

  timer_->start();
}

void MsgSaver::slotSrcDataDisconnected()
{
  warning_log << tr("Разрыв связи с srcdata.service");

  qDeleteAll(controllers_.keys());
  qDeleteAll(controllers_.values());
  controllers_.clear();

  QTimer::singleShot( 5000, this, SLOT(slotConnectToSrcData()) );
}

void MsgSaver::msgProcessed(google::protobuf::RpcController* c, surf::SaveMsgReply* r)
{
  if ( sendMsg_.contains(r) ) {
    MsgQueueItem item = queue_->item(sendMsg_.value(r));

    if ( r->has_error() ) {
      error_log << QString::fromStdString(r->error());
    }
    else {
      tlg::MessageNew mes = item.msg();
      mes.mutable_metainfo()->set_id(r->id());
      mes.mutable_metainfo()->set_duplicate(r->duplicate());
      item.setMsg(mes);
    }

    if ( false == item.nextStage() ) {
      emit msgProcessed(item.id());
    }

    sendMsg_.remove(r);
  }

  controllers_.remove(c);

  delete r;
  delete c;
}

void MsgSaver::resendMsg()
{
  if ( sendMsg_.isEmpty() ) { return; }

  QHashIterator<surf::SaveMsgReply*,int> it(sendMsg_);
  while ( it.hasNext() ) {
    it.next();


    MsgQueueItem item = queue_->item(it.value());
    tlg::MessageNew mes = item.msg();

    var(item.id());

    // FIXME
//    google::protobuf::RpcController* c = rpc::TController::createController();
//    google::protobuf::Closure* done = google::protobuf::NewCallback(this, &MsgSaver::msgProcessed, c, it.key());
//    surf::SurfaceService_Stub stub(channel_);
//    stub.SaveMsg(c, &mes, it.key(), done);

//    controllers_[c] = done;

    if ( false == item.nextStage() ) {
      emit msgProcessed(item.id());
    }

  }
}

} // meteo
