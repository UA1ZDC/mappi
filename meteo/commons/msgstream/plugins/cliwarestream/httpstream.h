#ifndef METEO_COMMONS_MSGSTREAM_PLUGINS_CliWareSTREAM_H
#define METEO_COMMONS_MSGSTREAM_PLUGINS_CliWareSTREAM_H

#include <qdir.h>
#include <qbytearray.h>
#include <qstringlist.h>

#include <commons/container/ts_list.h>
#include <meteo/commons/msgstream/streammodule.h>
#include <meteo/commons/proto/msgstream.pb.h>

class QTextCodec;
class QFileSystemWatcher;
class QNetworkReply;
class QNetworkAccessManager;

namespace meteo {

class TlgParser;

class HttpStream : public StreamModule
{
  Q_OBJECT

  enum StatusParam {
    kRequestsCount    = 1024,
    kReplyCount    ,
    kErrorReplyCount,
    kReplySize,
    kOperation,
  };

public:
  static QString moduleId() { return QString("cliware"); }

public:
  HttpStream(QObject* p = 0);
  virtual ~HttpStream();

  virtual void setOptions(const msgstream::Options& opt);

public slots:
  virtual bool slotInit();

  void slotTimeout();
  void slotProcess();
  bool slotGetData(QNetworkReply *reply);


protected:
  virtual void timerEvent(QTimerEvent* event);

private:
  void sendRequest(const QString & request);
  int getRequestsList(QStringList *rlist);
  bool parseReplyJson(const QByteArray &);
  bool checkReplyJson(const QByteArray &reply,QString *id=nullptr);


  //for testing:
  QByteArray getTestReply();
  void testReplyJson(const QByteArray & reply);

  // параметры

  QString  requests_filename_;// = QString(MnCommon::projectPath() + "/etc/meteo/cliware_requests.conf");

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
  QStringList processedFiles_;
  QTimer* timer_  = nullptr;
  bool run_ = false;
  QNetworkAccessManager *network_manager_ = nullptr;
  QStringList requests_list_;
  TlgParser* parser_ = nullptr;
  QByteArray magicBegin_;
  QByteArray magicEnd_;

};


} // meteo

#endif // METEO_COMMONS_MSGSTREAM_PLUGINS_FILESTREAM_FILESTREAMIN_H
