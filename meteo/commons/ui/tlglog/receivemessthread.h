#ifndef METEO_COMMONS_UI_TLGLOG_RECEIVEMESSTHREAD_H
#define METEO_COMMONS_UI_TLGLOG_RECEIVEMESSTHREAD_H

#include <qstring.h>
#include <qtimer.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/rpc/channel.h>
#include <meteo/commons/proto/msgparser.pb.h>
#include <meteo/commons/msgparser/common.h>
#include <meteo/commons/global/log.h>
//#include <meteo/commons/msgcenter/msgcenterservice.h>
#include <meteo/commons/settings/settings.h>
#include <qsettings.h>
#include <qdir.h>


class ReceiveMessThread : public QObject
{
Q_OBJECT
public:
  explicit ReceiveMessThread(QObject* p = 0);

public slots:
  void slotInit();

private:
  QList < meteo::tlg::MessageNew > msglist_;
  meteo::rpc::Channel* channel_ = 0;
  meteo::tlg::MessageNew msg_;
  QSettings* settings_;
  QTimer pseudoNotify_;
  int lastID_ = 0;
  int timemsec_ /*= 20*/;
  int telegrammsBiggerThen_/* = 450000*/;
  int thenshowmaxtelegramms_/* = 50000*/;

  meteo::tlg::MessageNew msgFromJson(const QJsonObject& obj);


private slots:
  void slotSend(meteo::tlg::MessageNew* msg);
  void slotServiceDisconnected();
  void slotTimeOut();
  void slotSaveTimeAndMaxTelegramms();
  void slotRestoreTimeAndMaxTelegrams();
  void slotOnNotify();

signals:
  void sendOneMessage(meteo::tlg::MessageNew message);

};

#endif // RECEIVEMESSTHREAD_H
