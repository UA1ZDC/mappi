#ifndef METEO_COMMONS_MSGSTREAM_PLUGINS_HTTPREQUEST_H
#define METEO_COMMONS_MSGSTREAM_PLUGINS_HTTPREQUEST_H

#include <qobject.h>
#include <QNetworkAccessManager>
#include <QNetworkReply>

#include "crontimer.h"

class HttpRequest: public QObject
{
  Q_OBJECT
public:
  HttpRequest(const QString &request, const QString &request_id, QObject* p = 0);
  HttpRequest(const QString &request, const QString &request_id, const QString &cron_interval, QObject* p = 0);
  virtual ~HttpRequest();

  virtual bool go(QNetworkAccessManager *);
  QByteArray getReply();
  const QString &getLastError();
  void setStatusParamId(int spid);

  void setRequest(const QString &request);
  void setCronInterval(const QString &interval);
  bool isCronable() {return cron_.isValid();};
  void restart();
  QString requestId();
public slots:
  void slotDownloadProgress(qint64 bytesSent, qint64 bytesTotal);

protected slots:
  virtual void slotGetData();
  void slotError(QNetworkReply::NetworkError );

  void slotTimeOut();
  void slotRunRequest();

signals:
  void replyReady(HttpRequest *);
  void replyError(HttpRequest *);
  void downloadProcess(int status_param_number, qint64 bytesSent, qint64 bytesTotal);
  void requestSend();
protected:
  void clear();

  CronTimer cron_; //обработчик строки cron
  QNetworkAccessManager *am_ = nullptr;
  QNetworkReply* nreply_   = nullptr;
  QNetworkReply* nreply2_  = nullptr;

  QByteArray reply_;
  QString request_;
  QString last_error_;
  qint64 bytes_sent_ = 0;
  qint64 bytes_total_ = 0;
  int status_param_number_ = 0;
  QString request_id_;
};

#endif // HTTPREQUEST_H
