#ifndef METEO_COMMONS_MSGSTREAM_PLUGINS_CliWareSTREAM_H
#define METEO_COMMONS_MSGSTREAM_PLUGINS_CliWareSTREAM_H

#include <qdir.h>
#include <qbytearray.h>
#include <qstringlist.h>

#include <commons/container/ts_list.h>
#include <meteo/commons/msgstream/streammodule.h>
#include <meteo/commons/proto/msgstream.pb.h>
#include <meteo/commons/msgstream/plugins/httpstream/httpstream.h>

class QTextCodec;
class QFileSystemWatcher;
class QNetworkReply;
class QNetworkAccessManager;

namespace meteo {

class TlgParser;

class CliWareStream : public HttpStream
{
  Q_OBJECT

public:
  static QString moduleId() { return QString("cliware"); }

public:
  CliWareStream( QObject* p = 0);
  virtual ~CliWareStream();

public slots:

//  void slotTimeout();
  void slotProcess();

private:
  //bool parseReply(const QByteArray &);
 // bool checkReply(const QByteArray &reply,QString *id=nullptr);
  //void sendRequest(const QString & request);
  int  getRequestsList(QStringList *rlist);
  void initRequestsList();
  void createRequest(QString * , const QString &);
  void createLastHourRequest(QString *request, int hour);
  void createLastMonthsRequest(QString *request, int months);


  // параметры

  QString  requests_filename_;// = QString(MnCommon::projectPath() + "/etc/meteo/cliware_requests.conf");

};


} // meteo

#endif // METEO_COMMONS_MSGSTREAM_PLUGINS_FILESTREAM_FILESTREAMIN_H
