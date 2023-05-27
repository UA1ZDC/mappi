#include "httpstream.h"

#include <cmath>
#include <climits>

#include <qelapsedtimer.h>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <cross-commons/debug/tlog.h>

#include <cross-commons/debug/tmap.h>
#include <commons/textproto/pbtools.h>
#include <commons/textproto/tprototext.h>

#include <meteo/commons/msgparser/common.h>
#include <meteo/commons/msgparser/msgmetainfo.h>
#include <meteo/commons/msgparser/tlgparser.h>
#include <cross-commons/app/paths.h>

namespace meteo {

HttpStream::HttpStream(QObject* p)
  : StreamModule(p)
{
  status_ = nullptr;
}

HttpStream::~HttpStream()
{
  delete status_;
  status_ = nullptr;
  delete timer_;
  timer_ = nullptr;
  delete []status_param_id;
  status_param_id = nullptr;
}

void HttpStream::setOptions(const msgstream::Options& opt)
{
  opt_ = opt;
}

bool HttpStream::slotInit()
{
  if ( !lastCheck_.isValid() ) {
    status_->setTitle(kRequestsCount, tr("Запросов отправлено сегодня/всего"));
    status_->setTitle(kReplyCount, tr("Ответов получено сегодня/всего"));
    status_->setTitle(kReplySize, tr("Принято данных сегодня/всего"));
    status_->setTitle(kErrorReplyCount, tr("Ответов с ошибками"));
    status_->setTitle(kRecvQueue, tr("Ожидание"));

    status_->setTitle(kOperation, tr("Операция"));
    status_->setParam(kRequestsCount, 0,0);
    status_->setParam(kReplyCount, 0,0);
    status_->setParam(kErrorReplyCount, 0);
  }
  manager_ = new QNetworkAccessManager(this);

  lastCheck_ = QDateTime::currentDateTime();
  QObject::startTimer(kTimeToLastActivity);

  if ( 0 == incoming_ ) { return false; }
  parser_ = new TlgParser( QByteArray(nullptr), opt_.split_stucked() );
  //maxQueue_ = 4;// opt_.max_queue();
  maxQueue_ = opt_.max_queue();
  status_param_id = new int[maxQueue_];
  for(int i =0;i< maxQueue_;++i){
    status_param_id[i] = -1;
  }

  id_ = pbtools::toQString(opt_.id());
  if(opt_.has_file_recv()&& opt_.file_recv().has_path()){
    filePath_ = pbtools::toQString(opt_.file_recv().path());
    QDir dir;
    if(false == dir.mkpath(filePath_)){
      error_log << "ошибка при создании директории";
    }
  }
  if ( nullptr != timer_ ) {
    timer_->deleteLater();
  }
  timer_ = new QTimer();
  run_ = false;
  QObject::connect( timer_, SIGNAL(timeout()), SLOT(slotTimeout()) );
  int timeoout = 30000;
  if(0 < opt_.sockspec().session_timeout()){
    timeoout = opt_.sockspec().session_timeout()*1000;
  }
  timer_->start(timeoout);
  slotProcess();
  return true;
}

void HttpStream::slotTimeout()
{
  if (maxQueue_ <= count_request_in_queue_) {
    status_->setParam(kRecvQueue, QString::number(count_request_in_queue_)+tr(" (Предел по количеству запросов в очереди)"),app::OperationState_WARN);
    return;
  }
  if (run_) {
    return;
  }
  run_ = true;
  slotProcess();
  run_ = false;
}

void HttpStream::slotDownloadProcess(int status_param_number, qint64 bytesSent, qint64 bytesTotal){
  if(false == status_->hasId(status_param_number)){
    status_->setTitle(status_param_number, tr("Принято по запросу сейчас/всего"));
  }
  if(0 < bytesTotal){
    status_->setParam(status_param_number,AppStatusThread::sizeStr(bytesSent)+"/"+ AppStatusThread::sizeStr(bytesTotal));
  } else {
    status_->setParam(status_param_number,AppStatusThread::sizeStr(bytesSent));
  }
  if(bytesSent == bytesTotal){
    status_param_id[status_param_number-kRecvStatus] = -1;
    status_->unsetParam(status_param_number);
  }
}

void HttpStream::slotErrorReply(HttpRequest *req)
{
  status_->setParam(kOperation, req->getLastError(),app::OperationState_ERROR);
  status_->setParam(kErrorReplyCount, ++errorReplyCount_);
  status_->setParam(kRecvQueue, --count_request_in_queue_);
  if(false == req->isCronable()){
    req->deleteLater();
  } else{
    req->restart();
  }
}

void HttpStream::slotParseReply(HttpRequest *req)
{
  status_->setParam(kOperation, tr("Обработка ответа"));
  status_->setParam(kReplyCount, ++replyCountToday_, ++replyCount_);
  status_->setParam(kRecvQueue, --count_request_in_queue_);

  const QByteArray &reply = req->getReply();
  replySize_ += reply.size();
  replySizeToday_ += reply.size();
  status_->setParam(kReplySize, AppStatusThread::sizeStr(replySizeToday_)+"/"+ AppStatusThread::sizeStr(replySize_));
  if(nullptr == incoming_){
    error_log<<tr("Обработчик не установлен!");
    //  debug_log << "slotParseReply req->deleteLater";
    if(false == req->isCronable()){
      req->deleteLater();
    } else{
      req->restart();
    }
    return;
  }

  QByteArray ba =  magicBegin_+reply+magicEnd_;
  tlg::MessageNew msg;
  msg.set_msgtype(msg_type_.toStdString());
  msg.set_format(tlg::kOther);
  msg.set_msg(pbtools::toBytes(ba));
  MsgMetaInfo meta;
  //TODO добавить описание заголовка
  /* if( !_metaTemplate.isEmpty() ){
    meta = MsgMetaInfo(_metaTemplate, info.fileName());
  }*/
  meta.t1 = 'y';
  if ( !meta.t1.isEmpty() ) { msg.mutable_header()->set_t1(meta.t1.toUtf8().constData()); }
  if ( !meta.t2.isEmpty() ) { msg.mutable_header()->set_t2(meta.t2.toUtf8().constData()); }
  if ( !meta.a1.isEmpty() ) { msg.mutable_header()->set_a1(meta.a1.toUtf8().constData()); }
  if ( !meta.a2.isEmpty() ) { msg.mutable_header()->set_a2(meta.a2.toUtf8().constData()); }
  if ( !meta.ii.isEmpty() ) { msg.mutable_header()->set_ii(meta.ii.toInt()); }
  if ( !meta.cccc.isEmpty() ) { msg.mutable_header()->set_cccc(meta.cccc.toUtf8().constData()); }
  if ( !meta.yygggg.isEmpty() ) { msg.mutable_header()->set_yygggg(meta.yygggg.toUtf8().constData()); }

  if ( !msg.header().has_t1() && opt_.has_predef_t1() ) { msg.mutable_header()->set_t1(opt_.predef_t1()); }
  if ( !msg.header().has_t2() && opt_.has_predef_t2() ) { msg.mutable_header()->set_t2(opt_.predef_t2()); }
  if ( !msg.header().has_a1() && opt_.has_predef_a1() ) { msg.mutable_header()->set_a1(opt_.predef_a1()); }
  if ( !msg.header().has_a2() && opt_.has_predef_a2() ) { msg.mutable_header()->set_a2(opt_.predef_a2()); }
  if ( !msg.header().has_ii() && opt_.has_predef_ii() ) { msg.mutable_header()->set_ii(opt_.predef_ii()); }
  if ( !msg.header().has_cccc() && opt_.has_predef_cccc() ) { msg.mutable_header()->set_cccc(opt_.predef_cccc()); }

  msg.mutable_metainfo()->set_from(pbtools::toString(id_));

  ba = tlg::proto2tlg(msg);
  parser_->setData(ba);

  if( true == parser_->parseNextMessage(&msg))
  {
    MsgMetaInfo meta(msg);
    QDateTime dt = meta.calcConvertedDt();
    msg.mutable_metainfo()->set_converted_dt(dt.toString(Qt::ISODate).toUtf8().constData());
    msg.mutable_metainfo()->set_from(id_.toStdString());
    incoming_->append(msg);
  }
  if(false == req->isCronable()){
    req->deleteLater();
  } else{
    req->restart();
  }
  status_->setParam(kOperation, tr("Ожидание"));
  return;
}

void HttpStream::timerEvent(QTimerEvent* event)
{
  Q_UNUSED( event );
  QDateTime dt = QDateTime::currentDateTime();
  if ( lastCheck_.date() != dt.date() ) {
    replySizeToday_ = 0;
    requestCountToday_ = 0;
    replyCountToday_ = 0;
  }
  lastCheck_ = dt;
}

int HttpStream::getNextStatusParamId()
{
  int i = 0;
  for(; i < maxQueue_-1;++i){
    if(-1 == status_param_id[i]){
      status_param_id[i] = i;
      break;
    }
  }
  if(i == maxQueue_-1 ){
    warning_log << "Внимание! Количество запросов за один сеанс превышает значение, указанное в настройках!" << maxQueue_;
  }
  //debug_log<<"getNextStatusParamId" <<kRecvStatus+i;
  return kRecvStatus+i;
}

void HttpStream::createProcess(HttpRequest * request)
{
  status_->setParam(kOperation, tr("Отправка запросa"));
  int spid = getNextStatusParamId();
  request->setStatusParamId(spid);
  status_->setTitle(spid, tr("Принято по запросу сейчас/всего"));
  connect(request, &HttpRequest::replyReady, this, &HttpStream::slotParseReply);
  connect(request, &HttpRequest::replyError, this, &HttpStream::slotErrorReply);
  connect(request, &HttpRequest::downloadProcess, this, &HttpStream::slotDownloadProcess);
  connect(request, &HttpRequest::requestSend, this, &HttpStream::slotNewRequestCounter);
  request->go(manager_);
  status_->setParam(kOperation, tr("Ожидание ответa"));
}

void HttpStream::slotNewRequestCounter(){
  status_->setParam(kRecvQueue, ++count_request_in_queue_);
  status_->setParam(kRequestsCount, ++requestCountToday_, ++requestCount_);
}


//for testing
} // meteo
