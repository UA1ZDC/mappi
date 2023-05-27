#include "httprequest.h"
#include <cross-commons/debug/tlog.h>

HttpRequest::HttpRequest(const QString &request, const QString &request_id, QObject* p)
  : QObject(p), request_id_(request_id)
{
  setRequest(request);
}

HttpRequest::HttpRequest(const QString &request, const QString &request_id,const QString &cron_interval, QObject* p)
  : QObject(p), request_id_(request_id)
{
  setRequest(request);
  setCronInterval(cron_interval);
}

HttpRequest::~HttpRequest()
{
  clear();
}

QString HttpRequest::requestId(){
  return request_id_;
}


void HttpRequest::setCronInterval(const QString &interval){
  cron_.setInterval(interval);
}

void HttpRequest::setRequest(const QString &request){
  request_ = request;
}

void HttpRequest::setStatusParamId(int spid)
{
  status_param_number_ = spid;
}

void HttpRequest::clear()
{
  //debug_log << "HttpRequest::clear";
  if(nullptr != nreply_){
    emit downloadProcess(status_param_number_, 0, 0);
    nreply_->deleteLater();
    nreply_ = nullptr;
  }
  if(nullptr != nreply2_){
    nreply2_->deleteLater();
    nreply2_ = nullptr;
  }

}

void HttpRequest::slotTimeOut()
{
  debug_log << "slotTimeOut: ";
  last_error_ = tr("Превышено время ожидания");
  emit replyError(this);
  clear();

}

const QString &HttpRequest::getLastError(){
  return last_error_;
}

bool HttpRequest::go(QNetworkAccessManager *am)
{
  am_ = am;
  if(cron_.isValid()){
    QTimer::singleShot(cron_.msecsToNext(),this,SLOT(slotRunRequest()));
  } else {
    slotRunRequest();
  }
  return true;
}



void HttpRequest::slotRunRequest()
{
  if(nullptr == am_){
    return;
  }
  //debug_log << "request: "<< request_;
  if(nullptr != nreply_){
    nreply_->deleteLater();
  }
  nreply_ = am_->get(QNetworkRequest(QUrl(request_)));
  emit requestSend();
  connect(nreply_, &QNetworkReply::finished, this, &HttpRequest::slotGetData);
  // connect(nreply_, &QNetworkReply::errorOccurred, this, &HttpRequest::slotError);
  connect(nreply_, &QNetworkReply::downloadProgress, this, &HttpRequest::slotDownloadProgress);
}

void HttpRequest::slotDownloadProgress(qint64 bytesSent, qint64 bytesTotal)
{
  //debug_log<<"slotUploadProgress";
  bytes_sent_ = bytesSent;
  bytes_total_ = bytesTotal;
  emit downloadProcess(status_param_number_, bytes_sent_, bytes_total_);
}

void HttpRequest::slotError(QNetworkReply::NetworkError error)
{
  // debug_log << "HttpRequest::slotError";
  last_error_ = tr("Ошибка при выполнении запроса")+QString::number(error);
  emit replyError(this);
  clear();
  restart();
}

QByteArray HttpRequest::getReply(){
  return reply_;
}

void HttpRequest::slotGetData()
{
  emit replyReady(this);
  clear();
}

void HttpRequest::restart(){
  if(cron_.isValid()){
    QTimer::singleShot(cron_.msecsToNext(),this,SLOT(slotRunRequest()));
  }
}

