#include "esimorequest.h"

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


ESIMORequest::ESIMORequest(const QString &request, const QString &request_id, QObject *p)
  :HttpRequest(request, request_id,p)
{
  QString req = request;
  // yyyy-MM-ddThh:00:00
  
  if (date_test_){
    setRequest( req.replace( "@date_beg@", QDateTime::currentDateTime().toString("yyyy-06-23Thh:00:00") )
                     .replace( "@date_end@", QDateTime::currentDateTime().toString("yyyy-06-23Thh:59:59") )  );  
  }else{
    setRequest( req.replace( "@date_beg@", QDateTime::currentDateTime().toString("yyyy-MM-ddThh:00:00") )
                     .replace( "@date_end@", QDateTime::currentDateTime().toString("yyyy-MM-ddThh:59:59") )  );
  }
  
}

ESIMORequest::ESIMORequest(const QString &request,const QString &request_id, const QString &interval, QObject *p)
  :HttpRequest(request, request_id,interval, p)
{
  QString req = request;
  // yyyy-MM-ddThh:00:00
  if (date_test_){
    setRequest( req.replace( "@date_beg@", QDateTime::currentDateTime().toString("yyyy-06-23Thh:00:00") )
                     .replace( "@date_end@", QDateTime::currentDateTime().toString("yyyy-06-23Thh:59:59") )  );  
  }else{
    setRequest( req.replace( "@date_beg@", QDateTime::currentDateTime().toString("yyyy-MM-ddThh:00:00") )
                     .replace( "@date_end@", QDateTime::currentDateTime().toString("yyyy-MM-ddThh:59:59") )  );
  }
}

ESIMORequest::ESIMORequest(const QString &request,const QString &request_id,const QString &query,  const QString &format, const QString &interval, QObject *p)
  :HttpRequest(request,request_id, interval, p)
{
  request_type_   = "post";
  request_format_ = format;
  request_query_  = query.toUtf8();
  download_list_ = new QStringList();

  // yyyy-MM-ddThh:00:00
  // setRequest( req.replace( "@date_beg@", QDateTime::currentDateTime().toString("yyyy-06-23Thh:00:00") )
  //                    .replace( "@date_end@", QDateTime::currentDateTime().toString("yyyy-06-23Thh:59:59") )  );
}

ESIMORequest::~ESIMORequest(){
  if(nullptr != download_list_){
    delete download_list_;
    download_list_ = nullptr;
  }
 // debug_log <<"~ESIMORequest()";
}

/**
 * @brief Пререопределяем класс запросов, потому что нам нужен предварительный запрос
 * 
 * @param am 
 * @return true 
 * @return false 
 */
bool ESIMORequest::go(QNetworkAccessManager *am){
  //debug_log << "1112 request: "<< request_;
  if(nullptr != nreply2_){
    nreply2_->deleteLater();
  }
  am_ = am;
  qDebug()<<"CRON"<<cron_.isValid();
  if(cron_.isValid()){
    // debug_log << "ESIMORequest::go" << cron_.msecsToNext();
    QTimer::singleShot(cron_.msecsToNext(),this,SLOT(slotRunRequest()));
  } else {
    slotRunRequest();
  }
  return true;
}

/**
 * @brief Обрабатываем ошибки
 * 
 */
void ESIMORequest::slotError(){
  //emit replyError(this);
  if(nullptr ==  nreply_) {
    return;
  }
  last_error_ = tr("Ошибка выполнения запроса")+ nreply_->errorString();
  error_log <<last_error_;
  clear();
}

void ESIMORequest::slotPrePOSTGetData(){
 // debug_log << "ESIMORequest::slotPrePOSTGetData";
  QStringList tokens;
  tokens.append("return");
  if(nullptr ==  nreply2_) {
    return;
  }
  if(QNetworkReply::NoError == nreply2_->error()){
    // reply_ = nreply2_->readAll();
    QString contents = QString::fromUtf8(nreply2_->readAll());
    // debug_log<<nreply2_->readAll();
    QMultiMap<QString, QString> result;
    result = parseXml( contents, tokens );

    QMapIterator<QString, QString> i(result);
    download_list_->clear();
    while (i.hasNext()) {
        i.next();
        if ( i.value().endsWith( request_format_,Qt::CaseInsensitive ) ){
          *download_list_<<i.value();
        }
    }

    // пробуем скачать все эти файлы
    emit multiDownloadStartReady(download_list_,request_id_);
    emit replyReady(this);
    clear();
    return;

  }
  error_log <<QObject::tr("Ошибка выполнения запроса");
  last_error_ = tr("Ошибка выполнения запроса");

  emit replyError(this);
  clear();
}


