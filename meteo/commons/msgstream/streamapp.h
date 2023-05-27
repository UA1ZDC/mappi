#ifndef METEO_COMMONS_MSGSTREAM_CORE_STREAMAPP_H
#define METEO_COMMONS_MSGSTREAM_CORE_STREAMAPP_H

#include <qhash.h>
#include <qobject.h>
#include <qdatetime.h>
#include <qstringlist.h>

#include <commons/container/ts_list.h>

#include <meteo/commons/proto/msgstream.pb.h>

class QTimer;

namespace google {
namespace protobuf {

class RpcController;

} // protobuf
} // google

namespace meteo {
namespace rpc {

class Channel;

} // rpc
} // meteo

namespace meteo {

class AppStatusThread;
class StreamModule;

} //meteo

namespace meteo {
namespace tlg {

class MessageNew;

} // tlg
} //meteo

namespace meteo {
namespace msgcenter {

class ProcessMsgRequest;
class ProcessMsgReply;
class DistributeMsgReply;

} // msgcenter
} //meteo


namespace meteo {

//! Взаимодействие msgstream с msgcenter.
//! 1. Сообщения отправляемые в msgcenter помещаются в отдельную очередь.
//! 2. Сообщения отправляются в msgcenter пока размер очереди не достигнет максимального размера.
//! 3. При получении ответа от msgcenter очередь очищается.
//! 4. При разрыве связи сообщения из очереди отправленных сообщений перемещаются в очередь приёма.
class StreamApp : public QObject
{
  Q_OBJECT

  enum StatusParam {
    kInQueueSize,
    kOutQueueSize,
    kCenterQueueSize,
    kCenterConnection,
  };

public:
  StreamApp(QObject* parent = nullptr);
  virtual ~StreamApp();

  void setStatus(AppStatusThread* status);
  void setIncoming(TSList<tlg::MessageNew>* list) { incoming_ = list; }
  void setOutgoing(TSList<tlg::MessageNew>* list) { outgoing_ = list; }

  bool init(const msgstream::Options& opt);

public slots:
  //! Отправляет данные в msgcenter на обработку.
  void slotSendToCenter();
  void slotConnectToCenter();
  void slotCenterDisconnected();

private:
  void msgProcessed(msgcenter::ProcessMsgReply* r);
  void receiveOutboundMsg(tlg::MessageNew* r);

  void subscribeToReceiveMsg();

private:
  // параметры
  msgstream::Options opt_;

  bool isReceiver_  = false;
  int msgListSize_  = 0;

  int maxQueueSize_ = 1;

  // данные
  TSList<tlg::MessageNew>* incoming_ = nullptr;
  TSList<tlg::MessageNew>* outgoing_ = nullptr;

//  QHash<QString,tlg::MessageNew> toCenter_;
  int notConfirmed_ = 0;

  // служебные
  rpc::Channel* channel_    = nullptr;
  AppStatusThread* status_  = nullptr;

  bool processIncoming_ = false;

signals:
  void newOutgoingMsg();

};

} // meteo

#endif // METEO_COMMONS_MSGSTREAM_CORE_STREAMAPP_H
