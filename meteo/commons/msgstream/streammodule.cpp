#include "streammodule.h"
#include <qprocess.h>

#include <cross-commons/app/paths.h>

namespace meteo {


  StreamModule::StreamModule(QObject* parent)
    : QObject(parent),
      status_(nullptr),
      incoming_(nullptr),
      outgoing_(nullptr),
      lc_(QDateTime::currentDateTime())
  {

  }


  void StreamModule::setIncoming(TSList<tlg::MessageNew>* list) {
    incoming_ = list;
  }
  void StreamModule::setOutgoing(TSList<tlg::MessageNew>* list) {
    outgoing_ = list;
  }

  void StreamModule::appendNewIncoming(const tlg::MessageNew& new_incoming){
    if(nullptr != incoming_){
        incoming_->append(new_incoming);
        emit newIncomingMsg();
      }
  }


  void StreamModule::slotMsgReceived( qint64 sended){
    recvCount_ += sended;
    recvCountToday_ += sended;
    if(nullptr != status_) {
        status_->setParam(kRecvCount,recvCountToday_, recvCount_);
      }
  }

  void StreamModule::slotSizeReceived( qint64 wsz ){
    recvSize_      += wsz;
    recvSizeToday_ += wsz;
    if(nullptr != status_) {
        status_->setParam(kRecvSize, AppStatusThread::sizeStr(recvSizeToday_)+"/"+AppStatusThread::sizeStr(recvSize_));
      }
  }

  void StreamModule::slotMsgSended( qint64 sended){
    sendCount_ += sended;
    sendCountToday_ += sended;
    if(nullptr != status_) {
        status_->setParam(kSendCount, sendCountToday_,sendCount_);
      }
  }

  void StreamModule::slotSizeSended( qint64 wsz ){
    sendSize_      += wsz;
    sendSizeToday_ += wsz;
    if(nullptr != status_) {
        status_->setParam(kSendSize, AppStatusThread::sizeStr(sendSizeToday_)+"/"+AppStatusThread::sizeStr(sendSize_));
      }
  }

  void StreamModule::slotConfirmSended( qint64 wsz ){
    confirmSended_      += wsz;
    confirmSendedToday_ += wsz;
    if(nullptr != status_) {
        status_->setParam(kConfirmSended, confirmSendedToday_,confirmSended_);
      }
  }

  void StreamModule::slotConfirmReceived( qint64 wsz ){
    confirmRecv_      += wsz;
    confirmRecvToday_ += wsz;
    if(nullptr != status_) {
        status_->setParam(kConfirmRecv,confirmRecvToday_,confirmRecv_);
      }
  }

  void StreamModule::timerEvent(QTimerEvent* event)
  {
    Q_UNUSED( event );
    QDateTime dt = QDateTime::currentDateTime();
    if ( lc_.date() != dt.date() ) {
        sendSizeToday_ = 0;
        sendCountToday_ = 0;
        recvSizeToday_ = 0;
        recvCountToday_ = 0;
        confirmSendedToday_ = 0;
        confirmRecvToday_ = 0;
      }
    lc_ = dt;
  }

  void StreamModule::resetChecker()
  {
    lc_= QDateTime::currentDateTime();
  }

} // meteo
