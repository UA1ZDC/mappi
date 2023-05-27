#ifndef METEO_COMMONS_MSGSTREAM_PLUGINS_ESIMOQUEST_H
#define METEO_COMMONS_MSGSTREAM_PLUGINS_ESIMOQUEST_H

#include <meteo/commons/msgstream/plugins/httpstream/httprequest.h>


class ESIMORequest: public HttpRequest
{
  Q_OBJECT
public:
  ESIMORequest(const QString &request, const QString &request_id,QObject* p = 0);
  ESIMORequest(const QString &request,const QString &request_id, const QString &interval, QObject* p = 0);
  ESIMORequest(const QString &request,const QString &request_id, const QString &query,  const QString &format, const QString &interval, QObject* p = 0);
  ~ESIMORequest();
  bool go(QNetworkAccessManager *);
  QMultiMap<QString, QString> parseXml(QString xml, QStringList tokens);

signals:
  void multiDownloadStartReady(QStringList *reqlist,QString req_id);

protected slots:
  void slotGetData();
  void slotPreGetData();
  void slotPrePOSTGetData();
  void slotError();
  void slotRunRequest();

private:
  bool checkReply(const QByteArray &reply, QString *id = nullptr);
 // QNetworkAccessManager *am_;
  QStringList*   download_list_   = nullptr; // список файлов, которые надо скачать
  QString        request_type_    = "get";
  QString        request_format_  = "txt";
  QByteArray     request_query_;
  // если выставлено в true, тогда дату берем для тестовой базы, чтобы проверить соединение
  bool           date_test_       = false;
};
/**
 * title           nc
 * RU_FERHRI_160 -
 * RU_Hydrometcentre_160 -
 * RU_Hydrometcentre_167 -
 * RU_AARI_1163 -
 * RU_RC-PLANET_20 -
 * RU_RC-PLANET_25 -
 * RU_RC-PLANET_64 +
 * RU_RIHMI-WDC_117 -
 * RU_RIHMI-WDC_1019 +
 * RU_RIHMI-WDC_1164 -
 * RU_RIHMI-WDC_1172 -
 * RU_RIHMI-WDC_1195 -
 * RU_RIHMI-WDC_1196 -
 * RU_RIHMI-WDC_1198 -
 * RU_RIHMI-WDC_1254 -
 * RU_RIHMI-WDC_1300 -
 * RU_RIHMI-WDC_2772 -
 * RU_RIHMI-WDC_2892 -
 */
#endif // ESIMOQUEST_H
