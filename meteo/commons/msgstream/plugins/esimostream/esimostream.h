#ifndef METEO_COMMONS_MSGSTREAM_PLUGINS_ESIMOSTREAM_H
#define METEO_COMMONS_MSGSTREAM_PLUGINS_ESIMOSTREAM_H

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

class ESIMOStream : public HttpStream
{
  Q_OBJECT

public:
  static QString moduleId() { return QString("esimo"); }

public:
  ESIMOStream(QObject* p = 0);
  virtual ~ESIMOStream();

public slots:

  void slotProcess();
  void multiDownloadStart(QStringList *reqlist, QString req_id);
  virtual void slotParseReply(HttpRequest *);

private:
  // int  getRequestsList(QStringList *rlist);
  void saveFile(const QByteArray &, const QString & req_id);

  QString getRequestXml( QString resourceid, QString dataparam);
  QString getWSXml(QString wsparam);
  QMultiMap<QString, QString> parseXml(QString xml, QStringList tokens);
  int getRequestsList(QStringList *rlist, QMap<QString, QMap<QString, QString> > *rrequest);
  void sendRequestLocal( QString urlstring, QString req, QStringList tokens );
  void getRequestJSON(QString urlstring);
  void getResourcesList();
  void getDataByResource(QString resourceid);
  void searchData(QString resourceid, QDateTime dt_beg, QDateTime dt_end );
  QString getTagFromMap(QString id,QString tag);
  void setTagFromMap(QString id,QString tag, QString val);
  
  // параметры

  QString  requests_filename_;
  QMap<QString, QMap<QString, QString> > *requests_        = nullptr;
  // QStringList *requests_query_  = nullptr;
  // QStringList *requests_format_ = nullptr;
  QStringList requests_list_;
};


} // meteo

#endif // METEO_COMMONS_MSGSTREAM_PLUGINS_FILESTREAM_FILESTREAMIN_H
