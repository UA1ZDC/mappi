#include "qjsondocument.h"
#include "qjsonobject.h"
#include "receivemessthread.h"

const QString uuid = QString("tlglog");

ReceiveMessThread::ReceiveMessThread(QObject *p)
  : QObject(p),
    settings_(0),
    timemsec_(20),
    telegrammsBiggerThen_(450000),
    thenshowmaxtelegramms_(50000)
{
  settings_ = new QSettings(QDir::homePath() + "/.meteo/" + "TlgMonitorForm" + ".ini", QSettings::IniFormat);
  slotRestoreTimeAndMaxTelegrams();
}

void ReceiveMessThread::slotInit()
{
  slotSaveTimeAndMaxTelegramms();
  if ( 0 == channel_ ) {
    channel_ = meteo::global::serviceChannel(meteo::settings::proto::kMsgCenter);
    if ( 0 == channel_ ) {
      error_log << meteo::msglog::kServiceNotFound.arg(meteo::global::serviceTitle(meteo::settings::proto::kMsgCenter));
      QTimer::singleShot( 15000, this, SLOT(slotInit()) );
      return;
    }
    QObject::connect( channel_, SIGNAL( disconnected() ), this, SLOT( slotServiceDisconnected()) );
  }
  if ( !channel_->isConnected() ) {
    if ( !channel_->connect(meteo::global::serviceAddress(meteo::settings::proto::kMsgCenter)) ) {
      delete channel_;
      channel_ = 0;
      QTimer::singleShot( 15000, this, SLOT(slotInit()) );
      return;
    }
  }
  debug_log << tr("Соединение с send.service установлено");
  meteo::msgcenter::DistributeMsgRequest req;
  req.set_uid(uuid.toStdString());
  auto rule = req.add_rule();
  rule->set_type(meteo::msgcenter::RuleType::kAllow);
  rule->mutable_tlgfilter()->set_msgtype("alphanum");
  rule->set_to(uuid.toStdString());
  rule->set_priority(0);
  rule = req.add_rule();
  rule->set_type(meteo::msgcenter::RuleType::kDeny);
  rule->set_to(uuid.toStdString());
  rule->set_priority(1);

  channel_->subscribe(&meteo::msgcenter::MsgCenterService::DistributeMsg, req, this, &ReceiveMessThread::slotSend);
  QTimer *t_ = new QTimer;
  t_->setInterval(20);
  QObject::connect(t_,SIGNAL(timeout()),this,SLOT(slotTimeOut()) );
  t_->start();
}

meteo::tlg::MessageNew ReceiveMessThread::msgFromJson(const QJsonObject &obj)
{
  Q_UNUSED(obj);
  return meteo::tlg::MessageNew();
}

void ReceiveMessThread::slotSend(meteo::tlg::MessageNew *msg)
{
  msglist_.append(*msg);
  if(msglist_.size()>telegrammsBiggerThen_){
    while(msglist_.size()>=thenshowmaxtelegramms_){
      msglist_.removeLast();
    }
  }
}

void ReceiveMessThread::slotServiceDisconnected()
{
  QTimer::singleShot( 5000, this, SLOT(slotInit()) );
}

void ReceiveMessThread::slotTimeOut()
{
  if(!msglist_.isEmpty()){
    emit sendOneMessage(msglist_.takeFirst());
  }
}

void ReceiveMessThread::slotSaveTimeAndMaxTelegramms()
{
  settings_->setValue("timemsec",timemsec_);
  settings_->setValue("telegrammsbiggerthen",telegrammsBiggerThen_);
  settings_->setValue("thenshowmaxtelegramms",thenshowmaxtelegramms_);
}

void ReceiveMessThread::slotRestoreTimeAndMaxTelegrams()
{
  if(!settings_->value("timemsec").isNull() ){
    timemsec_ = settings_->value("timemsec").toInt();
  }
  if(!settings_->value("telegrammsbiggerthen").isNull() ){
    telegrammsBiggerThen_ = settings_->value("telegrammsbiggerthen").toInt();
  }
  if(!settings_->value("thenshowmaxtelegramms").isNull() ){
    thenshowmaxtelegramms_ = settings_->value("thenshowmaxtelegramms").toInt();
  }
}

void ReceiveMessThread::slotOnNotify()
{
  not_impl;
//  QStringList filter;
//  filter << QString("\"_id\" : { \"$gt\" : %1 }").arg(lastID_);
//  nosql::Find fnd("msg", filter);
//  auto cursor = nosql_->find("telegramsdb", fnd);
//  QStringList docs;
//  if(nullptr != cursor) {
//    while (cursor->next()) {
//      QString doc = cursor->getDocument();
//      if(false == doc.isEmpty()) {
//        docs << doc;
//      }
//    }
//  }
//  for ( int i = 0, sz = docs.size(); i < sz; ++i ) {
//    QJsonDocument d = QJsonDocument::fromJson(docs.at(i).toUtf8());
//    QJsonObject obj;
//    if ( true == d.isObject() ) {
//      obj = d.object();
//    }
//    else {
//      continue;
//    }
//    meteo::tlg::MessageNew message;
//  }
}
