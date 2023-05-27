#ifndef METEO_COMMONS_MSGDATA_MSGSAVER_H
#define METEO_COMMONS_MSGDATA_MSGSAVER_H

#include <qhash.h>
#include <qobject.h>

#include <meteo/commons/rpc/rpc.h>
#include <meteo/commons/proto/surface_service.pb.h>
#include <meteo/commons/msgdata/msgqueue.h>

class QTimer;

namespace meteo {

//! Усвоение через srcdata.service
class MsgSaver : public QObject
{
  Q_OBJECT

public:
  explicit MsgSaver(QObject* parent = nullptr);

  void setMsgQueue(MsgQueue* queue) { queue_ = queue; }

signals:
  void msgProcessed(int id);

public slots:
  void slotInit();
  void slotRun();
  void slotConnectToSrcData();
  void slotSrcDataDisconnected();

private:
  void msgProcessed(google::protobuf::RpcController* c, surf::SaveMsgReply* r);
  void resendMsg();

private:
  // данные
  MsgQueue* queue_ = nullptr;
  QHash<surf::SaveMsgReply*,int> sendMsg_;
  QHash<google::protobuf::RpcController*,google::protobuf::Closure*> controllers_;

  // служебные
  bool run_ = false;

  QTimer* timer_ = nullptr;
  rpc::Channel* channel_ = nullptr;
};

} // meteo

#endif // METEO_COMMONS_MSGDATA_MSGSAVER_H
