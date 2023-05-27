#ifndef METEO_COMMONS_MSGSTREAM_PLUGINS_DWNLDQUEST_H
#define METEO_COMMONS_MSGSTREAM_PLUGINS_DWNLDQUEST_H

#include <meteo/commons/msgstream/plugins/httpstream/httprequest.h>


class DwnldRequest: public HttpRequest
{
  Q_OBJECT
public:
  DwnldRequest(const QString &request, const QString &request_id, QObject* p = 0);
  ~DwnldRequest();

protected slots:
  void slotGetData();
  void slotError();

private:
  bool checkReply(const QByteArray &reply, QString *id = nullptr);
};

#endif // DwnldQUEST_H
