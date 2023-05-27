#include "cliwarerequest.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>

#include <sql/rapidjson/document.h>
#include <sql/rapidjson/reader.h>
#include <sql/rapidjson/error/en.h>

#include <cross-commons/debug/tlog.h>

const std::string jsonType = "MeteoJson";
const std::string replyIdTag = "Id";
const std::string dataTag = "aaData";
const std::string describeDataTag = "Def_aaData";
const std::string describeDataTagTag = "Temp";


CliWareRequest::CliWareRequest(const QString &request,const QString &request_id, QObject *p)
  :HttpRequest(request,request_id, p)
{

}

CliWareRequest::CliWareRequest(const QString &request, const QString &request_id,const QString &cron_interval, QObject *p)
  :HttpRequest(request,request_id, cron_interval, p)
{

}

CliWareRequest::~CliWareRequest(){
}

void CliWareRequest::slotGetData(){
  /*  reply_ = getTestReply();
   emit replyReady(this);
   reply->deleteLater();
   return;*/
  if(nullptr ==  nreply_) {
    return;
  }
  debug_log <<"request "<< nreply_->request().url().toString();

  if(QNetworkReply::NoError == nreply_->error()){
    reply_ = nreply_->readAll();
    if(true ==  checkReply(reply_)){
      emit replyReady(this);
      clear();
      return;
    }
  } else {

    last_error_ = tr("Ошибка выполнения запроса");
    error_log <<last_error_;

    emit replyError(this);
    clear();
  }
}

bool CliWareRequest::checkReply(const QByteArray &reply,QString *id){
  auto return_func = [](QString err)
  {
    error_log << err;
    return false;
  };
  rapidjson::Document document;
  //создаём объект Json Document, считав в него все данные из ответа
  document.Parse(reply);
  // QJsonDocument document = QJsonDocument::fromJson(reply);
  if(document.HasParseError()){
    auto error = "Failed to parse JSON (offset " + std::to_string(document.GetErrorOffset()) + "). "
              "Error: " + rapidjson::GetParseError_En(document.GetParseError());
    return return_func(QString::fromStdString(error));
  }

  // Забираем из документа корневой объект
  const rapidjson::Value& jv = document["Type"];
  if(jv.GetString() != jsonType){
    //if(jv.toString() != jsonType){
    return return_func(QObject::tr("Ответ не соответствует типу MeteoJson"));
  }
  if (false == document.HasMember(describeDataTag.c_str()))
  {
    return return_func(QObject::tr("Ответ не содержит описание данных %1").arg(describeDataTag.c_str()));
  }
  if ( false == document.HasMember(replyIdTag.c_str()))
  {
    return return_func(QObject::tr("Ответ не содержит описание данных %1").arg(replyIdTag.c_str()));
  }
  if(true != document[describeDataTag.c_str()].IsObject()){
    return return_func(QObject::tr("Ответ содержит некорректное описание данных %1").arg(describeDataTag.c_str()));
  }
  const auto &obj =  document[describeDataTag.c_str()].GetObject();
  if ( false == obj.HasMember(describeDataTagTag.c_str()) )
  {
    return return_func(QObject::tr("Ответ не содержит описание данных %1").arg(describeDataTagTag.c_str()));
  }

  if(nullptr != id){
    //*id = root.value(replyIdTag).toString();
    *id = QString::fromStdString(document[replyIdTag.c_str()].GetString());
  }

  return true;
}
/*
bool CliWareRequest::checkReply(const QByteArray &reply,QString *id){
  auto return_func = [](QString err)
  {
    error_log << err;
    return false;
  };
  //создаём объект Json Document, считав в него все данные из ответа
  QJsonDocument document = QJsonDocument::fromJson(reply);
  if(true == document.isNull() || true == document.isEmpty()){
    return return_func(QObject::tr("Запрос вернул пустой ответ"));
  }
  // Забираем из документа корневой объект
  QJsonObject root = document.object();
  QJsonValue jv = root.value("Type");
  if(jv.toString() != jsonType){
    return return_func(QObject::tr("Ответ не соответствует типу MeteoJson"));
  }

  if(nullptr != id){
    *id = root.value(replyIdTag).toString();
  }

  if(false == root.contains(describeDataTag)){
    return return_func(QObject::tr("Ответ не содержит описание данных"));
  }
  QJsonObject jo;
  jo = root.value(describeDataTag).toObject(); //Берем описание полученных данных
  if(false == jo.contains(describeDataTagTag)){
    return return_func(QObject::tr("Ответ не содержит описание данных [Temp]"));
  }
  jv = jo.value(describeDataTagTag);
  if(true == jv.isNull() || false == jv.isArray()){
    return return_func(QObject::tr("Ответ не содержит описание данных [Temp]"));
  }
  return true;
}

//for testing

void CliWareRequest::testReplyJson(const QByteArray & reply)
{
  //создаём объект Json Document, считав в него все данные из ответа
  QJsonDocument document = QJsonDocument::fromJson(reply);
  // Забираем из документа корневой объект
  QJsonObject root = document.object();
  QJsonObject jo;
  jo = root.value(describeDataTag).toObject(); //Берем описание полученных данных
  QJsonValue jv = jo.value(describeDataTagTag);
  QJsonArray ja = jv.toArray();

  QStringList data_describe;
  for(int i = 0 ,max_i = ja.count(); i < max_i; i++){
    const QJsonValue& cjv = ja.at(i);
    if(false == cjv.isString()){
      debug_log << "ошибка в описании данных [Temp]";
      continue;
    }
    data_describe << cjv.toString();
  }

  jv = root.value( dataTag );

  if(true == jv.isNull() || false == jv.isArray()){
    error_log << QObject::tr("Ответ не содержит корректные данных");
    return;
  }
  // Если значение является массивом, ...
  // ... то забираем массив из данного свойства
  ja = jv.toArray();
  // Перебирая все элементы массива ...
  for(int i = 0; i < ja.count(); i++){
    if(false == ja.at(i).isArray()){
      debug_log << "Неизвестный формат данных";
      continue;
    }
    QJsonArray data = ja.at(i).toArray();
    for(int j = 0; j < data.count(); j++){
      if(false == data.at(j).isString()){
        debug_log << "Неизвестный формат данных";
        continue;
      }
      debug_log << data.at(j).toString();
    }
  }
}
*/

QByteArray CliWareRequest::getTestReply()
{
  QByteArray ba;
  QString filename = "/home/gotur/tmp/ESIMO/MCD/reply1.txt";
  QFile fl(filename);
  if(false == fl.open(QIODevice::ReadOnly)){
    debug_log << "Open file error"<< filename;
    return ba;
  }
  ba = fl.readAll();
  return ba;
}