void ESIMORequest::slotRunRequest()
{
  if(nullptr == am_){
    return;
  }
  // debug_log << "request: "<< request_;
  if(nullptr != nreply_){
    nreply_->deleteLater();
  }
  
  if ( request_type_=="get" ){
    nreply2_ = am_->get(QNetworkRequest(QUrl(request_)));
    connect(nreply2_, &QNetworkReply::finished, this, &ESIMORequest::slotPreGetData);
  }else{
    const QUrl url((request_));
    QNetworkRequest request(url);
    request.setRawHeader("Content-Type", "application/x-www-form-urlencoded");
    nreply2_ = am_->post(request, request_query_ );
    connect(nreply2_, &QNetworkReply::finished, this, &ESIMORequest::slotPrePOSTGetData);
  }
  
//  connect(nreply2_, &QNetworkReply::errorOccurred, this, &ESIMORequest::slotError);
  // connect(nreply2_, &QNetworkReply::downloadProgress, this, &HttpRequest::slotDownloadProgress);
  emit requestSend();
  
}

void ESIMORequest::slotPreGetData(){
  
//  debug_log << "ESIMORequest::slotPreGetData";
  if(nullptr ==  nreply2_) {
    return;
  }
  if( QNetworkReply::NoError == nreply2_->error()){
    reply_ = nreply2_->readAll();

    // результат предзапроса для получения ссылки для скачивания данных
    QString download_str = QString::fromStdString(reply_.toStdString());
    if ( download_str.indexOf("download:")==-1 ){
      return ;
    }
    // пытаемся найти ссылку на скачивание данных
    QRegExp dwnld("download:\\s+(http:.*)\\s+");
    int pos = dwnld.indexIn(download_str);

    QString newUrl;
    if (pos > -1) {
      newUrl = dwnld.cap(1);
    }

    QNetworkAccessManager *mgr = new QNetworkAccessManager(this);
    const QUrl url((newUrl));
    QNetworkRequest request(url);
    request.setRawHeader("Content-Type", "application/x-www-form-urlencoded");
    nreply_ = mgr->get(request);
    
    if(nullptr != nreply_){
      connect(nreply_, &QNetworkReply::finished, this, &ESIMORequest::slotGetData);
    }
    return;

  } else {
    debug_log << nreply2_->error();
  }
  error_log <<QObject::tr("Ошибка выполнения запроса");
  last_error_ = tr("Ошибка выполнения запроса");

  emit replyError(this);
  clear();
}



void ESIMORequest::slotGetData(){
  if(nullptr ==  nreply2_) {
    return;
  }
  if(QNetworkReply::NoError == nreply_->error()){

    reply_ = nreply_->readAll();

    if(true ==  checkReply(reply_)){
      emit replyReady(this);
      clear();
      return;
    }
  } else {
    debug_log << nreply_->error();
  }
  error_log <<QObject::tr("Ошибка выполнения запроса");
  last_error_ = tr("Ошибка выполнения запроса");

  emit replyError(this);
  clear();

}


bool ESIMORequest::checkReply(const QByteArray &reply,QString *id){
  
  Q_UNUSED(id);
  Q_UNUSED(reply);

  // debug_log<< QString::fromStdString( reply.toStdString() );

  return true;
}



QMultiMap<QString, QString> ESIMORequest::parseXml(QString xml, QStringList tokens){
    QMultiMap<QString, QString> result;

    QXmlStreamReader xmlReader(xml);
    //Parse the XML until we reach end of it
    while(!xmlReader.atEnd() && !xmlReader.hasError()) {
        // Read next element
        QXmlStreamReader::TokenType token = xmlReader.readNext();
        //If token is just StartDocument - go to next
        if(token == QXmlStreamReader::StartDocument) {
            continue;
        }
        //If token is StartElement - read it
        if(token == QXmlStreamReader::StartElement) {
            if( tokens.contains( xmlReader.name().toString() ) ) {
                QString txt = xmlReader.readElementText();
                result.insert( xmlReader.name().toString(), txt );
                // debug_log << xmlReader.name().toString() << txt;
            }
        }
    }

    if(xmlReader.hasError()) {
        debug_log<<"xml File Parse Error"<<xmlReader.errorString();
        return result;
    }

    //close reader and flush file
    xmlReader.clear();
    return result;
}
