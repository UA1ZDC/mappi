#include "cliwarestream.h"
#include "cliwarerequest.h"

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

const QString urlTag = "url";
const QString requestTag = "request";
const QString intervalTag = "time";
const QString type_dateTag = "type_date";

CliWareStream::CliWareStream(QObject* p)
  : HttpStream(p)
{
  msg_type_ = "clialphanum";
  status_ = nullptr;
  requests_filename_ = QString(MnCommon::projectPath() + "/etc/meteo/cliware_requests.conf");
  initRequestsList();
}

CliWareStream::~CliWareStream()
{
}

void CliWareStream::initRequestsList()
{
  QFile fl(requests_filename_);
  QByteArray ba;
  if(false == fl.open(QIODevice::ReadOnly)){
    debug_log << "file open error "<< requests_filename_;
    return;
  }
//  QDateTime dt = QDateTime::currentDateTimeUtc();
  ba = fl.readAll();
  QJsonDocument document = QJsonDocument::fromJson(ba);
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
    QString request = jo.value(requestTag).toString();
    QString type_date = jo.value(type_dateTag).toString();
    createRequest( &request, type_date );

    QPair<QString,QString> sp;
    sp.first = jo.value(urlTag).toString() + request;
    sp.second = jo.value(intervalTag).toString(); //интервал выполнения запроса в формате cron
    requests_interval_list_<<sp;
  }
}

void CliWareStream::createRequest( QString *request, const QString &type_date)
{
  if("LAST_3HOUR" == type_date){
    createLastHourRequest(request,3);
  }
  if("LAST_1HOUR" == type_date){
    createLastHourRequest(request,1);
  }
  if("LAST_1MONTHS" == type_date){
      createLastMonthsRequest(request,1);
  }


}

void CliWareStream::createLastHourRequest( QString *request, int hour)
{
  QDateTime cdt = QDateTime::currentDateTimeUtc();
  QDateTime dt = QDateTime::currentDateTimeUtc().addSecs(-hour*3600);
  request->replace("@year_beg@", QString::number(dt.date().year()));
  request->replace("@year_end@", QString::number(cdt.date().year()));
  request->replace("@MM-dd_hh:mm_beg@", dt.toString("MM-dd hh:00"));
  request->replace("@MM-dd_hh:mm_end@", cdt.toString("MM-dd hh:00"));
}

void CliWareStream::createLastMonthsRequest( QString *request, int months)
{
  QDateTime cdt = QDateTime::currentDateTimeUtc();
  QDateTime dt = QDateTime::currentDateTimeUtc().addMonths(-months);
  request->replace("@year_beg@", QString::number(dt.date().year()));
  request->replace("@year_end@", QString::number(cdt.date().year()));
  request->replace("@MM-dd_hh:mm_beg@", dt.toString("MM-dd hh:ss"));
  request->replace("@MM-dd_hh:mm_end@", cdt.toString("MM-dd hh:ss"));
}

void CliWareStream::slotProcess( )
{
  if(nullptr != timer_){
    timer_->stop();
  }
  for(const QPair<QString,QString> &srequest: qAsConst(requests_interval_list_)){
    createProcess(new CliWareRequest(srequest.first,srequest.second));
  }
}


} // meteo
