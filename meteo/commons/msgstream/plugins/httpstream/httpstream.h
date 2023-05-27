#ifndef METEO_COMMONS_MSGSTREAM_PLUGINS_HTTPSTREAM_H
#define METEO_COMMONS_MSGSTREAM_PLUGINS_HTTPSTREAM_H

#include <qdir.h>
#include <qbytearray.h>
#include <qstringlist.h>

#include <commons/container/ts_list.h>
#include <meteo/commons/msgstream/streammodule.h>
#include <meteo/commons/proto/msgstream.pb.h>

#include "httprequest.h"

class QTextCodec;
class QFileSystemWatcher;
class QNetworkReply;
class QNetworkAccessManager;

namespace meteo {

class TlgParser;

class HttpStream : public StreamModule
{
  Q_OBJECT


protected:
  enum StatusParam {
    kRequestsCount    = 1024,
    kReplyCount    ,
    kErrorReplyCount,
    kReplySize,
    kOperation,
    kRecvQueue,
    kRecvStatus
  };

public:
  HttpStream(QObject* p = 0);
  virtual ~HttpStream();

  virtual void setOptions(const msgstream::Options& opt);

public slots:
  virtual bool slotInit();

  void slotTimeout();
  virtual void slotProcess()=0;
  virtual void slotParseReply(HttpRequest *);
  virtual void slotErrorReply(HttpRequest *);
  void slotDownloadProcess(int status_param_number, qint64 bytesSent, qint64 bytesTotal);
  void slotNewRequestCounter();

protected:
  void timerEvent(QTimerEvent* event);
  void createProcess(HttpRequest * );

  int getNextStatusParamId();
  //virtual bool checkReply(const QByteArray &reply,QString *id=nullptr) = 0;
  //virtual void sendRequest(const QString & request) = 0;

  // параметры
  QString id_;
  int maxQueue_;
  msgstream::Options opt_;
  qint64 requestCount_  = 0;
  qint64 requestCountToday_  = 0;
  qint64 replyCount_  = 0;
  qint64 replyCountToday_  = 0;
  qint64 errorReplyCount_  = 0;

  qint64 replySize_   = 0;
  qint64 replySizeToday_   = 0;

  QDateTime lastCheck_;
  // служебные

  QTimer* timer_  = nullptr;
  bool run_ = false;

  QVector<QPair<QString,QString>> requests_interval_list_; //first: request; second:интервал выполнения запроса в формате cron


  TlgParser* parser_ = nullptr;
  QByteArray magicBegin_="";//= "\001";;
  QByteArray magicEnd_ = "\x0D\x0D\x0A\x03";
  QNetworkAccessManager *manager_ = nullptr;
  int *status_param_id=nullptr;
  int count_request_in_queue_ = 0;
  QString msg_type_ = "clialphanum";
  QString filePath_ = "/tmp/esimodata";

};


} // meteo

#endif // METEO_COMMONS_MSGSTREAM_PLUGINS_FILESTREAM_FILESTREAMIN_H
