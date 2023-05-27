#include "esimostream.h"
#include "esimorequest.h"
#include "dwnldrequest.h"

#include <cmath>
#include <climits>

#include <qelapsedtimer.h>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>

#include <QDomDocument>
#include <QXmlStreamReader>


#include <cross-commons/debug/tlog.h>

#include <cross-commons/debug/tmap.h>
#include <commons/textproto/pbtools.h>
#include <commons/textproto/tprototext.h>

#include <meteo/commons/msgparser/common.h>
#include <meteo/commons/msgparser/msgmetainfo.h>
#include <meteo/commons/msgparser/tlgparser.h>
#include <cross-commons/app/paths.h>

namespace meteo {

const QString urlTag       = "url";
const QString idTag        = "id";
const QString requestTag   = "request";
const QString queryTag     = "query";
const QString formatTag    = "format";
const QString hostTag      = "host";
const QString protocolTag  = "protocol";
const QString timerTag     = "interval";
const QString intervalTag  = "time";
const QString laststartTag = "laststart";

ESIMOStream::ESIMOStream(QObject* p)
  : HttpStream(p)
{
  msg_type_ = "esimoalphanum";
  status_            = nullptr;
  requests_filename_ = QString(MnCommon::projectPath() + "/etc/meteo/esimo_requests.conf");
  requests_          = new QMap< QString, QMap<QString, QString> >;
  
  // requests_query_    = new QStringList();
  // requests_format_   = new QStringList();
  getRequestsList(&requests_list_, requests_ );
}

ESIMOStream::~ESIMOStream()
{
  delete status_;
  status_ = nullptr;
  delete timer_;
  timer_ = nullptr;
//  debug_log << "~ESIMOStream";
}

void ESIMOStream::saveFile(const QByteArray & ba, const QString & req_id){

  QString cpath =filePath_+"/"+req_id+"/";
  QDir dir;
  if(false == dir.mkpath(cpath)){
    error_log <<tr("Ошибка создания каталога сохранения файла NETCDF!");
    return;
  }
  QString fname =cpath + QDateTime::currentDateTimeUtc().toString("ddMMyyyyhhmmsszzz")+".nc";

  QFile fl(fname);
  if(false == fl.open(QIODevice::WriteOnly)){
    error_log <<tr("Ошибка сохранения файла NETCDF!");
    return;
  }
  fl.write(ba);
  fl.close();
}


void ESIMOStream::slotParseReply(HttpRequest *req)
{
  status_->setParam(kOperation, tr("Обработка ответа"));
  status_->setParam(kReplyCount, ++replyCountToday_, ++replyCount_);
  status_->setParam(kRecvQueue, --count_request_in_queue_);

  const QByteArray &reply = req->getReply();
  replySize_ += reply.size();
  replySizeToday_ += reply.size();
  status_->setParam(kReplySize, AppStatusThread::sizeStr(replySizeToday_)+"/"+ AppStatusThread::sizeStr(replySize_));
  if(reply.left(3)=="CDF"||reply.left(3)=="cdf"){
    saveFile(reply,req->requestId());
    if(false == req->isCronable()){
      req->deleteLater();
    } else{
      req->restart();
    }
    status_->setParam(kOperation, tr("Ожидание"));
    return;
  }

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

/**
 * @brief Получаем список запросов из файла настроек
 * 
 * @param rlist 
 * @return int 
 */
int ESIMOStream::getRequestsList(QStringList *rlist, QMap<QString,  QMap<QString, QString> > *rrequest)
{  
  QFile fl(requests_filename_);
  QByteArray ba;
  if(false == fl.open(QIODevice::ReadOnly)){
    debug_log << "file open error "<< requests_filename_;
    return 0;
  }
  ba = fl.readAll();
  QJsonDocument document = QJsonDocument::fromJson(ba);
  if(true == document.isNull() || true == document.isEmpty() ){
    error_log << QObject::tr("Некорректное содержимое файла с запросами");
  }
  QJsonArray ja = document["queries"].toArray();
  for(int i = 0 ,max_i = ja.count(); i < max_i; i++){
    const QJsonValue& cjv = ja.at(i);
    if(false == cjv.isObject()){
      debug_log << "ошибка в файлe с запросами";
      continue;
    }
    const QJsonObject& jo = cjv.toObject();
    // debug_log<<document[urlTag].toString();
    QMap<QString, QString> m;
    // URL запроса
    m.insert( urlTag, QString("%1://%2").arg( document[protocolTag].toString() ).arg( document[hostTag].toString() ) + jo.value(urlTag).toString() );
    // тип запроса POST/GET
    m.insert( requestTag, jo.value(requestTag).toString() );
    // тело POST запроса, если есть
    m.insert( queryTag, jo.value(queryTag).toString() );
    // формат файлов, которые необходимо скачать
    m.insert( formatTag, jo.value(formatTag).toString() );
    // интервал, через который запрос будет выполняться
    m.insert( timerTag, jo.value(timerTag).toString() );
    // крон таймер для выполнения запроса
    m.insert( intervalTag, jo.value(intervalTag).toString() );
    // добавляем этот список к общему списку по айди
    (*rrequest)[ jo.value("id").toString() ] = m;

    *rlist    << jo.value("id").toString();
    // *rrequest << jo.value(requestTag).toString();
    // *rquery   << jo.value(queryTag).toString();
    // *rformat  << jo.value(formatTag).toString();
  }
  // debug_log<<"rlist";
  return rlist->size();
}


void ESIMOStream::slotProcess( )
{
  if(nullptr != timer_){
    timer_->stop();
  }
  for (int i = 0; i < requests_list_.size(); ++i){

    // проверяем, наступило ли время для выполнения процесса
    QDateTime t = QDateTime::currentDateTimeUtc();
    QString   ts;
    ts = getTagFromMap( requests_list_.at(i), laststartTag );
    // debug_log<<"~~ "<< laststartTag << "~~" << ts ;
    
    if (  ts.isEmpty() || 
          QDateTime::fromString(ts,Qt::ISODate).secsTo(t)>=getTagFromMap( requests_list_.at(i), timerTag ).toInt() ){
    
      // если это вдруг POST запрос
      if ( getTagFromMap( requests_list_.at(i), requestTag )=="post" ){
        ESIMORequest *req = new ESIMORequest( 
          getTagFromMap( requests_list_.at(i), urlTag ) , 
          requests_list_.at(i),
          getTagFromMap( requests_list_.at(i), queryTag   ) , 
          getTagFromMap( requests_list_.at(i), formatTag  ) ,
          getTagFromMap( requests_list_.at(i), intervalTag  ) );
        // когда список для загрузки готов- стартуем загрузку
        connect(req, &ESIMORequest::multiDownloadStartReady, this, &ESIMOStream::multiDownloadStart);
        createProcess( req );
      }else{
        createProcess( new ESIMORequest( getTagFromMap( requests_list_.at(i), urlTag ), getTagFromMap( requests_list_.at(i), intervalTag  )  ) );
      }
      // сохраняем время последнего запуска
      setTagFromMap( requests_list_.at(i), laststartTag, t.toString(Qt::ISODate) );
    }

  }

}

/**
 * @brief Получаем из настроек конкретный параметр
 * 
 * @param id 
 * @param tag 
 * @return QString 
 */
QString ESIMOStream::getTagFromMap(QString id,QString tag){
  // debug_log<<"++ GET TAG "<<id<<tag<<(*requests_)[id][tag];
  return (*requests_)[id][tag];
}

/**
 * @brief Записываем / изменяем параметр
 * 
 * @param id 
 * @param tag 
 * @param val 
 */
void ESIMOStream::setTagFromMap(QString id,QString tag, QString val){
  (*requests_)[id][tag] = val;
  // debug_log<<"---- SET TAG "<<id<<tag<<(*requests_)[id][tag]<<val;
  return;
}



/**
 * @brief Добавляем файлы в очередь загрузки
 * 
 * @param reqlist 
 */
void ESIMOStream::multiDownloadStart(QStringList *reqlist,QString req_id){
  for (int i = 0; i < reqlist->size(); ++i){
    createProcess( new DwnldRequest(reqlist->at(i),req_id) );
  }
  return;
}

/**
 * @brief Get the Request Xml object
 * 
 * @param resourceid 
 * @param dataparam 
 * @return QString 
 */
QString ESIMOStream::getRequestXml( 
    QString resourceid, 
    QString dataparam){
        
    QString body;
    
    body = QString("<data:%1><resourceID>%2</resourceID></data:%1>")
            .arg(dataparam)
            .arg(resourceid);
    
    QString result = QString("<soapenv:Envelope xmlns:soapenv=\"http://schemas.xmlsoap.org/soap/envelope/\" xmlns:data=\"http://data.ws.rti.nodc.meteo.ru/\">\
    <soapenv:Header/>\
    <soapenv:Body>\
        %1\
    </soapenv:Body>\
    </soapenv:Envelope>")
    .arg(body); 
    return result;
}

/**
 * @brief 
 * 
 * @param wsparam 
 * @return QString 
 */
QString ESIMOStream::getWSXml(QString wsparam){
    QString body;
    body = QString("<ws:%1/>").arg(wsparam);
    QString result = QString("<soapenv:Envelope xmlns:soapenv=\"http://schemas.xmlsoap.org/soap/envelope/\" xmlns:ws=\"http://ws.rti.nodc.meteo.ru/\">\
    <soapenv:Header/>\
    <soapenv:Body>\
        %1\
    </soapenv:Body>\
    </soapenv:Envelope>")
    .arg(body); 
    return result;
}

QMultiMap<QString, QString> ESIMOStream::parseXml(QString xml, QStringList tokens){
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

/**
 * @brief Отправляем запрос к сервису
 * 
 * @param url 
 * @param data 
 */
void ESIMOStream::sendRequestLocal( 
        QString urlstring, 
        QString req, 
        QStringList tokens ){
    QNetworkAccessManager *mgr = new QNetworkAccessManager();
    const QUrl url((urlstring));
    QNetworkRequest request(url);

    request.setRawHeader("Content-Type", "application/x-www-form-urlencoded");

    QByteArray data(req.toUtf8());
    QNetworkReply *reply = mgr->post(request,data);
    QObject::connect(reply, &QIODevice::readyRead, 
        [=](){
            // debug_log << "ready read";
        });
    QObject::connect(reply, &QNetworkReply::sslErrors,
        [=](){
            debug_log << "ssl error";
        });
    // debug_log<<reply->readAll();
    QObject::connect(reply, &QNetworkReply::finished, [=](){
        if(reply->error() == QNetworkReply::NoError){
            QString contents = QString::fromUtf8(reply->readAll());
            
            QMultiMap<QString, QString> result;
            result = parseXml( contents, tokens );

            QMapIterator<QString, QString> i(result);
            while (i.hasNext()) {
                i.next();
                // debug_log << i.key() << ": " << i.value() ;
            }

            // debug_log << contents;
            
        }
        else{
            QString err = reply->errorString();
            debug_log << err;
        }
        reply->deleteLater();

        // reply->close();
    }); 
}

/**
 * @brief Отправка GET запроса для получения JSON
 * 
 * @param urlstring 
 */
void ESIMOStream::getRequestJSON(QString urlstring){
  QNetworkAccessManager *mgr = new QNetworkAccessManager();
    const QUrl url((urlstring));
    QNetworkRequest request(url);

    request.setRawHeader("Content-Type", "application/json");

    QNetworkReply *reply = mgr->get(request);
    QObject::connect(reply, &QIODevice::readyRead, 
        [=](){
            // debug_log << "ready read";
        });
//    QObject::connect(reply, &QNetworkReply::errorOccurred,
//    QObject::connect(reply, &QNetworkReply::error,
//        [=](){
//            debug_log << "error";
//        });
    QObject::connect(reply, &QNetworkReply::sslErrors,
        [=](){
            debug_log << "ssl error";
        });
    // debug_log<<reply->readAll();
    QObject::connect(reply, &QNetworkReply::finished, [=](){
        if(reply->error() == QNetworkReply::NoError){
            
            QJsonDocument document = QJsonDocument::fromJson(reply->readAll());
            if(true == document.isNull() || true == document.isEmpty() || false == document.isArray()){
              error_log << QObject::tr("Некорректное содержимое файла с запросами");
            }
            QJsonArray ja = document.array();
            for(int i = 0 ,max_i = ja.count(); i < max_i; i++){
              const QJsonValue& cjv = ja.at(i);
              if(false == cjv.isObject()){
                debug_log << "ошибка в файлe с запросами";
                continue;
              }
              const QJsonObject& jo = cjv.toObject();
              // *rlist << jo.value(urlTag).toString()+jo.value(requestTag).toString();
              debug_log << cjv.toString();//jo.value(urlTag).toString()+jo.value(requestTag).toString();
            }
        }
        else{
            QString err = reply->errorString();
            debug_log << err;
        }
        reply->deleteLater();
    });
}

/**
 * @brief Get the Resources List of XML services
 * 
 */
void ESIMOStream::getResourcesList(){
    //objectSystemId
    QStringList tokens;
    tokens.append("objectSystemId");
    QString req = getWSXml("getE2ESearchMDList");
    sendRequestLocal("http://localhost:8080/imitatorWSITV/MetadataWS", req, tokens);
}

/**
 * @brief Get the Data By Resource object
 * 
 * @param resourceid 
 */
void ESIMOStream::getDataByResource(QString resourceid){
    QStringList tokens;
    tokens.append("return");
    QString req = getRequestXml(resourceid,"getDataFromCacheByResourceId");
    sendRequestLocal("http://localhost:8080/imitatorWSITV/DataCacheWSService", req, tokens);
}


void ESIMOStream::searchData(QString resourceid, QDateTime dt_beg, QDateTime dt_end ){
    QString url = QString("http://localhost:8080/node-rest/services/search/data/%1?periodBegin=%2&periodEnd=%3")
                  .arg( resourceid )
                  .arg( dt_beg.toString("yyyy-MM-ddThh:mm:ss") )
                  .arg( dt_end.toString("yyyy-MM-ddThh:mm:ss") );
    getRequestJSON(url);
}



} // meteo
