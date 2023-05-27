#include "dwnldrequest.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>


#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkRequest>
#include <QDomDocument>
#include <QXmlStreamReader>

#include <cross-commons/debug/tlog.h>

DwnldRequest::DwnldRequest(const QString &request, const QString &request_id, QObject *p)
  :HttpRequest(request,request_id, p)
{
  setRequest( request  );
}

DwnldRequest::~DwnldRequest(){
  //debug_log <<"~DwnldRequest()";
}


/**
 * @brief Обрабатываем ошибки
 *
 */
void DwnldRequest::slotError(){
  if(nullptr ==  nreply_) {
    return;
  }
  last_error_ = tr("Ошибка выполнения запроса")+ nreply_->errorString();
  error_log <<last_error_;
  clear();
}


void DwnldRequest::slotGetData(){
  if(nullptr ==  nreply_) {
    return;
  }
  if(QNetworkReply::NoError == nreply_->error()){

    reply_ = nreply_->readAll();
    //debug_log <<"slotGetData"<<status_param_number_<< reply_.size();
    if(true ==  checkReply(reply_)){
      emit replyReady(this);
      clear();
      return;
    }
  } else {

    last_error_ = tr("Ошибка выполнения запроса")+nreply_->error();
    error_log <<last_error_;

    emit replyError(this);
    clear();
  }
}


bool DwnldRequest::checkReply(const QByteArray &reply,QString *id){
  
  Q_UNUSED(id);
  Q_UNUSED(reply);

  // debug_log<< QString::fromStdString( reply.toStdString() );

  return true;
}

