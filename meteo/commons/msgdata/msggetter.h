#ifndef METEO_COMMONS_MSGDATA_MSGGETTER_H
#define METEO_COMMONS_MSGDATA_MSGGETTER_H

#include <qobject.h>

#include <sql/nosql/document.h>
#include <sql/dbi/gridfs.h>
#include <sql/dbi/dbi.h>

#include <commons/container/ts_list.h>
#include <commons/container/ringvector.h>

#include <meteo/commons/global/appstatusthread.h>

#include "types.h"

class QTimer;

namespace meteo {

class MsgGetter : public QObject
{
  Q_OBJECT

public:
  explicit MsgGetter(QObject* parent = nullptr);

  void setSyncQueue(TSList<google::protobuf::Closure*>* queue) { syncQueue_ = queue; }
  void setStatusSender(meteo::AppStatusThread* status);
  void setRequestQueue(TSList<GetTelegramCard>* queue) { queue_ = queue; }

  static void fillMessage(const DbiEntry &doc, tlg::MessageNew* mes);

signals:
  void syncQueueChanged();

public slots:
  void slotInit();
  void slotRun();

private:
  // параметры
  ConnectProp params_;

  // данные
  TSList<GetTelegramCard>* queue_ = nullptr;
  RingVector<TimeCountMetric> metric_;

  // служебные
  bool run_ = false;

  QTimer* timer_  = nullptr;
  meteo::AppStatusThread* status_ = nullptr;
  TSList<google::protobuf::Closure*>* syncQueue_ = nullptr;
};



enum JobState
{
  kWaitCard,
  kRunQuery,
  kSendResponse,
};

struct TelegramJob
{
  int id;
  JobState state = kWaitCard;
  QString query;
  GetTelegramCard card;
  bool remove = false;
  google::protobuf::Closure* done = nullptr;
  rpc::Controller* callback = nullptr;
  Dbi* db = nullptr;
};

//! MsgGetterMulti обрабатывает запросы на выдачу телеграмм с многократным ответом.
class MsgGetterMulti : public QObject
{
  Q_OBJECT

public:
  explicit MsgGetterMulti(QObject* parent = nullptr);
  ~MsgGetterMulti();

  void setSyncQueue(TSList<google::protobuf::Closure*>* queue) { syncQueue_ = queue; }

signals:
  void syncQueueChanged();

public slots:
  void slotInit();
  void slotRun();

//  void slotClientSubscribed(meteo::rpc::CallbackStore* call);
//  void slotClientUnsubscribed(meteo::rpc::CallbackStore* call);
  void slotClientSubscribed(meteo::rpc::Controller* ctrl);
  void slotClientUnsubscribed(meteo::rpc::Controller* ctrl);

  void slotNewGetTelegramMulti(const meteo::GetTelegramCard& card);

protected:
  virtual void timerEvent(QTimerEvent *event);

private:
  // параметры
  ConnectProp params_;

  // данные
  QHash<int,TelegramJob> jobs_;
  TSList<google::protobuf::Closure*>* syncQueue_ = nullptr;

  // служебные
  bool run_             = false;
  bool wasdisconnected_ = false;

  int cardId_ = 0;
  int timerId_ = -1;
};


} // meteo

#endif // METEO_COMMONS_MSGDATA_MSGGETTER_H
