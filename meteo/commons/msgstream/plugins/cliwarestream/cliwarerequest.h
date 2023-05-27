#ifndef METEO_COMMONS_MSGSTREAM_PLUGINS_CLIWAREREQUEST_H
#define METEO_COMMONS_MSGSTREAM_PLUGINS_CLIWAREREQUEST_H

#include <meteo/commons/msgstream/plugins/httpstream/httprequest.h>


class CliWareRequest: public HttpRequest
{
  Q_OBJECT
public:
  CliWareRequest(const QString &request, const QString &request_id, QObject* p = 0);
  CliWareRequest(const QString &request, const QString &request_id,const QString &interval, QObject* p = 0);
  ~CliWareRequest();
protected slots:
  virtual void slotGetData();

private:
  bool checkReply(const QByteArray &reply, QString *id = nullptr);

/*
  void testReplyJson(const QByteArray & reply);
*/
  QByteArray getTestReply();

};

#endif // CLIWAREREQUEST_H
